@echo off
set fonttool=fonttool.exe
set fontdir=fonts

REM ensure the folder exists
mkdir %fontdir%

mkdir %fontdir%\small

REM mkdir %fontdir%\medium

mkdir %fontdir%\large

REM and delete any old files
del %fontdir%\*.tga
del %fontdir%\*.ftab
del %fontdir%\*.fraw

Rem only need the 'basic Latin' ASCII set 
set CharSet=Latin

REM set extracharsets=greek-coptic,cyrllic
REM set unicodecharsets=cjk-unified,hangul-syllables,hiragana,katakana

REM %fonttool% -f "Bauhaus 93" -h 22 -o %fontdir%\small\Bauhaus93 -c %CharSet%
REM %fonttool% -f "Bauhaus 93" -h 30 -o %fontdir%\medium\Bauhaus93 -c %CharSet%


%fonttool% -f "Bauhaus 93" -h 50 -o %fontdir%\large\Bauhaus93 -c %CharSet%

%fonttool% -f "Arial" -h 22 -o %fontdir%\small\Arial -c %CharSet%


REM %fonttool% -f "Arial" -h 30 -o %fontdir%\medium\Arial -c %CharSet%
REM %fonttool% -f "Arial" -h 40 -o %fontdir%\large\Arial -c %CharSet%


rem %fonttool% -f "Arial Unicode MS" -h 14 -o %fontdir%\small -c %unicodecharsets%
pause
