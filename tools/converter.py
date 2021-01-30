# Converts screenshot dump from the stepper analyzer to a png.
# Crude and for developement purposes only.

import sys
from PIL import Image, ImageDraw
import os


# Read lines from input file
file1 = open('screenshot.txt', 'r')
lines = file1.readlines()


image = Image.new(mode="RGB", size=(480, 320), color="red")

# Set one pixel in the image.


def put_pixel(x, y, color8):
    r3 = (color8 >> 5) & 0x7
    g3 = (color8 >> 2) & 0x7
    b2 = color8 & 0x3
    r = int(r3 * 255 / 7)
    g = int(g3 * 255 / 7)
    b = int(b2 * 255 / 3)
    image.putpixel((x, y), (r, g, b, 255))

# Process a line with pixels.


def process_data_line(l, line):
    print(f"Processing line {l+1}")
    if not line.startswith("#"):
        raise Exception(f"Data lines {l+1} doesn't start with a #.")
    tokens = line[1:].split(',')
    x0 = int(tokens[0])
    y0 = int(tokens[1])
    n = int(tokens[2])
    color_tokens = tokens[3:]
    x = x0
    for color_token in color_tokens:
        parts = color_token.split(':')
        count = int(parts[0])
        color = int(parts[1], 16)
        for i in range(count):
           put_pixel(x, y0, color)
           x+=1


# Increment l to the index of data line past the BEGIN line.
l = 0
while True:
    if l >= len(lines):
        raise Exception("###BEGIN line not found")
    if lines[l].startswith("###BEGIN"):
        l += 1
        break
    l += 1

# Process the data lines until END line.
while True:
    if l >= len(lines):
        raise Exception("###END line not found")
    line = lines[l].rstrip()
    if line.startswith("###END"):
        break
    process_data_line(l, line)
    l += 1

image.save("screenshot.png")
print("All done.")
