A customizable desk lamp controlled with a 10 cents coin that acts as a touch control.
You can switch the light on and off with a single touch, or adjust the brightness keeping the touch.

The lamp uses an Atmega 2560 Pro microcontroller that can be programmed like any other Arduino board, so you can use the default program or make any changes you like!
Each pin of the atmega controls a single LED, so you can adjust the brightness of each led individually using PWMs.

# What's needed
- an Atmega 2560 Pro board (a normal Atmega 2560 would be too big and will not fit into the lamp);
- 9 (or up to 15) 3V LEDs;
- a medium/high resistor (e.g. 1MΩ);
- some wires and a soldering machine to realize the connections;
- strip connectors;
- a 3D printer.

# Realisation
## 1: Printing
In the _Stl_ folder you will find tree files. You have to print each file once.

## 2: Touch sensor
The touch detection uses the [CapacitiveSensor](https://www.arduino.cc/reference/en/libraries/capacitivesensor/) library. You can find the schematic in the _Schematics_ folder.
Connect the sensor between the 49 and 51 pins. Follow the schematic to make sure you attach the sensor in the right way.

## 3: LEDs
There are tree pieces to be print: The basement, the shell and a stand for the LEDs. In the _Pictures_ folder there is a picture called _Led_Placement_ to figure out how to stick the LEDs to the stand. This helps to keep the led disposed quite regularly. You can add LEDs up to a total of 15, you just have to make sure to connect them to the correct pins (from 2 to 13 and from 44 to 46): in fact the LEDs are powered in PWM to avoid additional resistors, and these are the only pins capable of managing PWM.

## 4: Assembling
-Once you've glued all the led to the stand, make sure the wires are the right length to ensure that everything fits inside the shell. 
-Glue the Atmega board to the basement (i've glued it upside down to cover the colored light of the on-board LEDs, otherwise cover them with some black tape).
-Glue the stand to the basement using the guide holes. 
-Glue the coin to the top of the shell and that's all!




