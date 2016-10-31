struct SolarJetman {
    // All variables are u32 rather than the i8 or bool that I wanted,
    // because this language does not seem to support generic closures.
    score:u32, lives:u32, level:u32, map:u32, supermap:u32, podtype:u32, shield:u32, thruster:u32
}
impl SolarJetman {
    fn decode(&mut self, password: &std::string::String) -> bool {
        let mut bytes = [0; 6];
        let mut ok    = true;
        // Decipher
        for (k,c) in password.chars().enumerate() {
            match "BDGHKLMNPQRTVWXZ".find(c) {
                Some(position) => bytes[k%6 as usize] += (position << (4-k/6*4)) as u8,
                None           => ok = false
        }   }
        // Read data and verify checksum
        bytes[3] == self.have(&mut bytes, |byte,shift,var,mask,range,scale| {
            let t = (*byte >> shift) & mask;
            if t >= range  { ok = false }
            *var = t as u32 * scale + if scale != 1 { *var } else { 0 }
        }) && ok
    }
    fn encode(&mut self) -> std::string::String {
        let mut bytes = [0; 6];
        // Insert data. Checksum will be written last.
        bytes[3] = self.have(&mut bytes, |b,shift,var,_,range,scale| *b += ((*var / scale) as u8 % range) << shift );
        // Encipher
        (0..12).fold("".to_string(),
            |x,k| x + &(['B','D','G','H','K','L','M','N','P','Q','R','T','V','W','X','Z']
                        [((bytes[k % 6] >> (4-k/6*4)) & 15) as usize]).to_string())
    }
    fn have<F>(&mut self, b:&mut[u8], mut d:F) -> u8 where F: FnMut(&mut u8,u8,&mut u32,u8,u8,u32) {
        // d is a functor that either reads b[] and writes to self.things, or the opposite.
        d(&mut b[1],4,&mut self.score,15,10, 1);   d(&mut b[1],0,&mut self.score,   15,10, 10000);
        d(&mut b[5],4,&mut self.score,15,10, 10);  d(&mut b[5],0,&mut self.score,   15,10, 100000);
        d(&mut b[0],4,&mut self.lives,15,16, 1);   d(&mut b[0],0,&mut self.score,   15,10, 1000);
        d(&mut b[4],4,&mut self.score,15,10, 100); d(&mut b[4],2,&mut self.podtype,  3,3,  1);
        d(&mut b[4],1,&mut self.shield,1,2,  1);   d(&mut b[4],0,&mut self.thruster, 1,2,  1);
        d(&mut b[2],2,&mut self.level, 63,64,1);
        d(&mut b[2],1,&mut self.supermap,1,2,1);   d(&mut b[2],0,&mut self.map,      1,2,  1);
        (b[5] as u32 + (((b[0] ^ b[1]) as u32 + b[2] as u32) ^ b[4] as u32) * 0x101 / 0x100) as u8
}   }

use std::env;

fn try_decode(pass: std::string::String) {
    let mut test = SolarJetman{score:0,lives:0,level:0,map:0,supermap:0,podtype:0,shield:0,thruster:0};
    let ok       = test.decode(&pass);
    println!("Password {}: {}, Score: {:06}, level: {:02}, lives {} map {} supermap {} pod {} shield {} thruster {}",
             pass, ["failure", "success"][ok as usize],
             test.score, test.level, test.lives, test.map, test.supermap, test.podtype, test.shield, test.thruster);
}
fn main() {
    // Try decoding a password given on commandline
    match env::args().nth(1) {
        Some(password) => { try_decode(password); }
        None           => { println!("To decode, execute: solarjet <password>") }
    }
    // Generate a password for these specs:
    let pass = SolarJetman{score:0,lives:4,level:46,map:0,supermap:0,podtype:0,shield:0,thruster:0}.encode();
    // Try decoding what we just generated
    try_decode(pass);
}
