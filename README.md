# uStepper
To add hardware support for uStepper in the Arduino IDE (1.6x) do the following:
 - Open Arduino
 - Open preferences
 - Almost at the bottom there is a field stating: "Additional Boards Manager URLs" insert this url: https://raw.githubusercontent.com/uStepper/uStepper/master/package_ustepper_index.json
 - Press OK
 - Go into tools -> Boards and press "Boards Manager"
 - Go to the bottom (after it has loaded new files) select "uStepper by ON Development IVS" and press install

You have now added uStepper hardware support and should be able to select uStepper under tools -> boards.

For now, only a single example sketch showing uSteppers functionalities is available. This sketch requires two external libraries:
 - For encoder: http://ams.com/eng/Support/Demoboards/Position-Sensors/Rotary-Magnetic-Position-Sensors/AS5600-POTUINO
 - For stepper: http://www.airspayce.com/mikem/arduino/AccelStepper/

We are working on our own libraries, but since the hardware is ready almost two months early, we are releasing this preliminary sketch to give our backers something to play with.

For more information visit www.ustepper.com
