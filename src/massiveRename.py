import tkinter as tk
from tkinter import filedialog
import os
import glob

root = tk.Tk()
root.withdraw()

file_path = filedialog.askdirectory()
prefix = input("Was steht am Anfang?\n")
n_pre = input("Was soll am Angang stehen?\n")
suffix = input("Welches Datei-Format?\n")
for i in [x[0] for x in os.walk(file_path)]:
    for j in glob.glob(i+"/"+prefix+'*.'+suffix):
        os.rename(r''+j, r''+i+"/"+n_pre+j[len(i)+1+len(prefix):])
