import tkinter as tk
from tkinter import filedialog
import os
import glob
import numpy as np

root = tk.Tk()
root.withdraw()

file_path = filedialog.askdirectory()
prefix = input("Was steht am Anfang?\n")
n_pre = input("Was soll am Angang stehen?\n")
suffix = input("Welches Datei-Format?\n")
for i in [x[0] for x in os.walk(file_path)]:
    y = np.array(glob.glob(i + "/" + prefix + '*.' + suffix))[
        np.argsort([int(x[len(i) + 1 + len(prefix):-len(suffix) - 1])
                    for x in glob.glob(i + "/" + prefix + '*.' + suffix)])]
    for j in range(len(y)):
        os.rename(r'' + y[j], r'' + i + "/" + n_pre + str(j) + '.' + suffix)

# import tkinter as tk
# from tkinter import filedialog
# import os
# import glob
# import re
#
# def atoi(text):
#     return int(text) if text.isdigit() else text
#
# def natural_keys(text):
#     return [ atoi(c) for c in re.split(r'(\d+)', text) ]
#
# root = tk.Tk()
# root.withdraw()
#
# file_path = filedialog.askdirectory()
# prefix = input("Was steht am Anfang?\n")
# n_pre = input("Was soll am Angang stehen?\n")
# suffix = input("Welches Datei-Format?\n")
#
# for root, dirs, files in os.walk(file_path):
#     for file in sorted(files, key=natural_keys):
#         print(file)
