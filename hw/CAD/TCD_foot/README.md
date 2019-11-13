# Time Circuit Display Foot CAD file

## What is it ?
This is the STL source file for a pretty clean and simple foot the the TimeCircuitDisplay PCB.
There is an inclination of 25° of the display so it is better readable on a table.

![Time Circuit Display Foot](https://github.com/lochej/TimeCircuitDisplay/blob/master/hw/CAD/TCD_foot/TCD_foot_LEFT.JPG)

This only is the **LEFT** foot when facing the PCB, so you need to **mirror** the part in you slicer to create the **RIGHT** foot.

**You need to do so to make sure that the grove for the 7-segments displays's pins are not in the way. **

*The part is not symmetric.*

## Printing parameters
Simply put it in you favorite slicer and print with following parameters:
1. 0.4mm nozzle width 
1. 0.2 mm layer height 
1. First layer percentage : 110%
1. Infill : Cubic 20%
1. 3 perimeter walls
1. 3 top and bottom layers

*Print it flat to save on support !*
This part is a **tight fit** and should not require any glue to stay fixed on the PCB.

