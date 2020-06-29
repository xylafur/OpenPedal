# Open Pedal
Open Source implementation of a Digital Guitar Effects Pedal.

## Loading the image on the board

If you have OpenOCD installed 'make program' can be used to flash the .bin file to the board. OpenOCD must be installed with stlink enabled.


# Thank You
I used many open source projects and tutorials to help me with this project.

## Mike Szczys
### <a name="https://github.com/szczys/stm32f0-discovery-basic-template"></a>szczys/stm32f0-discovery-basic-template

I used this project for the basic libraries STM libraries and the nice Makefile
that Mike put together.  This allowed me a base project to start with using
OpenOCD to program the board.

## Matthew Blythe
### <a name="https://github.com/mblythe86/stm32f3-discovery-basic-template"></a>mblythe86/stm32f0-discovery-basic-template
I used Matt's fork of the project as a reference for adding OpenOCD debugging
using the SWD on the STM32F0 chip.  This enabled me to get through quite a few
bugs!

## Sash Bremec
### <a name="http://www.sasabremec.com/"></a>Sash's Website
Sash had a nice tutorial on using USART with an STM32F0308 MCU that helped me
through a problem I was having.


## Nano Age
### <a name="http://blog.nano-age.co.uk/"></a>Nano Age's Website
I'm not sure of the name of the author of Nano Age, but they provided a nice
detailed guide detailing how to write the systemcalls that allow you to call
printf (and other stdlib functions) on a baremetal card.
<a name="https://sites.google.com/site/stm32discovery/stm32-discovery-up-close"></a>You can checkout that tutorial here
