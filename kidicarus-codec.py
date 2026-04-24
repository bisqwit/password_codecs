import re,sys
encode,prg1 = True,True

password = "ICARUS FIGHTS MEDUSA ANGELS"
#password = "DANGER !!!!!! TERROR HORROR"
#password = "A20000 000000 O10000 0000a8"
#password = "000000 000000 800000 00002A"
#password = "ANGELS ATTACK FLYING MOTELS"
#password = "Angels attack flying briefs"
#password = "Resist glossy winter tights"
#password = "2S06US FIGHWV cDDUSA ANGEb7"
#password = "2U06US FIGHTS sDDUSA ANGELO"
#password = "AS06US FIGHTS cDDUSA ANGELw"

#password = "6eW3!! !!!!00 F38W!H C0042N" # 1-2 with maxed strength, hearts and items
#password = "0000eu 60j700 uG0004 1000J0" # 1-4
#password = "0000ys T0J300 m2001C H000aS" # 2-1
#password = "00008C i04400 mIG01D I0005F" # 2-4
#password = "0000mu w0K200 O3G00H I100s5" # 3-1
#password = "ICARUS ANDTHE ARROWS FLYING" # 3-3
#password = "0000y0 11X200 u0G00H I100t0" # 3-4
#password = "00008p 414100 O3G00H I500eB" # 4-1

if len(sys.argv) > 1: password,encode = sys.argv[1], False

data = {
  'flag_flaming_arrow': 2, 'weapon0':   1,
  'flag_sacred_bow':    2, 'weapon1':   2,
  'flag_crystal_rod':   2, 'weapon2':   3,
  'flag_credit_card':   1, 'mallets':   99,
  'flag_mirror_shield': 1, 'feathers':  99,
  'flag_arrow_of_light':1, 'bottles':   0x48,
  'flag_pegasus_wings': 0, 'centurions':99,
  'zero':               0, 'strength':  4, 
  'score':         200000, 'endurance':   4,
  'hearts':           999, 'module': 6,
  'debt':               0, 'param':  0,
  'random':             0, 'secondquest': 1
}
qdata = {
  'flag_flaming_arrow': 0, 'weapon0':   0,
  'flag_sacred_bow':    0, 'weapon1':   0,
  'flag_crystal_rod':   0, 'weapon2':   0,
  'flag_credit_card':   0, 'mallets':   0,
  'flag_mirror_shield': 0, 'feathers':  0,
  'flag_arrow_of_light':0, 'bottles':   0,
  'flag_pegasus_wings': 0, 'centurions':0,
  'zero':               0, 'strength':  0, 
  'score':              0, 'endurance':   0,
  'hearts':             0, 'module': 4,
  'debt':               0, 'param':  0,
  'random':             0, 'secondquest': 0
}
qdata = { # ICARUS FIGHTS MEDUSA ANGELS
  'flag_flaming_arrow':  2,  'weapon0':             1,
  'flag_sacred_bow':     7,  'weapon1':             0,
  'flag_crystal_rod':    0,  'weapon2':             3,
  'flag_credit_card':    0,  'mallets':           211,
  'flag_mirror_shield':  0,  'feathers':          120,
  'flag_arrow_of_light': 3,  'bottles':           156,
  'flag_pegasus_wings':  1,  'centurions':        144,
  'zero':             1669,  'strength':           10,
  'score':         4781854,  'endurance':           2,
  'hearts':           1104,  'module':           5,
  'debt':             3441,  'param':               3,
  'secondquest':         0,  'random':             75,
}
           
qdata = {
  'flag_flaming_arrow':  2,  'weapon0':             2,
  'flag_sacred_bow':     3,  'weapon1':            20,
  'flag_crystal_rod':    4,  'weapon2':            13,
  'flag_credit_card':    0,  'mallets':           211,
  'flag_mirror_shield':  0,  'feathers':          120,
  'flag_arrow_of_light': 3,  'bottles':           156,
  'flag_pegasus_wings':  1,  'centurions':        144,
  'zero':                1,  'strength':           10,
  'score':         4781854,  'endurance':           2,
  'hearts':           1104,  'module':              5,
  'debt':             3441,  'param':               3,
  'secondquest':         0,  'random':              3,
}

qdata = {
  'flag_flaming_arrow': 0, 'weapon0':   0,
  'flag_sacred_bow':    0, 'weapon1':   0,
  'flag_crystal_rod':   0, 'weapon2':   0,
  'flag_credit_card':   0, 'mallets':   0,
  'flag_mirror_shield': 0, 'feathers':  0,
  'flag_arrow_of_light':0, 'bottles':   0,
  'flag_pegasus_wings': 0, 'centurions':0,
  'zero':               0, 'strength':  0, 
  'score':              0, 'endurance':   0,
  'hearts':             0, 'module': 0,
  'debt':               0, 'param':  0,
  'random':             0, 'secondquest': 0
}


###################################
# IMPLEMENTATION

syms   = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz?!"
recipe = {k:{
  'flag_flaming_arrow': ((0,0x03,0, 0,0x80,-5),(0,0x07,0))[prg1], 'weapon0': ((0,0x0C,-2),(0,0x18,-3, 0,0xE0,-3))[prg1],
  'flag_sacred_bow':    ((1,0x03,0, 1,0x80,-5),(1,0x07,0))[prg1], 'weapon1': ((1,0x0C,-2),(1,0x18,-3, 1,0xE0,-3))[prg1],
  'flag_crystal_rod':   ((2,0x03,0, 2,0x80,-5),(2,0x07,0))[prg1], 'weapon2': ((2,0x0C,-2),(2,0x18,-3, 2,0xE0,-3))[prg1],
  'flag_credit_card':   (9,0x01,0),    'mallets':    (10,0xFF,0), 'score':     (3,0xFF,0, 4,0xFF,8, 5,0xFF,16),
  'flag_mirror_shield': (14,0x03, 0),  'feathers':   (11,0xFF,0), 'strength':  (13,0xF0,-4),
  'flag_arrow_of_light':(14,0x0C,-2),  'bottles':    (12,0xFF,0), 'endurance': (13,0x0F,0),
  'flag_pegasus_wings': (14,0x30,-4),  'centurions': (15,0xFF,0), 'hearts':    (6,0xFF,0, 7,0x0F,8),
  'secondquest':        (16,0x08,-3),  'param':      (16,0x07,0), 'debt':      (8,0xFF,0, 7,0xF0,4),
  'module':             (16,0xF0,-4),                             'random':              ((9,0xFE,-1),(9,0xE,-1))[prg1],
  'zero':               (14,0xC0,-6) +                                      ((0,0x70,-2, 1,0x70,2, 2,0x70,4), ())[prg1]
}[k] for k in data}
def bit_convert(i,o,d): return [(sum(b << i*c for c,b in enumerate(d)) >> o*n) & ((1<<o)-1) for n in range(len(d)*i//o)]

out,e,w,W,b='','Error','Warning',[],"\b"*12+" "*12
if encode: # Encode
  # Encode data
  bytes = [sum(sum(int(d / (2**p[n+2])) & p[n+1] for n in range(0,len(p),3) if p[n]==i)
           for p,d in ((recipe[name],data[name]) for name in data)) for i in range(18)]
  # Report any data items that were not consumed entirely (out-of-range values)
  print(sep='',end='', *((lambda r=lambda d,p: sum(d & int(p[n+1] * (2**p[n+2])) for n in range(0,len(p),3)): \
        (("%s: Too large value for '%s'. Not consumed entirely (%u / 0x%X remains)\n" % (w,name, d-r(d,p), d-r(d,p)))
            for name,p,d in ((name,recipe[name],data[name]) for name in recipe) if d > r(d,p)))()))
  # Generate and store checksum
  bytes[17], bytes[9] = sum(bytes)&0xFF, bytes[9] + prg1*((sum(bytes)&0xF00)>>4)
  # Convert bytes into 6-bit symbols and then 6-bit symbols into printable characters
  password = ''.join(syms[c] for c in bit_convert(8,6,bytes))
  # Print out the password formatted in groups of 6 symbols
  out += "%s %s\n%s %s\n" % tuple(password[n:n+6] for n in range(0,24,6))

if True: # Decode
  # Convert printable characters to 6-bit symbols and then 6-bit symbols into bytes
  bytes = bit_convert(6,8, [syms.find(c) for c in (password+"0"*24) if syms.find(c) >= 0])
  # Verify checksum (PRG1 and PRG0)
  ok0 = (bytes[17]                     ) == sum(bytes[:17]) & 0xFF
  ok1 = (bytes[17] + 17*(bytes[9]&0xF0)) == sum(bytes[:17])
  if not ok0: W += [tuple(["Checksum fails in PRG0 version" + " (OK in PRG1 though)"*ok1,b]*(2-prg1))]
  if not ok1: W += [tuple(["Checksum fails in PRG1 version" + " (OK in PRG0 though)"*ok0,b]*(1+prg1))]
  # Decode
  data = {name: sum(int((bytes[p[n]] & p[n+1]) * (2**p[n+2])) for n in range(0,len(p),3))
          for name,p in ((name,recipe[name]) for name in recipe)}
  # If the non-empty password is for 1-1, print out a warning and clear the password.
  if data['module']==2 and data['param']==0 and sum(data.values()) != data['zero']+data['random']+2:
    print("%s: This password is IGNORED by the game, because it starts from 1-1" % w)
    data = {k:data[k] if k=='module' or k=='zero' or k=='random' else 0 for k in data}
  # Print out the contents as a Python dict object init statement.
  out += "data = {\n%s}\n" % ''.join("  %s%*d%s" % (n, 24-len(n), d, ",\n" if i%2 else ",")
              for i,n,d in ((i,"'%s':" % name,data[name]) for i,name in enumerate(data)))

# Report any data items that are in abnormal ranges, followed by the output
limit,ok,q = lambda m,e='': lambda v: (v<=m, '0..%d%s'%(m,e)), (True,''), data['module']
treas      = lambda m: lambda v: (v==m[q],"%d when 'module' = %d"%(m[q],q))if 1<q<9 else ok
cat,p      = lambda v: v if v<4 else v//4, 'value %s should appear in the weapon list'
w0,w1,w2,r  = data['weapon0'],data['weapon1'],data['weapon2'], 'should have score ≥ %d'
s          = set([cat(w0)|(w0&4), cat(w1)|(w1&4), cat(w1)|(w1&4)])
W += (lambda limits: [k for k in (
     ("'%s' = %u" % (name,d), ' or '.join(limits[p](d)[1] for p in limits if re.search(p,name) and not limits[p](d)[0]))
     for name,d in ((n,data[n]) for n in data)
                                ) if len(k[1])])({
          '^we':lambda v: [limit(3)(v), (v in [0,1,2,3,21,22,25,26,29,30], '0..3, 21, 22, 25, 26, 29 or 30')][prg1],
          '^flag_(fl|cry)':limit(7),
          '^flag_s':[limit(7), lambda v:(v==2, "2 when 'module' = 8")][q==8],
          '^flag_cre':lambda v: [ok, (v,'1 when has debt')][data['debt']>0],
          '^flag_m':treas([0,0,0,0,1,1,1,1,2]),
          '^flag_a':treas([0,0,0,0,0,0,1,1,2]),
          '^flag_p':treas([0,0,0,0,0,0,0,0,2]),
          '^ma':lambda v: (v<=99, '0..99' + '; when >128, item usable only once'*(v>128)),
          '^fe':lambda v: (v<=99, '0..99' + '; when >128, item not actually usable'*(v>128)),
          '^ce':lambda v: (v<=99, '0..99' + '; when >127, item not actually usable'*(v>127)),
          '^h':lambda v: [limit(999)(v), (v==0,'0 when has debt'+' (and <=999 overall)'*(v>999))][data['debt']>0],
          '^d':limit(999),
          '^sc':limit(9999999),
          '^str':limit(4),
          '^e':lambda v: (lambda s,q: (0<0, '0..4')if v>4 else (s>=q[v], r%q[v]))(data['score'],[0,20000,50000,100000,20000]),
          '^zero':lambda v: (v<1, '0'),
          '^bo':lambda v: (v<2 or 0x3F<v<0x49, '0..1 or 64..72'),
          '^mo':lambda v: (1<v<9, '2..8' + ' (>9 causes crash)'*(v>9)),
          '^pa':limit((0 if q%2 or q>7 else 3) if 1<q<9 else 7, " when 'module' = %d"%q),
          'on1':lambda v: (w0 or not v, "0 when '%s' = %d" % ('weapon0', w0)),
          'on2':lambda v: (w1 or not v, "0 when '%s' = %d" % ('weapon1', w1)),
          'n0':lambda v: (cat(v)!=cat(w1) or not v, "different powerup index than 'weapon1'"),
          'n1':lambda v: (cat(v)!=cat(w2) or not v, "different powerup index than 'weapon2'"),
          'n2':lambda v: (cat(v)!=cat(w0) or not v, "different powerup index than 'weapon0'"),
          'min':lambda v: (lambda d: (s.intersection(d), p % ' or '.join(str(i) for i in d)))([[0],[1],[[1,2],[5,6,21,22]][prg1]][(v>0)+(v>3)]),
          'bow':lambda v: (lambda d: (s.intersection(d), p % ' or '.join(str(i) for i in d)))([[0],[2],[[1,2],[9,10,25,26]][prg1]][(v>0)+(v>3)]),
          'rys':lambda v: (lambda d: (s.intersection(d), p % ' or '.join(str(i) for i in d)))([[0],[3],[[1,2],[13,14,29,30]][prg1]][(v>0)+(v>3)])
})
s = set([w0,w1,w2])
for p,i in [('flag_flaming_arrow',1),
            ('flag_sacred_bow',   2),
            ('flag_crystal_rod',  3)]:
  for r in ['weapon0','weapon1','weapon2']:
    if data[p]==0 and cat(data[r])==i:
      W.append(("'%s = %u" % (r,data[r]), "not %u when '%s' = %d" % (data[r],p,data[p])))
print("%d content warnings" % len(W))
print(end=out,sep='',*("%s: %*s -- normally %s\n" % (w if len(k)==2 else e, max(len(k[0]) for k in W), k[0],k[1]) for k in sorted(W)))
