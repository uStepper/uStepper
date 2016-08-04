# uStepper

To add hardware support for uStepper in the Arduino IDE (1.6x) do the following:
 - Open Arduino
 - Open preferences
 - Almost at the bottom there is a field stating: "Additional Boards Manager URLs" insert this url: https://raw.githubusercontent.com/uStepper/uStepper/master/package_ustepper_index.json
 - Press OK
 - Go into tools -> Boards and press "Boards Manager"
 - Go to the bottom (after it has loaded new files) select "uStepper by ON Development IVS" and press install

You have now added uStepper hardware support and should be able to select uStepper under tools -> boards.

To add the uStepper library do the following:
 - Download this repository as a zip file
 - Open the Arduino IDE
 - Select sketch from the top menu
 - Select include library
 - Select add .ZIP library
 - Navigate to where you downloaded the zip file to and select
 
MAC users should be aware, that OSX does NOT include FTDI VCP drivers, needed to upload sketches to the uStepper, by default. This driver should be downloaded and installed from FTDI's website:
	
	http://www.ftdichip.com/Drivers/VCP.htm

The uStepper should NOT be connected to the USB port while installing this driver !
This is not a problem for windows/linux users, as these drivers come with the arduino installation.

## Change Log

0.4.3:

- Fixed bug where initial deceleration phase (used when changing speed setting or changing directionwhile motor is moving), would never be entered, causing motor to accelerate

0.4.2:
- Fixed bug with setHome() causing getAngleMoved() to return alternating values (offset by exactly 180 degrees) between resets.
- Fixed bug in runContinous(), where direction of the motor did not get updated if the function was called with the motor standing still.

0.4.1:

- Fixed bug with getAngleMoved() returning alternating values (offset by exactly 180 degrees) between resets.
- Added keywords
- Updated pin connection description for Drop-in example 

0.4.0:

- Added Drop-in feature to replace stepsticks with uStepper for error correction
- Fixed bug in stepper acceleration algorithm, making the motor spin extremely slow at certain accelerations. Also this fix reduced the motor resonance
- Implemented an IIR filter on the speed measurement, to smooth this out a bit.

0.3.0:

- Added support for speed readout
- Added support for measuring the shaft position with respect to a zero reference. (absolute within multiple revolutions)

0.2.0:

- Complete rewrite of the stepper algorithm in assembler
- Changed from fixed point to floating point variables, due to the need for more precision
- Removed the getSpeed() method, as it didn't work, and therefore it would make more sense to remove it
  and re-add it when i get the time to fix it
- Added a few doxygen comments
- Added a new method (getStepsSinceReset()), which returns all steps performed since reset of the uStepper.
  positive values corresponds to steps in clockwise direction, while negative values corresponds to steps
  in counterclockwise direction.	

0.1.0:	

- Initial release

## Documentation
We have now added documentation for the uStepper library, you can find it here:
http://ustepper.com/docs/html/index.html

The library contains support for driving the stepper, measuring temperature and reading out encoder data. Two examples are included to show the functionality of the library.
The library is supported in Arduino IDE 1.6.7.

For more information, visit www.ustepper.com

### Closed loop
Closed loop is well on it's way, but we are still not far enough to release a beta yet. We are confident however, that we will be ready no later than at the end of August. For now you can watch a short video demo showing uStepper (with feedback) acting as a drop-in replacement on a 3D printer:
https://youtu.be/wp6koQZXqkw


<a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/"><img alt="Creative Commons License" style="border-width:0" src="https://i.creativecommons.org/l/by-nc-sa/4.0/88x31.png" /></a><br /><span xmlns:dct="http://purl.org/dc/terms/" property="dct:title">uStepper</span> by <a xmlns:cc="http://creativecommons.org/ns#" href="www.ustepper.com" property="cc:attributionName" rel="cc:attributionURL">ON Development</a> is licensed under a <a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/">Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License</a>.
