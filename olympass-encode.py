#!/usr/bin/env python3
import random,math,sys
d = {'hero': 'Bisqwt',    'maxhp': 40,
     'girl': 'Pretty',    'map':   8,
     'olives': 99,        'flags': 0x3b0044073,
     'skins':  31,        'zero':  0
    } # Example game data

namesyms = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz?!"
passsyms = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz?!"
magic    = [38,56,10,18] # List of magic integers used for encryption

# Initialize a blank password: 26 zero-symbols.
password    = [0] * 26

# In slot 0, place a random number.
password[0] = random.randint(0,63)

# In slots 2..24, place the bitstream containing the game data (138 bits).
n    = lambda s: sum(64**i*namesyms.index((s+"!!!!!!")[i]) for i in range(6)) # Converts a name into bits
swap = lambda b,n,w: sum(((w >> (b*c)) %(1<<b)) << (b*(n-1-c)) for c in range(n))
flags = sum(((d['flags'] >> (8*k)) & 255) << (8*(4-k)) for k in range(5))    # Byteswapped flags
target = 25*6 # Start writing at the end of the password.
# Process each bitstream item (e.g. flags begins at bit 2, maxhp at bit 42, girl name at 49, etc.)
for numbits, dataitem in ((2,  d['zero']),                  (40, swap(8,5, d['flags'])),
                          (7,  d['maxhp']),
                          (36, swap(6,6, n(d['girl']))),
                          (36, swap(6,6, n(d['hero']))),    (5,  d['skins']),
                          (8,  swap(4,2, d['olives'] + d['olives']//10*6)),
                          (4,  d['map'])):
    src = 0
    while src < numbits: # Write this data item into the slots, in units of 6 bits at most
        fit      = min(numbits-src, ((target%6+5)%6+1)) # Write as many bits as fits in this slot
        password[(target-fit) // 6] += ((dataitem >> src) % (1 << fit)) << ((150-target)%6)
        target  -= fit
        src     += fit
assert(target == 2*6) # Should have gotten to beginning of slot 2.

# In slot 1, place a checksum of the bitstream
password[1] = 63 & -sum(password[2:25])

# Encrypt
for x in range(1,25):
    password[x] = 63 & ((password[x] ^ password[x-1]) + magic[x&3])

# In the last slot, place a checksum of all preceding slots.
password[25] = 63 & -sum(password[0:25])

# Convert the password into a printable string.
s = ''.join(passsyms[c] for c in password)

# Print out the password formatted in groups of 6, 7, 6, 7 symbols.
print(sys.argv[1], "\t", s[0:6], ' ', s[6:13], " ", s[13:19], ' ', s[19:26], sep='')
