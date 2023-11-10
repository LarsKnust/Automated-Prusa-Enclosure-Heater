/*
##########################################################
AUTOMATED HEATING SYSTEM FOR ORIGINAL PRUSA ENCLOSURE
Originally written by user @lars and published on
printables.com in august 2023.
##########################################################
######################### CONFIG #########################
##########################################################
This file is meant to configure the user configurable
parameters of the software. If you're just starting out
with this, make sure to at least adjust the addresses of
your temperature sensors, as you will otherwise get an
error! The address that's written here is just an example.
##########################################################

**********************************************************
* YOU NEED TO RENAME THIS FILE TO configuration.h        *
* (REMOVE THE -sample) TO USE THIS SOFTWARE. IT WILL NOT *
* WORK WIHTOUT RENAMING!                                 *
**********************************************************
*/

// Definitions of the OneWire addresses of the thermometers.
// *You need to find the addresses of your thermometers yourself!*
// If you don't know how you can take a look at the examples of
// the DallasTemperature library.
#define CASE_THERMOMETER_ADDRESS 0x28, 0x23, 0x41, 0x57, 0x04, 0xE1, 0x3C, 0xB9
#define HEATER_THERMOMETER_ADDRESS 0x28, 0x47, 0xAB, 0x57, 0x04, 0xE1, 0x3C, 0xC1

// Values for the position of the servo while open and closed.
// Needs to be changed to your specific setup, as cheap no-name servos
// tend to vary a lot regarding its position to a given signal.
#define SERVO_POS_OPEN 65
#define SERVO_POS_CLOSED 120

// Different servos take different amounts of time to move to their destination.
// Because I don't want the Servo jitter all the time, I'm "attaching" and
// "detaching" before and after every move. The defined time here ist effectively a delay
// which gives the servo time to move. If you need more than ~750ms, you should
// expect the software to noticably "lag" when the servo is moving.
#define SERVO_MOVING_MS 500

// Definitions of pins on which buttons are connected
#define BUTTON_SELECT 3
#define BUTTON_UP 4
#define BUTTON_DOWN 2

// Definitions of pins where relays are for heater and fan are connected
#define HEATER_PIN 7
#define FAN_PIN 6
#define SERVO_PIN 9

// Definitions of temperatures which will trip overtemperatures errors.
// Needs to be adjusted to your heater, as some heaters will heat the air a
// little higher than others. Just remember to check the maximum temperatures
// of your materials, especially for the flap, as it gets directly hit with
// hot air!
#define CASE_TEMP_OVERTEMP_LIMIT 50
#define HEATER_TEMP_OVERTEMP_LIMIT 100

// Normally the temperature sensors  report -127°C when they are not wired
// correctly or are defective, so we check for sensor-readings below -20°C,
// as I dont expect anyone actually trying to use this system below that temperature.
// If you do, adjust the threshold accordingly.
// *** DO NOT ADJUST IF NOT NECCESSARY FOR YOUR SETUP ***
#define SENSOR_ERROR_UNDERTEMP -20