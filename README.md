# Open Pedal
Open Source implementation of a Digital Guitar Effects Pedal.

## Building
You need the ARM GNU toolchain.  The file `install.sh` will download the 2020q2
version of the toolchain from ARM's website as a tarball, uncompress it and
move the binaries to somewhere they can be executed.

## Loading the image on the board

If you have OpenOCD installed 'make program' can be used to flash the .bin file to the board. OpenOCD must be installed with stlink enabled.


# Thank You
I used many open source projects and tutorials to help me with this project.

## Mike Szczys
[szczys/stm32f0-discovery-basic-template] (https://github.com/szczys/stm32f0-discovery-basic-template)

I used this project for the basic libraries STM libraries and the nice Makefile
that Mike put together.  This allowed me a base project to start with using
OpenOCD to program the board.

## Matthew Blythe
[mblythe86/stm32f0-discovery-basic-template] (https://github.com/mblythe86/stm32f3-discovery-basic-template)
I used Matt's fork of the project as a reference for adding OpenOCD debugging
using the SWD on the STM32F0 chip.  This enabled me to get through quite a few
bugs!

## Sash Bremec
[Sash's Website] (http://www.sasabremec.com/)
Sash had a nice tutorial on using USART with an STM32F0308 MCU that helped me
through a problem I was having.


## Nano Age
[Nano Age's Website] (http://blog.nano-age.co.uk/)
I'm not sure of the name of the author of Nano Age, but they provided a nice
detailed guide detailing how to write the systemcalls that allow you to call
printf (and other stdlib functions) on a baremetal card.
You can checkout that tutorial [here](https://sites.google.com/site/stm32discovery/stm32-discovery-up-close)
