#!/usr/bin/env python
import random,urllib,gd

# Binary pattern: Coordinates of dot in each group for none, first, second, both
groups = [['A2','B1','C2','A1', 'TO','BR'],
          ['A3','B4','C3','A4', 'PH','DR'],
          ['A5','B6','C5','B5', 'RI','DU'],
          ['D1','F2','E1','E2', 'SK','DI'],
          ['E3','D4','D3','F3', 'WI','BA']]

# Population count indicator locations
popcnt = ['E5','A6','B2','B3','C1','C4','C6','D2','E4','E6','F1']

# Generate a random set of items
bitmask = random.randint(0, 1023)

# Load the skeleton of the password entry picture
#im = gd.image('megaman4-model2.png')
im = gd.image(urllib.urlretrieve('http://iki.fi/bisqwit/jkp2/megaman4-model2.png')[0])

# Locations of the model dot (F6)
model = (136, 120)

# Shorthand functions for placing dots and drawing filled rectangles
def place(l):  im.copyTo(im, (56+16*(ord(l[1])-ord('1')),40+16*(ord(l[0])-ord('A'))), model, (8,8))
def rect(p,w): im.filledRectangle(p, (p[0]+w-1, p[1]+7), im.colorClosest((9,9,9)))

# Place the population count indicator dot, according to the number of 1-bits in bitmask
place(popcnt[bin(bitmask).count('1')])

# Process all the five groups
for nr in range(0, 5):
    # Place dot
    place(groups[nr][bitmask & 3])

    # Blot out the names of items we don't have
    if(not(bitmask & 1)): rect((64+nr*24, 176), 16)
    if(not(bitmask & 2)): rect((64+nr*24, 194), 16)
    bitmask >>= 2

# Remove the model dot from the always-unused cell F6
rect(model, 8)

# Save output
im.writePng('megaman4password.png')
