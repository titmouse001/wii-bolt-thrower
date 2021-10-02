# wii-bolt-thrower

Wii Bolt Thrower is a Space shoot 'em up video game for the Wii.  
Developed under the Wii homebrew platform and is released free for all.

---
Built with DevKitPro using Windows & MS Visual Studio editor, with the following settings:-

```
Property pages for m/s VS
- Debugging
set command to: C:\devkitPro\devkitPPC\bin\wiiload.exe 
set Command Arguments to: BoltThrower.dol
- NMake
set Build command line to: make -r 2>&1 | sed -e 's/\(.[a-zA-Z]\+\):\([0-9]\+\):/\1(\2):/
Rebuild all: make clean && make -r 2>&1 | sed -e 's/\(.[a-zA-Z]\+\):\([0-9]\+\):/\1(\2):/
Clean Command line: make clean
```
```
Configute a new windows variable using 'System Properties' Advanced tab.
Add new Sytem variable called 'WIILOAD' using the value 'tcp:192.168.1.nn'
lower case for ctp! (or whatever your IP is)
Needs VS restarting.
```
*Launch using visual studio by pressing F5 - this will run ‘wiiload.exe’ on the pc & pipe the compiled code (BoltThrower.dol) over to the Wii, the homebrew channel needs to be running to recieve.*

### Libraries used
```
LIBS := -lfat -lpng -lz -lmodplay -lwiiuse -lbte -lasnd -logc -lm 
```

### Development background
The first public released was in early December 2010 as a proof of concept demo for anyone to download via the 'Home brew website' or via the home brew browser.  Development ended in 2012, with the final release on the 26/11/2012

#### Releases
- 0.01	08/12/2010	First Demo
- 0.12	12/12/2010	Demo Update
- 0.23	18/12/2010	Demo Update
- 0.34	29/01/2011	First Game Release
- 0.35	05/02/2011	User requested feature update
- 0.36	18/03/2011	Release of feature update
- 0.47	23/04/2011	Release of feature update
- 0.58	08/07/2011	Release of feature update
- 0.59	09/07/2011	Release of feature update
- 0.60	29/10/2011	Release of feature update
- 0.61	29/01/2012	Release of feature update
- 0.62	21/01/2012	Minor release (music playback fix)
- 0.70	28/07/2012	Release of feature update
- 0.71	26/11/2012	Final release

----

see http://wiibrew.org/wiki/BoltThrower for extra details
Exported from code.google.com/p/wii-bolt-thrower
