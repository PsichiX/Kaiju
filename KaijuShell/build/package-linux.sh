#!/usr/bin/env bash

FILEPATH="kaiju-shell-1.0-linux"

echo "Prepare package files..."

echo "- package binaries..."
mkdir ${FILEPATH}
cp templates/linux/INSTALL ${FILEPATH}/
mkdir ${FILEPATH}/bin
cp ../bin/Linux/Release/kaiju ${FILEPATH}/bin/
cp templates/linux/setupenv.sh ${FILEPATH}/bin/

echo "- package STD scripts..."
mkdir ${FILEPATH}/std
cp ../../test/Atom.kj ${FILEPATH}/std/
cp ../../test/Object.kj ${FILEPATH}/std/
cp ../../test/Bool.kj ${FILEPATH}/std/
cp ../../test/Int.kj ${FILEPATH}/std/
cp ../../test/Float.kj ${FILEPATH}/std/
cp ../../test/String.kj ${FILEPATH}/std/
cp ../../test/Array.kj ${FILEPATH}/std/
cp ../../test/Pointer.kj ${FILEPATH}/std/
cp ../../test/Buffer.kj ${FILEPATH}/std/
cp ../../test/OS.kj ${FILEPATH}/std/

echo "Make package archive..."
mkdir output
tar -cvzf output/${FILEPATH}.tar.gz ${FILEPATH}/

echo "Cleanup..."
rm -rf ${FILEPATH}

echo "Done!"

