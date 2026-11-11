"""
build_check.py - Safety check before the OBD test file is generated.

Shows the selected vehicle (battery pack) and the linked linker command file, and asks the
user Yes / No.
  Yes -> exit code 0
  No  -> exit code 1

CCS post-build step (one line, '&&' so hex_processor_auto.py runs only after Yes):
  python "${ProjDirPath}\\Scripts\\build_check.py" && python "${ProjDirPath}\\Scripts\\hex_processor_auto.py" "${BuildArtifactFileBaseName}.txt"

Note: do NOT put the two commands on separate lines. CCS runs every post-build line with '-'
(errors ignored), so hex_processor_auto.py would run even after No.
"""

import sys
import os
import re
import argparse
import tkinter as tk

# main_data.h: project root = folder above this Scripts folder
SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
DEFAULT_HEADER = os.path.join(SCRIPT_DIR, "..", "main_data.h")

# Linker command file the OBD test file must be built with
EXPECTED_CMD_FILE = "ETX_BMS.cmd"

# Linker command files that are always linked (not shown)
COMMON_CMD_FILES = ("F2806x_Headers_nonBIOS.cmd",)


def get_vehicles(header_file):
    """Return the uncommented defines in the '---- Select ONE Battery Pack ----' block
    of main_data.h, or None if main_data.h is not found."""
    try:
        with open(header_file, "r", encoding="utf-8", errors="ignore") as f:
            text = f.read()
    except OSError:
        return None
    text = text.replace("\x00", "")                         # in case the file is UTF-16
    text = re.sub(r"/\*.*?\*/", "", text, flags=re.S)       # drop /* ... */ comments

    start = text.find("Select ONE Battery Pack")
    if start < 0:
        return []
    block = text[start:]
    end = re.search(r"^\s*#\s*if\b", block, flags=re.M)     # block ends at the first #if
    if end:
        block = block[:end.start()]
    return re.findall(r"^[ \t]*#[ \t]*define[ \t]+(\w+)", block, flags=re.M)


def get_cmd_files(build_dir):
    """Return the .cmd files CCS linked, from <build_dir>/subdir_vars.mk (CMD_SRCS),
    without the always-linked ones. Returns None if subdir_vars.mk is not found."""
    mk = os.path.join(build_dir, "subdir_vars.mk")
    try:
        with open(mk, "r", encoding="utf-8", errors="ignore") as f:
            text = f.read()
    except OSError:
        return None
    cmd_files = []
    for block in re.findall(r"CMD_SRCS\s*\+?=\s*((?:[^\n]*\\\n)*[^\n]*)", text):
        for item in block.replace("\\\n", " ").split():
            name = os.path.basename(item.strip('"'))
            if name.lower().endswith(".cmd") and name not in COMMON_CMD_FILES and name not in cmd_files:
                cmd_files.append(name)
    return cmd_files



def check_build(header_file, build_dir):
    """Return a dict describing the build:
    vehicle, vehicle_ok, cmd, cmd_ok, warnings, message (plain text for the console)."""
    warnings = []

    # vehicle
    vehicles = get_vehicles(header_file)
    vehicle_ok = False
    if vehicles is None:
        vehicle_text = "NOT FOUND"
        warnings.append(f"main_data.h not found: {os.path.abspath(header_file)}")
    elif len(vehicles) == 1:
        vehicle_text = vehicles[0]
        vehicle_ok = True
    elif not vehicles:
        vehicle_text = "NOT FOUND"
        warnings.append("No battery pack selected in main_data.h.")
    else:
        vehicle_text = ", ".join(vehicles)
        warnings.append("More than one battery pack selected in main_data.h.")

    # linker command file
    cmd_files = get_cmd_files(build_dir)
    cmd_ok = False
    if cmd_files is None:
        cmd_text = "NOT FOUND"
        warnings.append(f"subdir_vars.mk not found in {os.path.abspath(build_dir)}.")
    elif not cmd_files:
        cmd_text = "NONE"
        warnings.append(f"No linker command file linked, expected {EXPECTED_CMD_FILE}.")
    else:
        cmd_text = ", ".join(cmd_files)
        cmd_ok = (cmd_files == [EXPECTED_CMD_FILE])
        if not cmd_ok:
            warnings.append(f"Linker command file is {cmd_text}, expected {EXPECTED_CMD_FILE}.")

    message = f"Vehicle:              {vehicle_text}\nLinker command file:  {cmd_text}"
    if warnings:
        message += "\n\nWARNING:\n- " + "\n- ".join(warnings)
    message += "\n\nGenerate the OBD test file for this build?"

    return {"vehicle": vehicle_text, "vehicle_ok": vehicle_ok,
            "cmd": cmd_text, "cmd_ok": cmd_ok,
            "warnings": warnings, "message": message}


# Dialog colours (CCS dark theme dialog + blue border + coloured bold values)
BORDER_BLUE = "#3386C5"     # outer border (CCS accent blue)
DLG_BG      = "#515658"     # dialog background (CCS dark theme)
DLG_FG      = "#E8E8E8"     # normal text
BTN_BORDER  = "#C3C4C5"     # button border
BTN_HOVER   = "#5E6366"     # button background under the mouse
FOCUS_OUT   = "#2672AC"     # focused / default button: outer border
FOCUS_IN    = "#B9CFE0"     # focused / default button: inner border
OK_BG       = DLG_BG        # no box behind a correct value (option C)
OK_FG       = "#A8D4FF"     # correct value: bold light blue text
BAD_BG      = DLG_BG        # no box behind a wrong value (option C)
BAD_FG      = "#FFB4B4"     # wrong value: bold light red text
WARN_FG     = "#FF9E9E"     # warning text
UI_FONT     = "Segoe UI"    # Windows dialog font


def draw_icon(parent, warning):
    """Draw the dialog icon: blue circle with '?' (CCS question icon), or yellow triangle
    with '!' when there is a warning."""
    c = tk.Canvas(parent, width=34, height=34, bg=DLG_BG, highlightthickness=0)
    if warning:
        c.create_polygon(17, 2, 33, 31, 1, 31, fill="#FFC83D", outline="#E0A500", joinstyle="round")
        c.create_text(17, 20, text="!", font=(UI_FONT, 14, "bold"), fill="#1F1F1F")
    else:
        c.create_oval(1, 1, 33, 33, fill="#BBC1D1", outline="")       # light rim
        c.create_oval(3, 3, 31, 31, fill="#5471BA", outline="")       # blue disc
        c.create_text(17, 17, text="?", font=(UI_FONT, 15, "bold"), fill="#FFFFFF")
    return c


def dark_title_bar(root):
    """Dark Windows title bar, like the CCS dialogs (Windows 10 2004+ / 11)."""
    try:
        import ctypes
        hwnd = ctypes.windll.user32.GetParent(root.winfo_id())
        value = ctypes.c_int(1)
        for attr in (20, 19):           # DWMWA_USE_IMMERSIVE_DARK_MODE (new, old number)
            if ctypes.windll.dwmapi.DwmSetWindowAttribute(hwnd, attr, ctypes.byref(value),
                                                          ctypes.sizeof(value)) == 0:
                break
    except Exception:
        pass


def set_blank_icon(root):
    """Replace the default Tk feather icon with a fully transparent 16x16 icon."""
    import struct
    import tempfile
    size = 16
    pixels = bytes(size * size * 4)                         # 32-bit BGRA, alpha 0 = transparent
    mask = b"\xff" * (size * 4)                             # AND mask: 1 bit/pixel, rows padded to 4 bytes
    bmp = struct.pack("<IiiHHIIiiII", 40, size, size * 2, 1, 32, 0,
                      len(pixels) + len(mask), 0, 0, 0, 0) + pixels + mask
    ico = struct.pack("<HHH", 0, 1, 1) \
        + struct.pack("<BBBBHHII", size, size, 0, 0, 1, 32, len(bmp), 6 + 16) + bmp
    path = os.path.join(tempfile.gettempdir(), "build_check_blank.ico")
    try:
        with open(path, "wb") as f:
            f.write(ico)
        root.iconbitmap(default=path)
    except Exception:
        pass                                                # keep the default icon if this fails


def make_button(parent, text, command):
    """CCS dark theme button: flat, light border; blue double border when focused.
    Returns (outer frame, button)."""
    outer = tk.Frame(parent, bg=BTN_BORDER, padx=1, pady=1)
    inner = tk.Frame(outer, bg=DLG_BG, padx=1, pady=1)
    inner.pack()
    btn = tk.Button(inner, text=text, width=11, command=command, font=(UI_FONT, 9),
                    bg=DLG_BG, fg=DLG_FG, activebackground=BTN_HOVER, activeforeground=DLG_FG,
                    relief="flat", bd=0, highlightthickness=0, pady=3, cursor="hand2")
    btn.pack()

    def focus(on):
        outer.configure(bg=FOCUS_OUT if on else BTN_BORDER)
        inner.configure(bg=FOCUS_IN if on else DLG_BG)

    btn.bind("<FocusIn>", lambda e: focus(True))
    btn.bind("<FocusOut>", lambda e: focus(False))
    btn.bind("<Enter>", lambda e: btn.configure(bg=BTN_HOVER))
    btn.bind("<Leave>", lambda e: btn.configure(bg=DLG_BG))
    btn.bind("<space>", lambda e: btn.invoke())
    return outer, btn


def ask_user(info):
    """Show the Yes / No dialog (CCS dark theme style, blue outer border, vehicle and
    linker command file highlighted: blue = OK, red = wrong).
    Returns True on Yes. Enter = default button, Esc / close = No."""
    answer = {"yes": False}

    try:                                    # sharp text on scaled Windows displays
        import ctypes
        ctypes.windll.shcore.SetProcessDpiAwareness(1)
    except Exception:
        pass

    root = tk.Tk()
    root.title("Check build before generating")
    set_blank_icon(root)                    # no feather icon in the title bar
    root.configure(bg=BORDER_BLUE)
    root.resizable(False, False)
    root.attributes("-topmost", True)       # keep the dialog in front of CCS

    f_text = (UI_FONT, 9)
    f_value = (UI_FONT, 10, "bold")

    # blue outer border = blue window background around the inner frame
    inner = tk.Frame(root, bg=DLG_BG)
    inner.pack(padx=2, pady=2, fill="both", expand=True)

    # message area
    body = tk.Frame(inner, bg=DLG_BG, padx=14, pady=18)
    body.pack(fill="both", expand=True)
    draw_icon(body, bool(info["warnings"])).grid(row=0, column=0, sticky="n", padx=(0, 14))

    text = tk.Frame(body, bg=DLG_BG)
    text.grid(row=0, column=1, sticky="w")

    values = tk.Frame(text, bg=DLG_BG)      # own grid, so long warning lines do not move the values
    values.grid(row=0, column=0, sticky="w")
    rows = (("Vehicle:", info["vehicle"], info["vehicle_ok"]),
            ("Linker command file:", info["cmd"], info["cmd_ok"]))
    for r, (name, value, ok) in enumerate(rows):
        tk.Label(values, text=name, font=f_text, bg=DLG_BG, fg=DLG_FG).grid(row=r, column=0, sticky="w", pady=1)
        tk.Label(values, text=value, font=f_value,
                 bg=OK_BG if ok else BAD_BG, fg=OK_FG if ok else BAD_FG,
                 padx=6, pady=0).grid(row=r, column=1, sticky="w", padx=(8, 0), pady=1)

    r = 1
    if info["warnings"]:
        tk.Label(text, text="WARNING:", font=(UI_FONT, 9, "bold"), fg=WARN_FG, bg=DLG_BG
                 ).grid(row=r, column=0, sticky="w", pady=(12, 0))
        r += 1
        for w in info["warnings"]:
            tk.Label(text, text="- " + w, font=f_text, fg=WARN_FG, bg=DLG_BG,
                     justify="left", wraplength=420).grid(row=r, column=0, sticky="w")
            r += 1

    tk.Label(text, text="Generate the OBD test file for this build?", font=f_text,
             bg=DLG_BG, fg=DLG_FG).grid(row=r, column=0, sticky="w", pady=(12, 0))

    # buttons, bottom right
    def choose(yes):
        answer["yes"] = yes
        root.destroy()

    bar = tk.Frame(inner, bg=DLG_BG, padx=10, pady=10)
    bar.pack(fill="x")
    no_frame, b_no = make_button(bar, "No", lambda: choose(False))
    yes_frame, b_yes = make_button(bar, "Yes", lambda: choose(True))
    no_frame.pack(side="right")
    yes_frame.pack(side="right", padx=(0, 6))

    default = b_no if info["warnings"] else b_yes   # warnings -> No is the default
    default.focus_set()

    def on_enter(event):
        focused = root.focus_get()
        (focused if focused in (b_yes, b_no) else default).invoke()   # Tab moves between the buttons

    root.bind("<Return>", on_enter)
    root.bind("<Escape>", lambda e: choose(False))
    root.protocol("WM_DELETE_WINDOW", lambda: choose(False))

    # centre on the screen
    root.update_idletasks()
    root.minsize(max(root.winfo_reqwidth(), 420), root.winfo_reqheight())   # room for the title
    root.update_idletasks()
    x = (root.winfo_screenwidth() - root.winfo_width()) // 2
    y = (root.winfo_screenheight() - root.winfo_height()) // 3
    root.geometry(f"+{x}+{y}")
    dark_title_bar(root)
    default.focus_force()

    root.mainloop()
    return answer["yes"]


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Confirm vehicle and linker command file before generating the OBD test file.")
    parser.add_argument("build_dir", nargs="?", default=os.getcwd(),
                        help="CCS build folder with subdir_vars.mk (default: current folder = Debug)")
    parser.add_argument("--header", default=DEFAULT_HEADER,
                        help="main_data.h with the selected battery pack (default: ../main_data.h)")
    args = parser.parse_args()

    build = check_build(args.header, args.build_dir)
    print(build["message"])

    if ask_user(build):
        print("Confirmed by user.")
        sys.exit(0)

    print("Test file NOT generated (cancelled by user).")
    sys.exit(1)
