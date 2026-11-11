import sys
import os
import re
import argparse
import tkinter as tk
from tkinter import filedialog
from tkinter import messagebox
from datetime import datetime

# main_data.h: project root = folder above this Scripts folder
SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
DEFAULT_HEADER = os.path.join(SCRIPT_DIR, "..", "main_data.h")

# Function to find the selected battery pack (vehicle) in main_data.h
def get_vehicle_name(header_file):
    """Return the uncommented pack define in the '---- Select ONE Battery Pack ----' block
    of main_data.h, e.g. 'ETX_10_kWH'. Returns None if not found or more than one is selected."""
    try:
        with open(header_file, "r", encoding="utf-8", errors="ignore") as f:
            text = f.read()
    except OSError:
        print(f"WARNING: {header_file} not found, vehicle name not added")
        return None

    text = text.replace("\x00", "")                         # in case the file is UTF-16
    text = re.sub(r"/\*.*?\*/", "", text, flags=re.S)       # drop /* ... */ comments

    start = text.find("Select ONE Battery Pack")
    if start < 0:
        print("WARNING: 'Select ONE Battery Pack' not found in main_data.h, vehicle name not added")
        return None
    block = text[start:]
    end = re.search(r"^\s*#\s*if\b", block, flags=re.M)     # block ends at the first #if
    if end:
        block = block[:end.start()]

    selected = re.findall(r"^[ \t]*#[ \t]*define[ \t]+(\w+)", block, flags=re.M)  # uncommented only
    if len(selected) == 1:
        return selected[0]
    if not selected:
        print("WARNING: no battery pack selected in main_data.h, vehicle name not added")
    else:
        print(f"WARNING: more than one battery pack selected in main_data.h ({', '.join(selected)}), vehicle name not added")
    return None

# Function to build the output file name: <base>_<vehicle>_<YYYY_MM_DD_HH_MM_SS>.txt
def make_output_name(base_name, header_file):
    vehicle = get_vehicle_name(header_file)
    timestamp = datetime.now().strftime("%Y_%m_%d_%H_%M_%S")
    if vehicle:
        return f"{base_name}_{vehicle}_{timestamp}.txt"
    return f"{base_name}_{timestamp}.txt"

# Function to pad the hex array
def pad_hex_array(hex_array, divisor):
    rem = len(hex_array) % divisor
    if rem != 0:
        return hex_array + ["00"] * (divisor - rem)
    else:
        return hex_array
    
# Function to process the file
def process_file(get_file, save_file, divisor, num_data_line):
    with open(get_file, "r") as f:
        data = f.read()[1:-3]

    data = data.replace(" ", ",").replace('\n', '')
    hexs = data.split(",")
    numb = pad_hex_array(hexs, divisor)

    nums = ["0x" + num for num in numb]
    new_data = ""
    count = 0

    for num in nums:
        new_data += num + ","
        count += 1
        if count == num_data_line:
            new_data += "\n"
            count = 0

    if new_data.endswith(","):
        new_data = new_data[:-1]

    with open(save_file, "w") as f:
        f.write(new_data)

# Function to get user input from GUI
def get_user_input():
    try:
        divisor = int(divisor_entry.get())
        num_data_line = int(num_data_line_entry.get())
    except ValueError:
        messagebox.showerror("Error", "Divisor and number of data per line must be integers.")
        return

    get_file = filedialog.askopenfilename(title="Select file to get data from")
    
    # Auto-generate a suggested name with date and time for the GUI too
    if get_file:
        base_name = os.path.splitext(os.path.basename(get_file))[0]
        suggested_name = make_output_name(base_name, DEFAULT_HEADER)
        
        save_file = filedialog.asksaveasfilename(
            title="Select file to save data to", 
            initialfile=suggested_name,
            defaultextension=".txt"
        )

        if save_file:
            process_file(get_file, save_file, divisor, num_data_line)
            messagebox.showinfo("Success", "The file has been processed and saved successfully.")
        else:
            messagebox.showerror("Error", "Save file selection cancelled.")

def launch_gui():
    # Create the main window
    global divisor_entry, num_data_line_entry
    root = tk.Tk()
    root.title("Hex Data Processor")

    tk.Label(root, text="Enter the divisor:").grid(row=0, column=0)
    divisor_entry = tk.Entry(root)
    divisor_entry.insert(0, "12")
    divisor_entry.grid(row=0, column=1)

    tk.Label(root, text="Enter the number of data per line:").grid(row=1, column=0)
    num_data_line_entry = tk.Entry(root)
    num_data_line_entry.insert(0, "32")
    num_data_line_entry.grid(row=1, column=1)

    process_button = tk.Button(root, text="Process File", command=get_user_input)
    process_button.grid(row=2, column=0, columnspan=2)

    root.mainloop()

if __name__ == "__main__":
    # Check if command line arguments were provided (used for CCS automation)
    if len(sys.argv) > 1:
        parser = argparse.ArgumentParser(description="Process hex data files.")
        parser.add_argument("input_file", help="The input file from CCS")
        parser.add_argument("--divisor", type=int, default=12, help="Divisor value (default: 12)")
        parser.add_argument("--num_data", type=int, default=32, help="Number of data per line (default: 32)")
        parser.add_argument("--header", default=DEFAULT_HEADER, help="main_data.h with the selected battery pack (default: ../main_data.h)")
        
        args = parser.parse_args()
        
        # 1. Get the directory and the base name of the input file
        input_path = args.input_file
        directory = os.path.dirname(input_path)
        base_name = os.path.splitext(os.path.basename(input_path))[0]
        
        # 2. Create the "OBD fimware" directory inside the current directory (Debug)
        output_dir = os.path.join(directory, "OBD fimware")
        os.makedirs(output_dir, exist_ok=True)
        
        # 3. + 4. Output file name: <base>_<vehicle from main_data.h>_<YYYY_MM_DD_HH_MM_SS>.txt
        output_filename = make_output_name(base_name, args.header)
        output_path = os.path.join(output_dir, output_filename)
        
        print(f"Processing: {input_path}")
        print(f"Saving to:  {output_path}")
        
        process_file(input_path, output_path, args.divisor, args.num_data)
    else:
        # No arguments provided, launch the GUI manually
        launch_gui()
