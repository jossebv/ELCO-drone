import tkinter as tk
from tkinter import ttk

# Create the main window
window_height = 844
window_width = 1500

window = tk.Tk()
window.title("ELCOCOPTER CONTROL PANEL")
hs = window.winfo_screenheight()
ws = window.winfo_screenwidth()
x = (ws / 2) - (window_width / 2)
y = (hs / 2) - (window_height / 2)
window.geometry("%dx%d+%d+%d" % (window_width, window_height, x, y))

# Create the Frames
style = ttk.Style()
style.configure("TFrame", background="red")
window.update()
frame1 = tk.Frame(
    window, bg="red", width=window.winfo_width(), height=window.winfo_height() / 2
)
frame1.pack()


window.mainloop()
