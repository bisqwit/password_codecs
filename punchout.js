'use strict'
var s = require('./sprintf') // Download this file from: https://raw.githubusercontent.com/alexei/sprintf.js/master/src/sprintf.js
var hash = [6,3,5,7,9, 0,1,4,2,8], offs = [1,2,0,-1, 2,0,1,-1, 0,1,2,-1, -1,-1,-1,-1]
var stats = {}, p = [0,0,0,0,0, 0,0,0,0,0]
for(var pass=0; pass<=9999999999; ++pass)
{
  // Convert password into an array of digits
  for(var v=pass, n=9; n>=0; --n) { p[n] = v%10; v = Math.floor(v/10) }
  // Undo the first round of encryption: Substitution cipher
  for(var n=0; n<10; ++n) if( (p[n] -= hash[n]) < 0 ) p[n] += 10
  // Undo the second round of encryption: Bit rotation
  var roll = p[9] & 3
  for(var r = roll; r--; )
  {
    for(var n=9; n>=1; n--)
      p[n] = (p[n] >> 1) | ((p[n-1] & 1) << 2)
    p[0] = (p[0] >> 1) | ((p[9] & 2) << 1)
  }
  // Combine the ten 3-bit units into five 6-bit units
  for(var n=0; n<5; ++n) p[n] = (p[n*2] << 3) | p[n*2+1]
  // Data extraction and range checking
  var wins10 = (p[0] & 0xC) | (p[1] & 3); if(wins10 >= 10) continue
  var wins01 = (p[3] & 0xC) | (p[2] & 3); if(wins01 >= 10) continue
  var losses = (p[4] >> 4) & 3
  var ko10   = (p[2] & 0xC) | (p[3] & 3); if(ko10 >= 10) continue
  var ko01   = (p[1] & 0xC) | (p[0] & 3); if(ko01 >= 10) continue
  var r6 = (p[4] >> 2) & 3; if(roll != offs[r6*4 + losses]) continue // This verifies roll and also ensures r6<3 and losses<3.
  // Checksum validation
  if(((wins10+wins01+losses+ko10+ko01) ^ 0xFF)
  != ((p[0] & 0x30) | ((p[3] & 0x30) << 2) | ((p[1] & 0x30) >> 2) | ((p[2] & 0x30) >> 4))) continue
  // Verify that the number of wins is at least 3,
  // and that the number of KOs is not larger than the number of wins.
  // This check doesn't quite work right when number of wins < 10, but it's exactly what the game does.
  var A = wins10
  if(A == 0) { A = wins01; if(A < 3) continue }
  if(A < ko10 || (A == ko10 && wins01 < ko01)) continue

  var name = s.sprintf('win=%d%d lose=%d ko=%d%d circuit=%d', wins10,wins01, losses, ko10,ko01, r6)
  if(typeof stats[name] === 'undefined')
      stats[name] = [pass]
  else
      stats[name].push(pass)
  console.log(s.sprintf("%s: %010d", name, pass))
}

var totalsets=0, totalpass=0;
for(var name in stats)
{
  var line = name, t = stats[name], b = t.length
  for(var a=0; a<b; ++a) { line += ' '; line += s.sprintf('%010d', t[a]) }
  console.log(line)
  totalsets += 1
  totalpass += b
}
console.log(s.sprintf('%u settings, %u passwords', totalsets, totalpass))
