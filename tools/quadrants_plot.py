# Python program to generate the quadrants plot.
# To update, run the program and save as .png file.
#
# Usage:
# ------
# python3 quadrants_plot.py
#
# Module installation:
# --------------------
# $ pip3 install matplotlib
#

import matplotlib.pyplot as plt
import numpy as np

x = np.arange(0, 360, 1)   # start,stop,step, in deg
v1 = np.cos((x - 45) * np.pi / 180)
v2 = np.sin((x - 45) * np.pi / 180)
xq = (x - 45) / 90  # quadrants

fig = plt.figure()
ax = fig.add_axes([0.2, 0.1, 0.7, 0.7])  # main axes

plt.axhline(y=0,  linestyle=":", color='lightgray')

for xb in [0, 1, 2, 3]:
    plt.axvline(x=xb, linestyle=":", color='lightgray')

for xb in [-0.5, 0.5, 1.5, 2.5, 3.5]:
    plt.axvline(x=xb, linestyle="-", color='lightblue')

ax.plot(xq, v1, color='red', label='Coil1')
ax.plot(xq, v2, color='green', label='Coil2')
plt.legend()

plt.title('Stepper Motor Four Full Steps Cycle')
#plt.xlabel('Full steps')
plt.ylabel('Coil currents [A]')

ax.set_xticks([-0.25, 0.25, 0.75, 1.25, 1.75, 2.25, 2.75, 3.25])
ax.set_xticklabels(['case3\nstep0', 'case1\nstep0',
                    'case2\nstep1', 'case6\nstep1',
                    'case5\nstep2', 'case7\nstep2',
                    'case8\nstep3', 'case4\nstep3'])

plt.show()
