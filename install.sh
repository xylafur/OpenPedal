#!/bin/bash

if ! which wget ; do
    echo "Must have wget installed!"
    exit 1
done

printf "######################################################################\n"
printf "#   Installing ARM GCC toolchain\n"
printf "#\n"
printf "#   This will put the toolchain binary files (compiler, etc) into"
printf "# ~/.local.bin\n"
printf "######################################################################\n\n\n"

# Note that this install is for x86_64 linux only.  See the ARM website for
# a version for your arch/OS if it is different
wget -O gcc-arm-none-eabi-9-2020-q2-update-x86_64-linux.tar.bz2 'https://developer.arm.com/-/media/Files/downloads/gnu-rm/9-2020q2/gcc-arm-none-eabi-9-2020-q2-update-x86_64-linux.tar.bz2?revision=05382cca-1721-44e1-ae19-1e7c3dc96118&la=en&hash=D7C9D18FCA2DD9F894FD9F3C3DC9228498FA281A'

tar -xvjf gcc-arm-none-eabi-9-2020-q2-update-x86_64-linux.tar.bz2

mv gcc-arm-none-eabi-9-2020-q2-update ${HOME}/.local

if [ ! -d ${HOME}/.local/bin ]; then
    mkdir ${HOME}/.local/bin
fi

for each in ${HOME}/.local/gcc-arm-none-eabi-9-2020-q2-update/bin/* ; do
    ln -s $each ${HOME}/.local/bin/$(basename $each)
done

# If ~/.local/bin is already in their path then don't need to add it!
if ! which arm-none-eabi-gcc ; then
    echo PATH=${PATH}:${HOME}/.local/bin >> ${HOME}/.bashrc
    echo "Run 'source ${HOME}/.bashrc' to complete the process"
fi

rm -rf gcc-arm-none-eabi-9-2020-q2-update-x86_64-linux.tar.bz2

printf "\n\nNote: You may also need to install libcurses\n\n"

