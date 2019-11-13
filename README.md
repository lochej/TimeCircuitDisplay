# What is it ? 

This is my version of the famous Time Circuit Display showcased in **Back to the Future** movies.

This is a piece of electronics from the *De Lorean*.

You have the hardware files for **Multisim and Ultiboard** CAD software.

**PDF are provided for documentation of the design.**

Also, I made a Library to drive the display using a Linux ARM **I2C** from userspace.

It is available in the **sw/TimeCircuitDisplay** source files.

It used the **/dev/i2c-1** specified in the **ht16k33.c** file.

The program takes an argument which is the brightness of the display:
1. 0=Minimum brightness
2. 15=Maximum brightness

```
./TimeCircuitDisplay 15
```

