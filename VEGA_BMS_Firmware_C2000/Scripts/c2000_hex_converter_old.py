import tkinter as tk
from tkinter import filedialog
from tkinter import messagebox

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

    new_data = new_data[:-1]

    with open(save_file, "w") as f:
        f.write(new_data)

# Function to get user input and call processing function
def get_user_input():
    divisor = int(divisor_entry.get())
    num_data_line = int(num_data_line_entry.get())
    get_file = filedialog.askopenfilename(title="Select file to get data from")
    save_file = filedialog.asksaveasfilename(title="Select file to save data to", defaultextension=".txt")

    if get_file and save_file:
        process_file(get_file, save_file, divisor, num_data_line)
        messagebox.showinfo("Success", "The file has been processed and saved successfully.")
    else:
        messagebox.showerror("Error", "File selection cancelled or invalid input.")

# Create the main window
root = tk.Tk()
root.title("Hex Data Processor")

# Create and place the divisor input
tk.Label(root, text="Enter the divisor:").grid(row=0, column=0)
divisor_entry = tk.Entry(root)
divisor_entry.grid(row=0, column=1)

# Create and place the num_data_line input
tk.Label(root, text="Enter the number of data per line:").grid(row=1, column=0)
num_data_line_entry = tk.Entry(root)
num_data_line_entry.grid(row=1, column=1)

# Create and place the process button
process_button = tk.Button(root, text="Process File", command=get_user_input)
process_button.grid(row=2, column=0, columnspan=2)

# Run the main loop
root.mainloop()
