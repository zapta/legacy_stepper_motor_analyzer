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
v1 = np.cos((x) * np.pi / 180)
v2 = np.sin((x) * np.pi / 180)
xq = x / 90  # quadrants

fig = plt.figure()
ax = fig.add_axes([0.2, 0.1, 0.7, 0.7])  # main axes

plt.axhline(y=0,  linestyle=":", color='lightgray')

for xb in [0.5, 1.5, 2.5, 3.5]:
    plt.axvline(x=xb, linestyle=":", color='lightgray')

for xb in [0.0, 1.0, 2.0, 3.0, 4.0]:
    plt.axvline(x=xb, linestyle="-", color='lightblue')

ax.plot(xq, v1, color='red', label='Coil1')
ax.plot(xq, v2, color='green', label='Coil2')
plt.legend()

plt.title('Stepper Motor Four Full Steps Cycle')
plt.ylabel('Coil current')

ax.set_xticks([0.25, 0.5, 0.75, 1.25, 1.5, 1.75, 2.25, 2.5, 2.75, 3.25, 3.5, 3.75])
ax.set_xticklabels(['sector0', '\nStep0', 'sector1',
                    'sector2', '\nstep1', 'sector3',
                    'sector4', '\nstep2', 'sector5',
                    'sector6', '\nstep3', 'sector7'])

plt.show()
