#!/usr/bin/env python3
import sys
namesyms = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz?!"
passsyms = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz?!"
magic    = [38,56,10,18] # List of magic integers used for encryption

# Input password. Convert it into numbers.
words = '?SYKud 8t4fHwH K9ghBo KICMe6a'
if(len(sys.argv) != 1): words = sys.argv[1]
password = [passsyms.index(c) for c in words if c in passsyms]

if (sum(password)&63) != 0: print("Invalid password (sum2)")

# Decrypt
for x in range(24,0,-1):
    password[x] = 63 & ((password[x] - magic[x&3]) ^ password[x-1])

if (sum(password[1:25]) & 63) != 0: print("Invalid password (sum1)")

results = {}
data = [('map',4), ('olive01',4), ('olive10',4), ('skins',5),
        ('hero0',6),('hero1',6),('hero2',6),('hero3',6),('hero4',6),('hero5',6),
        ('girl0',6),('girl1',6),('girl2',6),('girl3',6),('girl4',6),('girl5',6),
        ('hp',7),
        ('flags0',8),('flags1',8),('flags2',8),('flags3',8),('flags4',8), ('zero',2)]
source      = 2*6 # Target bit index in the password. Start from slot 2.
for dataitem, numbits in data:                # Process each data item sequentially.
    results[dataitem] = 0
    while numbits > 0:                        # Process all bits of data item
        fit      = min(numbits, 6-source%6)   # Write as many bits as fit in this slot
        results[dataitem] |= ((password[source // 6] >> (6-fit-source%6)) % (1 << fit)) << (numbits-fit)
        numbits -= fit
        source  += fit
assert(source == 25*6)

results['olives'] = results['olive10']*10 + results['olive01']
results['hero'] = ''.join(namesyms[results['hero'+str(i)]] for i in range(6))
results['girl'] = ''.join(namesyms[results['girl'+str(i)]] for i in range(6))
del results['olive10']
del results['olive01']
for i in range(6): del results['hero'+str(i)]
for i in range(6): del results['girl'+str(i)]

print(results)
