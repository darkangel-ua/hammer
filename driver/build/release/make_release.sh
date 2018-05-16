#!/bin/bash
set -e

rm -f *.tar.bz2 *.zip

dhammer release address-model=64
strip -s ./hammer
version=$(./hammer --version)
tar -cjf hammer-${version}-x64.tar.bz2 ./hammer
rm ./hammer

dhammer release address-model=32
strip -s ./hammer
version=$(./hammer --version)
tar -cjf hammer-${version}-x32.tar.bz2 ./hammer
rm ./hammer

dhammer release gcc-5w32 target-os=windows address-model=32
version=$(wine ./hammer.exe --version | tr -d "\n" | tr -d "\r")
zip hammer-${version}-win32.zip ./hammer.exe
rm ./hammer.exe

echo "Successfull"
