#!/usr/bin/python3

# generate-image.py - Image generator for light wand
# Copyright (c) 2015 Matthew Petroff
# http://mpetroff.net
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.

import argparse
import struct

from PIL import Image


# Parse input
parser = argparse.ArgumentParser(description='Generate an image for the light wand.',
                                 formatter_class=argparse.ArgumentDefaultsHelpFormatter)
parser.add_argument('inputFile', metavar='INPUT',
                    help='Input image filename')
parser.add_argument('startDelay', metavar='START_DELAY', type=int,
                     help='Start delay, in milliseconds')
parser.add_argument('lineDelay', metavar='LINE_DELAY', type=int,
                     help='Delay between lines, in milliseconds')
args = parser.parse_args()

# Open image
im = Image.open(args.inputFile)
im = im.rotate(-90)
raw = im.tobytes()

# Open output file
f = open(args.inputFile.split('.')[0] + '.bin', 'wb')

# Write start delay configuration
# First two bytes in the file are a little endian unsigned short containing the
# start delay in milliseconds
f.write(struct.pack('<H', args.startDelay))

# Write line delay configuration
# Third and fourth bytes in the file are a little endian unsigned short
# containing the delay between lines in milliseconds
f.write(struct.pack('<H', args.lineDelay))

# Gamma table
gamma = 1.8
gammaTable = []
for i in range(256):
    gammaTable.append(int((pow(float(i) / 255.0, gamma) * 255.0 + 0.5)))

# Convert image to correct format
output = b''
for x in range(int(len(raw) / 3)):
    # Convert RGB to GRB, applying gamma correction
    pixel = gammaTable[struct.unpack_from('B', raw, x * 3)[0]] << 8 \
        | gammaTable[struct.unpack_from('B', raw, x * 3 + 1)[0]] << 16 \
        | gammaTable[struct.unpack_from('B', raw, x * 3 + 2)[0]]
    
    # Convert to OctoWS2811 format
    mask = 0x800000
    for m in range(24):
        b = 0
        if (pixel & mask) != 0:
            b |= 1
        output += struct.pack('B', b)
        mask = mask >> 1

# Write image
f.write(output)

# Close output
f.close()
