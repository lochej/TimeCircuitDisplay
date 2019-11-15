# Driving program for Linux SBCs

This is a driving program written in ARM DS-5 IDE (Eclipse IDE with CDT extensions) and compiled with GCC 4.x minimum.

*You have to enable C dialect = C99.*


## How to compile the program

Either open the project in ARM DS-5 by setting you workspace to the **sw** folder. 
Then selecting **File->Open Project From File System** and selecting the **sw/TimeCircuitDisplay** directory.

Or compile it yourself with your prefered compiler.

## How to run the program

Connect the Time Circuit Display to you SBC i2c port.

The program uses address default 0x70 for the HT16K33 chip.
Make sure yours is set to the same, or change it in the program:

**Hint**
In the main function, modify the **hdev.address=0** value to your A2:A1:A0 bits configured on the HT16K33 adafruit breakout.

**Program Arguments** 

1. argument 1 : 0 = turns OFF the display, 1 to 16 sets the brightness of the display, higher values will just saturate to 16.
1. argument 2 : your i2c device: if not provided by default it will use /dev/i2c-1 