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
#define HEATER_THERMOMETER_ADDRESS \
  0x28, 0x47, 0xAB, 0x57, 0x04, 0xE1, 0x3C, 0xC1

// Values for the position of the servo while open and closed.
// Needs to be changed to your specific setup, as cheap no-name servos
// tend to vary a lot regarding its position to a given signal.
#define SERVO_POS_OPEN 65
#define SERVO_POS_CLOSED 120

// Different servos take different amounts of time to move to their destination.
// Because I don't want the Servo jitter all the time, I'm "attaching" and
// "detaching" before and after every move. The defined time here ist
// effectively a delay which gives the servo time to move. If you need more than
// ~750ms, you should expect the software to noticably "lag" when the servo is
// moving.
// *** DON'T USE A VALUE HIGHER THAN 3500 IF YOU'RE USING THE WATCHDOG ***
#define SERVO_MOVING_MS 500

// Definitions of pins on which buttons are connected
#define BUTTON_SELECT 3
#define BUTTON_UP 4
#define BUTTON_DOWN 2

// Definitions of pins where relays are for heater and fan are connected
#define HEATER_PIN 7
#define FAN_PIN 6
#define SERVO_PIN 9
#define EXTERNAL_FAN_PIN 11

// Definition of whether to use serial output logging or not, and how many
// milliseconds between each write.
// Since the temperature doesn't change quickly, default is 10 seconds per
// write. WARNING: Opening a serial monitor to collect data will auto-reset the
// Arduino Nano in most cases. Connect the logger before setting any
// temperatures or modes or they'll be reset. Data format is Temp Target, Case
// Temp, Heater Temp, Mode, e.g., 30, 25,100,HEAT

#define SERIAL_LOGGING false
#define SERIAL_RATE_MS 10000


// Definitions of temperatures which will trip overtemperatures errors.
// Needs to be adjusted to your heater, as some heaters will heat the air a
// little higher than others. Just remember to check the maximum temperatures
// of your materials, especially for the flap, as it gets directly hit with
// hot air!

// Set TEMPERATURE_SCALE_C = false to enable Fahrenheit. Set
// CASE_TEMP_OVERTEMP_LIMIT_F and HEATER_TEMP_OVERTEMP_LIMIT_F to sane values
// for your setup. Currently these closely match the Celsius settings.

// NOTE: KEEP INITIAL_TARGET AND MAX_TEMP_SETTING VALUES IN 5 DEGREE INCREMENTS.
// The program only changes in 5 degree steps.

#define TEMPERATURE_SCALE_C true

// Limits in Celsius
#define CASE_TEMP_OVERTEMP_LIMIT_C 50
#define HEATER_TEMP_OVERTEMP_LIMIT_C 100
#define INITIAL_TARGET_C 20
#define MAX_TEMP_SETTING_C 45
#define TOLERANCE_C 3

// Limits in Fahrenheit
#define CASE_TEMP_OVERTEMP_LIMIT_F 120
#define HEATER_TEMP_OVERTEMP_LIMIT_F 210
#define INITIAL_TARGET_F 70
#define MAX_TEMP_SETTING_F 115
#define TOLERANCE_F 3

// Normally the temperature sensors  report -127°C (-196°F) when they are not
// wired correctly or are defective, so we check for sensor-readings below -20°C
// (-4°F), as I dont expect anyone actually trying to use this system below that
// temperature. If you do, adjust the threshold accordingly.
// *** DO NOT ADJUST IF NOT NECCESSARY FOR YOUR SETUP ***
#define SENSOR_ERROR_UNDERTEMP_C -20
#define SENSOR_ERROR_UNDERTEMP_F -4

// SAFETY FEATURE

// As a safety feature a watchdog timer can be used to reset the Arduino in case
// of a software-lock. In the worst case the software would get stuck while the
// heater is running, which could result in high temperatures inside the
// enclosure and ultimately, if lots of things go wrong, in a fire. To prevent
// this you can use a watchdog, which will restart the Arduino if the watchdog
// is not reset after a specified time, in this case 4 seconds. As there is a
// bug in the Arduino Nanos old Bootloader, you should only use this with a Nano
// which already has the new Bootloader. In my experience, all of the cheap
// clone models use the old one. Only the original Arduino Unos are shipping
// with the new one. You can update your Bootloader yourself, i.e. with another
// Arduino. See:
// https://docs.arduino.cc/built-in-examples/arduino-isp/ArduinoISP/
// *** DO NOT USE WATCHDOG IF YOUR BOARD IS nanoatmega328, this is the old
// Bootloader! ***
// *** ONLY USE WITH nanoatmega328new ***
// *** YOU COULD SOFTBRICK YOUR BOARD ***

// #define USE_WATCHDOG
