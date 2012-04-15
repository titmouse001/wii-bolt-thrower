echo off
cls
echo PLEASE NOTE: Rename folder (remove JUST_SOUND part to match the line below) to compile - these libs may now be rather old, best just install the 
echo aesndlib.a / aesndlib.h files to your C:\devkitPro\libogc\ path whatever that is.
echo or download latest libogc source and add the changes (should be a simple thing to do)
echo ---
echo However you can just use the compiled libs provided - take a look in the 'Extra_libs_needed.libs' dir
echo ---
pause
cd libogc-src-1.8.8_has_changes_needed_for_game\
make wii
pause
make install
pause