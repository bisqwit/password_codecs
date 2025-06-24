# Password Codecs

Collection of password encoders and decoders created
with the *Cracking Videogame Passwords* video series at:
https://www.youtube.com/playlist?list=PLzLzYGEbdY5nEFQsxzFanSDv_38Hz0w7B

Back in the day when games were supplied on ROM cartridges, there were no
files or memory cards to which the game could be saved.

On cartridges that did not have battery-backed SRAM, some games still
provided the save-and-load function by using *paper* as the storage:
The game state would be encoded in such a way
that the player can transcribe it down on paper.
For example, as a sequence of letters.
Later, the player can enter the password,
and if the password passes validation,
the game will load the state stored in it.

Because the password was stored in media that is subject to
not only unintentional degradation (misspelling or misreading),
but also intentional degradation (hacking and cracking),
most games employed encryption and checksumming.
The purpose of encryption and checksumming was to reduce the chances
of the player gaining an unfair advantage by entering a modified password.
Each game did this a bit differently.
[A
The act of saving the game as a password was an art unto itself.
Some games were particularly creative with how they did it.
This makes it an entertaining subject of study in my opinion.
It also provides a window into the minds and the process of game development
in the early console era, and is a way to document and appreciate
the hard work the programmers did back in the day.
Sometimes it may spotlight caveats that are still applicable to developers even today.

Personally for me, it is a way to gain closure to the hundreds of hours,
that I invested in attempting to crack the passwords in my early teens;
to close a book in an honorable and satisfying manner.

Note: The “Research by” headers list the research that was utilized in the
making of the episode. It is not a leaderboard of people who were
globally the first to crack the passwords.

The word “codec” means a combined en**co**der-**dec**oder.
These programs encode and decode passwords.

## Season 1 Episode 1: NES Mega Man 2 (Rockman 2)

Video link: http://youtu.be/0eQyYrSQPew  
Episode date: 2016-09-03  
Research by: Bisqwit, 199x

No files

## Season 1 Episode 2: NES Mega Man 3 (Rockman 3)

Video link: http://youtu.be/QnRcAyYmL0U  
Episode date: 2016-09-04  
Research by: Bisqwit, 199x

No files

## Season 1 Episode 3: NES Gremlins 2

Video link: http://youtu.be/iajgztvLxGc  
Episode date: 2016-09-05  
Research by: Bisqwit, 2016

* multigrep.php (PHP): A tool for searching files for strings with a character set offset and possibility of sparse strings

## Season 1 Episode 4: NES Bubble Bobble

Video link: http://youtu.be/PIu9J_CD818  
Episode date: 2016-09-06  
Research by: Bisqwit, 2016

* bubbob-password.php (PHP): A tool that prints the list of all passwords that can be decoded by the game, producing 1719 lines of output.
* bubbob-output.txt.gz: The output printed by this program.

## Season 1 Episode 5: NES Mega Man 4 (Rockman 4)

Video link: http://youtu.be/PFcxmdH_4ac  
Episode date: 2016-09-07  
Research by: unknown, 200x

* mm4minimal1.php (PHP): A minimal tool for generating a random valid password as an image.
* mm4minimal1.py (Python): A Python version of the same.
* megaman4-model2.png (PNG): An image file used by the above two programs.

## Season 1 Episode 6: NES Castlevania II — Simon’s Quest (Dracula II — Noroi no Fuuin)

Video link: http://youtu.be/_3ve0YEQEMw  
Episode date: 2016-09-10  
Research by: unknown, 200x  
Research by: Bisqwit, 200x

* simonspass.cc (C++): An encoder and decoder for the passwords. No main program.

## Season 1 Episode 7: NES Bomberman

Video link: http://youtu.be/PPfPmP44-r8  
Episode date: 2016-09-15  
Research by: Bisqwit, 2016

* bomberpass.cc (C++): An encoder and decoder for the passwords.

## Season 1 Episode 8: NES Solar Jetman — Hunt for the Golden Warpship

Video link: http://youtu.be/Ex1iFZuUdJ4  
Episode date: 2016-09-27  
Research by: TNSe, 2004  
Research by: Bisqwit, 2016

* solarpass.rs (Rust): An encoder and decoder for the passwords.

## Season 1 Episode 9: NES River City Ransom (Downtown Nekketsu Monogatari)

Video link: http://youtu.be/cDvHy4RtAek  
Episode date: 2016-10-02  
Research by: Bisqwit, 2016

* rcrpass.cc (C++): An encoder and decoder for the passwords. UTF-8 combining diacritics and accents supported.

## Season 1 Episode 10: NES The Guardian Legend (Guardic Gaiden)

Video link: http://youtu.be/9c848FjqUmI  
Episode date: 2016-10-30  
Research by: TNSe, 2004  
Research by: Bisqwit, 2016

* guardicpass.cc (C++): An encoder and decoder for the passwords. ASCII only.

## Season 1 Episode 11: NES Punch-Out!!

Video link: http://youtu.be/ap1YL_kGGlg  
Episode date: 2016-11-07  
Research by: Bisqwit, 2016

* punchout.js (JavaScript): A tool that prints the list of all passwords that can be decoded by the game, producing 49177 lines of output.
* punchout-js.cc (C++): A C++ translation of the JavaScript program. Completes in 1 minute, while the JavaScript program (run in Node) takes several hours to complete.
* punchout-output.txt.gz: The output printed by this program.

## Season 1 Episode 12: NES Swords & Serpents

Video link: http://youtu.be/twIR0yE8Ll4  
Episode date: 2016-12-05  
Research by: Bisqwit, 2016  

* swspass.cc (C++): An encoder and decoder for the passwords.

## Season 1 Episode 13: NES Wizards & Warriors II — Iron Sword

### Preparation video (livestream)

Video link: http://youtu.be/GVjqY3n5bTA    
Episode date: 2017-04-26    
Research by: Bisqwit, 2017

* ironswordpass.php (PHP): An encoder and decoder for the passwords.

### Actual episode

Video link: http://youtu.be/5teRgdsvwEI  
Episode date: 2019-06-22  
Research by: Bisqwit, 2017  

* ironswordpass.php (PHP): An encoder and decoder for the passwords.

## Season 2 Episode 1: NES G.I. Joe — The Atlantis Factor

Video link: http://youtu.be/7tUtO_2BknQ  
Episode date: 2019-07-11  
Research by: Bisqwit, 2019  

* gijoeatlantis.scm (Scheme): An encoder and decoder for the passwords.

## Season 2 Episode 2: NES Metroid

Video link: http://youtu.be/wiOalxCQ1Mw  
Episode date: 2019-08-31  
Research by: SnowBro, 2010; Bisqwit, 2019  

* metroid-passwords.cc (C++): An encoder and decoder for the passwords.
* metroid-map.php (PHP): The program I wrote for generating the map shown
  near the beginning of the video.

## Season 2 Episode 3: DOS Dyna Blaster (also Amiga)

Video link: http://youtu.be/JMz_PYnpJPY  
Episode date: 2019-09-14  
Research by: Bisqwit, 2019  

* dynablaster-passwords.php (PHP): An encoder and decoder for the passwords.
* dynablaster-supporting_files.zip: dosbox.conf, a commented disassembly (of
  relevant parts) and a list of coincidentally English-language passwords.

## Season 3 Episode 1: NES The Battle of Olympus

Video link: https://youtu.be/CLKLSXjWNRk  
Episode date: 2025-06-23  
Research by: Bisqwit, 2025

* olympass-decode.py (Python): A decoder for passwords
* olympass-encode.py (Python): An encoder for passwords

## Intermission (between season 1 episodes 6 and 7)

Video link: http://youtu.be/qmxZndDVgmc  
Episode date: 2016-09-13  
Thanks to: Everyone who supports me on Patreon and Paypal!  
Link to: https://patreon.com/Bisqwit  
Link to: http://iki.fi/bisqwit/donate.html

## Intermission 2 (after season 1 episode 12)

Video link: http://youtu.be/gi7bW16yWBk  
Episode date: 2016-12-05  
Thanks to: Everyone who has subscribed to my channel!  
