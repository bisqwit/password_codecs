hard  = 1
ally  = 3
random= 0
stage = 0
slots = [0,3,0,0, 1,0,0,3, 0,2,3,0, 3,2,0,0] # Password to be decoded (overwritten if encoding)
name  = "AAAAAAAA"                           # Name
encode= True

sym   = "ABCDEFGHIJKLMNOPQRSTUVWXYZ!?ZZZZ ZZ."
magic = 0x5249B24BEFF_D93FC4A17_619C7BE24_0E351A78F_748236150
hash  = (sum(sym.index(c) for c in (name+' '*8)[:8] if sym.index(c) is not None) ^ 4) & 7
nib   = lambda i: (magic >> (4*i)) & 0xF
crypt = lambda v,k: (((k+1)*0x55^v*0x10001)*0x11&0xF000F0)*0x1001>>16

if encode: # encode
  # Generate two bytes
  low      = hard%2 + ally%4*2 + random%2*8 + stage%2*16 + hash*32
  high     = stage + crypt(low, random)
  slots    = [0]*16
  # Determine and place the key
  keytype               = nib(stage//2)//3*9+9
  slots[nib(keytype+8)] = nib(stage//2) %3+1
  # Place the rest of data
  for i in range(8): slots[nib(keytype+i)] = (high>>i)%2 + (low>>i)%2*2
  # Print the grid
  for i,k in enumerate(slots): print(k, end="\n" if (i%4==3) else ' ')

if True: # decode
  # Identify key(s)
  d=[(s,k) for s,k in [(s,nib(s)//3*9+9) for s in range(9)] if slots[nib(k+8)]==nib(s)%3+1]
  stage,keytype = d[0] if d else (0,0)
  # Extract the two bytes
  high  = sum((slots[nib(keytype+i)]   %2) << i for i in range(8))
  low   = sum((slots[nib(keytype+i)]//2%2) << i for i in range(8))
  # Extract game data
  hard,ally,random,stage,bits = low%2, low//2%4, low//8%2, stage*2+low//16%2, low//32
  # Check for error cases
  if len(d) != 1:                                                                     print("Error 0: Multiple/no key symbols found")
  if any(slots[j] and all(j != nib(keytype+i) for i in range(9)) for j in range(16)): print("Error 1: Extra symbols found in the grid")
  if high != (stage + crypt(low, random)) & 0xFF:                                     print("Error 2: Checksum error")
  if ally and not hard and (magic >> (144+stage*3+ally%3))&1:                         print("Error 3: Selected ally illegal in this stage")
  if hash != bits:                                                                    print("Error 4: Name and password discrepancy: hash=",hash,"but password requires",bits)
  # Print the extracted game data
  print("Hard=",hard,"ally=",ally,"random=",random,"stage=",stage,"bits=",bits)
