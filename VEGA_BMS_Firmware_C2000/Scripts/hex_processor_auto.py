import sys
import os
import argparse
import tkinter as tk
from tkinter import filedialog
from tkinter import messagebox
from datetime import datetime

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
        timestamp = datetime.now().strftime("%Y_%m_%d_%H_%M_%S")
        suggested_name = f"{base_name}_{timestamp}.txt"
        
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
        
        args = parser.parse_args()
        
        # 1. Get the directory and the base name of the input file
        input_path = args.input_file
        directory = os.path.dirname(input_path)
        base_name = os.path.splitext(os.path.basename(input_path))[0]
        
        # 2. Create the "OBD fimware" directory inside the current directory (Debug)
        output_dir = os.path.join(directory, "OBD fimware")
        os.makedirs(output_dir, exist_ok=True)
        
        # 3. Create the date and time string (Format: YYYY_MM_DD_HH_MM_SS)
        timestamp = datetime.now().strftime("%Y_%m_%d_%H_%M_%S")
        
        # 4. Combine them to make the new output file name
        output_filename = f"{base_name}_{timestamp}.txt"
        output_path = os.path.join(output_dir, output_filename)
        
        print(f"Processing: {input_path}")
        print(f"Saving to:  {output_path}")
        
        process_file(input_path, output_path, args.divisor, args.num_data)
    else:
        # No arguments provided, launch the GUI manually
        launch_gui()
