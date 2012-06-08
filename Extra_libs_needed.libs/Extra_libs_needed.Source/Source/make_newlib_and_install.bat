echo off

cls

echo PLEASE NOTE: these libs may now be rather old, but hopefuly will still work (things do change)
echo maybe download latest libogc source and add the changes yourself. (compare just aesndlib.c/h)
echo ---
echo However you can just use the compiled libs provided - take a look in the 'Extra_libs_needed.libs\libogc' dir
echo ---

cd libogc-src-1.8.11
make wii
make install
pause