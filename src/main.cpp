/*
##########################################################
AUTOMATED HEATING SYSTEM FOR ORIGINAL PRUSA ENCLOSURE
Originally written by user @lars and published on
printables.com in august 2023.
Updated in november 2023.
V1.4
##########################################################

*******************************************************
** YOU NEED TO RENAME configuration-sample.h TO      **
** configuration.h (REMOVE THE -sample) TO USE THIS  **
** SOFTWARE. IT WILL NOT WORK WIHTOUT RENAMING!      **
*******************************************************
**    ALSO: GO AND CONFIGURE YOUR SETUP IN THERE!    **
*******************************************************
*/

// Include neccessary libraries
#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <DallasTemperature.h>
#include <OneWire.h>
#include <Servo.h>
#include <Wire.h>

// Include configuartion file.
// Remember to rename (see above)!
#include <configuration.h>
#include <Utils.hpp>

// Declaration of variables
// Do not set values here, they are set in configuration.h

int target;   // initial target temperature
int maxtemp;  // target temperature limit
int tolerance; // set total tolerance for target temperature
int undertemp; //set temperature limit for detecting faulty/missing sensors
char scale; // set Celsius or Fahrenheit scale

int caseovertemplimit = 0;
int heaterovertemplimit = 0;

int upperlimit;
int lowerlimit;
int caseTemp = 0;
int heaterTemp = 0;
char temp_buf[4];
char serial_temp_buf[4];
char serial_mode_buf[5];
char serial_output[17];

// Definitions of names for operating modes. Just to make it more readable.
#define MODE_IDLE 0
#define MODE_HEATING 1
#define MODE_COOLING 2
#define MODE_FAN 3

byte operatingMode = MODE_IDLE;
// Setup MODE_IDLE after startup
bool changeMode = true;

// Declaration of states of actuators
bool heaterState = false;
bool fanState = false;
bool servoState = false; // Servo closed
bool closeServo = true;

// Flags for recurring events
bool updateTarget = false;

// Timing related variables
#define TEMPERATURE_DELAY 100 // get Temperature every X second
uint32_t previousMillis_temp = 0;
uint32_t previousMillis_serial = 0;

// Parameters of display
#define i2c_Address 0x3c
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET -1    //   QT-PY / XIAO
Adafruit_SH1106G display =
    Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Parameters of thermosensors
#define ONE_WIRE_BUS 5         // Pin used for temp sensors
OneWire oneWire(ONE_WIRE_BUS); // Setup a oneWire instance
DallasTemperature sensors(&oneWire);

// OneWire device addresses of the thermometers as configured in configuration.h
DeviceAddress caseThermometer = {CASE_THERMOMETER_ADDRESS};
DeviceAddress heaterThermometer = {HEATER_THERMOMETER_ADDRESS};

// Variables for button states
byte prevButtonPressed = 0;
byte buttonPressed = 0;

// Definition of servo-object and initialization of its position
Servo servo;

// Bitmap-icons for heating, cooling and fan, stored in PROGMEM

const unsigned char PROGMEM logo24_heat[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x30, 0x60,
    0x0c, 0x18, 0x30, 0x06, 0x0c, 0x18, 0x03, 0x06, 0x0c, 0x03, 0x06, 0x0c,
    0x03, 0x06, 0x0c, 0x03, 0x06, 0x0c, 0x06, 0x0c, 0x18, 0x0c, 0x18, 0x30,
    0x18, 0x30, 0x60, 0x18, 0x30, 0x60, 0x18, 0x30, 0x60, 0x18, 0x30, 0x60,
    0x0c, 0x18, 0x30, 0x06, 0x0c, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x1f, 0xff, 0xfc, 0x1f, 0xff, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
const unsigned char PROGMEM logo24_cool[] = {
    0x00, 0x18, 0x00, 0x00, 0x18, 0x00, 0x00, 0xdb, 0x00, 0x04, 0xff, 0x20,
    0x0c, 0x7e, 0x30, 0x26, 0x3c, 0x64, 0x7e, 0x18, 0x7e, 0x3f, 0x18, 0xfc,
    0x0f, 0x99, 0xf0, 0x7d, 0xdb, 0x7e, 0x30, 0xff, 0x0c, 0x00, 0x7e, 0x00,
    0x00, 0x7e, 0x00, 0x30, 0xff, 0x0c, 0x7d, 0xdb, 0x7e, 0x0f, 0x19, 0xf0,
    0x3f, 0x18, 0xfc, 0x7e, 0x18, 0x7e, 0x26, 0x3c, 0x64, 0x0c, 0x7e, 0x30,
    0x04, 0xff, 0x20, 0x00, 0xdb, 0x00, 0x00, 0x18, 0x00, 0x00, 0x18, 0x00};
const unsigned char PROGMEM logo24_fan[] = {
    0x00, 0x3e, 0x00, 0x00, 0x7f, 0x80, 0x00, 0x7f, 0x80, 0x00, 0x7f, 0x80,
    0x00, 0x7f, 0x80, 0x00, 0x7f, 0x00, 0x00, 0x7c, 0x00, 0x78, 0x7c, 0x00,
    0xfc, 0x78, 0x00, 0xfc, 0x7f, 0xfe, 0xff, 0x67, 0xff, 0xff, 0xc3, 0xff,
    0xff, 0xc3, 0xff, 0xff, 0xe6, 0xff, 0x7f, 0xfe, 0x3f, 0x00, 0x1e, 0x3f,
    0x00, 0x3e, 0x1e, 0x00, 0x3e, 0x00, 0x00, 0xfe, 0x00, 0x01, 0xfe, 0x00,
    0x01, 0xfe, 0x00, 0x01, 0xfe, 0x00, 0x01, 0xfe, 0x00, 0x00, 0xfc, 0x00};

const unsigned char PROGMEM logo24_idle[]{
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0xc0,
    0x00, 0x00, 0x40, 0x00, 0x00, 0x80, 0x00, 0x01, 0x00, 0x00, 0x02, 0x00,
    0x01, 0xf7, 0xd5, 0x00, 0x10, 0x00, 0x00, 0x20, 0x00, 0x00, 0x40, 0x00,
    0x00, 0x80, 0x00, 0x7d, 0xf5, 0x40, 0x04, 0x00, 0x00, 0x09, 0xf7, 0xbc,
    0x11, 0x14, 0x20, 0x21, 0x14, 0x20, 0x7d, 0x17, 0x38, 0x01, 0x14, 0x20,
    0x01, 0xf4, 0x20, 0x00, 0x00, 0x00, 0x01, 0xff, 0xfc, 0x00, 0x00, 0x00};

// Error messages
#define TEMP_ERROR 0 
#define SENSOR_ERROR 1

// Functions for drawing the individual symbols on the display.

void drawHeatingSymbol()
{
  display.fillRect(97, 12, 24, 24, SH110X_BLACK);
  display.drawBitmap(97, 12, logo24_heat, 24, 24, SH110X_WHITE);
}
void drawCoolingSymbol()
{
  display.fillRect(97, 12, 24, 24, SH110X_BLACK);
  display.drawBitmap(97, 12, logo24_cool, 24, 24, SH110X_WHITE);
}
void drawFanSymbol()
{
  display.fillRect(97, 12, 24, 24, SH110X_BLACK);
  display.drawBitmap(97, 12, logo24_fan, 24, 24, SH110X_WHITE);
}
void drawIdleSymbol()
{
  display.fillRect(97, 12, 24, 24, SH110X_BLACK);
  display.drawBitmap(97, 12, logo24_idle, 24, 24, SH110X_WHITE);
}

// Setup OLED display
// This functions prints all the neccessary text on the display once.
void display_prepare()
{
  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);
  display.clearDisplay();

  display.setCursor(0, 0);
  display.print(F("Target:         Mode:"));
  display.setCursor(0, 16);
  display.print(F("Case  :"));
  display.setCursor(0, 32);
  display.print(F("Heater:"));
  display.setCursor(0, 48);
  display.print(F("Heater: OFF"));
  display.setCursor(0, 56);
  display.print(F("Fan   : OFF"));
  display.setCursor(90, 48);
  display.print(F("Vent:"));
  
  //Print initial target temperature
  display.setTextSize(2);
  display.setCursor(42, 0);
  sprintf(temp_buf, "%3d", target);
  display.print(temp_buf);
  display.print(scale);

  display.display();
}

// Turn heater on and update display
void heaterOn()
{
  if (heaterState == true) return;

  digitalWrite(HEATER_PIN, HIGH);
  display.setTextSize(1);
  display.fillRect(48, 48, 30, 8, SH110X_BLACK);
  display.setCursor(48, 48);
  display.print(F("ON"));
  display.display();
  heaterState = true;
}

// Turn heater off and update display
void heaterOff()
{
  if (heaterState == false) return;

  digitalWrite(HEATER_PIN, LOW);
  display.setTextSize(1);
  display.fillRect(48, 48, 30, 8, SH110X_BLACK);
  display.setCursor(48, 48);
  display.print(F("OFF"));
  display.display();
  heaterState = false;
}

// Turn fan on and update display
void fanOn()
{
  if (fanState == true) return;

  digitalWrite(FAN_PIN, HIGH);
  display.setTextSize(1);
  display.fillRect(48, 56, 30, 8, SH110X_BLACK);
  display.setCursor(48, 56);
  display.print(F("ON"));
  display.display();
  fanState = true;
}

// Turn fan off and update display
void fanOff()
{
  if (fanState == false) return;

  digitalWrite(FAN_PIN, LOW);
  display.setTextSize(1);
  display.fillRect(48, 56, 30, 8, SH110X_BLACK);
  display.setCursor(48, 56);
  display.print(F("OFF"));
  display.display();
  fanState = false;
}

// Set position of servo to open flap
void servoOpen()
{
  servo.attach(SERVO_PIN);
  servo.write(SERVO_POS_OPEN);
  if (servoState != true)
  {
    display.setCursor(90, 56);
    display.setTextSize(1);
    display.fillRect(90, 56, 48, 8, SH110X_BLACK);
    display.print(F("OPEN"));
    display.display();
    servoState = true;
  }
  delay(SERVO_MOVING_MS); // Allow Servo time to reach position
  servo.detach();         // Detach servo so that it wont try to move all the time
}

// Set position of servo to close flap
void servoClose()
{
  servo.attach(SERVO_PIN);
  servo.write(SERVO_POS_CLOSED);
  if (servoState != false)
  {
    display.setCursor(90, 56);
    display.setTextSize(1);
    display.fillRect(90, 56, 48, 8, SH110X_BLACK);
    display.print(F("CLOSED"));
    display.display();
    servoState = false;
  }
  delay(SERVO_MOVING_MS); // Allow Servo time to reach position
  servo.detach();         // Detach servo so that it wont try to move all the time
}

// Flash the error message and get stuck here.
// This will cause the software to essentially stop
// so that the heater can only be turned on again if
// the user power-cycles the heating system.
void showError(byte err_text_mode)
{
  heaterOff();
  fanOff();
  servoOpen();
  bool is_black = true;
  while (true)
  {
    display.clearDisplay();
    display.setCursor(0, 0);
    if (is_black == true)  {
      display.setTextColor(SH110X_WHITE);
    } else {
      display.fillRect(0, 0, 128, 64, SH110X_WHITE);
      display.setTextColor(SH110X_BLACK);
    }
    display.setTextSize(3);
    display.println(F("ERROR!"));
    display.setTextSize(1);

    if (err_text_mode == TEMP_ERROR){
      display.println(F("Overtemp. detected."));
    } else if (err_text_mode == SENSOR_ERROR) {
      display.println(F("Sensor error! Are"));
      display.println(F("they wired correctly?"));
    }

    display.println(F("Shutting down heater."));
    display.println(F("Please powercycle!"));
    display.display();
    is_black = !is_black;
    delay(1000);
    resetWatchdog();
  }
}

// Checks if there are any temperature related problems and turns off the
// heating system accordingly. There are two different cases here: Overtemp and
// sensor-error. Overtemp should be pretty self-explanatory, sensor error means
// that the sensor reports values, that are obviously false. Normally they
// report -127°C (-196°F) when they are not wired correctly or are defective, so we check
// for sensor-readings below -20°C (-4°F), as I dont expect anyone actually trying to
// use this system below that temperature. If you do, adjust the threshold
// accordingly.
void checkForTempProblems()
{
  if (caseTemp >= caseovertemplimit || heaterTemp >= heaterovertemplimit)
  {
    showError(TEMP_ERROR);
  }
  else if (caseTemp <= undertemp || heaterTemp <= undertemp)
  {
    showError(SENSOR_ERROR);
  }
}

// Checks when button is pressed UP after pressed DOWN
bool checkButton(byte button) {
  if (!digitalRead(button) == true)
  {
    if (prevButtonPressed == 0) prevButtonPressed = button;
  } else if (prevButtonPressed == button) 
  {
    prevButtonPressed = 0;
    return true;
  }
  return false;
}

// Checks if buttons were pressed and return pressed button
byte checkButtons()
{
  if (checkButton(BUTTON_UP)) return BUTTON_UP;
  if (checkButton(BUTTON_DOWN)) return BUTTON_DOWN;
  if (checkButton(BUTTON_SELECT)) return BUTTON_SELECT;
  return 0;
}

// Gets temperature readings from the sensors and draws them on the display.
void measureTemperatures()
{
  sensors.requestTemperatures();
  if(TEMPERATURE_SCALE_C == true) {
    caseTemp = sensors.getTempC(caseThermometer);
    heaterTemp = sensors.getTempC(heaterThermometer);
  }
  else {
    caseTemp = sensors.getTempF(caseThermometer);
    heaterTemp = sensors.getTempF(heaterThermometer);
  }
  // Show current temp on display
  display.fillRect(42, 16, 46, 32, SH110X_BLACK);
  display.setCursor(42, 16);
  display.setTextSize(2);
  sprintf(temp_buf, "%3d", caseTemp);
  display.print(temp_buf);
  display.print(scale);
  display.setCursor(42, 32);
  sprintf(temp_buf, "%3d", heaterTemp);
  display.print(temp_buf);
  display.print(scale);
  display.setTextSize(1);
  display.display();
}

// Adjustment of target temperature
// If up/down button was pressed, change target temp accordingly
void updateTargetTemperature()
{
  if (buttonPressed == BUTTON_UP)
  {
    target = target + 5;
    if (target > maxtemp)
    { // Limit target temperature to maxtemp
      target = maxtemp;
    }
    updateTarget = true;
    buttonPressed = 0;
  }
  if (buttonPressed == BUTTON_DOWN)
  {
    target = target - 5;
    if (target < 0)
    { // Limit target to positive temperatures
      target = 0;
    }
    updateTarget = true;
    buttonPressed = 0;
  }

  // Only write changes to display when something actually changed.
  if (updateTarget == true)
  {
    // As target changed, set new limits
    upperlimit = target + tolerance / 2;
    lowerlimit = target - tolerance / 2;

    // Remove old target temp from display
    display.fillRect(42, 0, 42, 16, SH110X_BLACK);
    display.setTextColor(SH110X_WHITE);
    display.setTextSize(2);
    display.setCursor(42, 0);
    sprintf(temp_buf, "%3d", target);
    display.print(temp_buf);
    display.print(scale);
    display.display();
    updateTarget = false; // Reset flag
  }
}

void writeSerialData() {
  //first clear the existing output string
  serial_output[0] = '\0';
  // then write the target temperature
  sprintf(serial_temp_buf, "%3d", target);
  strcat(serial_output, serial_temp_buf);
  strcat(serial_output, ",");
  // then write the case temperature
  sprintf(serial_temp_buf, "%3d", caseTemp);
  strcat(serial_output, serial_temp_buf);
  strcat(serial_output, ",");
  //then write the heater temperature
  sprintf(serial_temp_buf, "%3d", heaterTemp);
  strcat(serial_output, serial_temp_buf);
  strcat(serial_output, ",");

  // then write the mode setting
  serial_mode_buf[0] = '\0';
  switch (operatingMode)
  {
  case MODE_IDLE:
    strcat(serial_mode_buf, "IDLE");
    break;
  case MODE_HEATING:
    strcat(serial_mode_buf, "HEAT");
    break;
  case MODE_COOLING:
    strcat(serial_mode_buf, "COOL");
    break;
  case MODE_FAN:
    strcat(serial_mode_buf, " FAN");
    break;
  }
  strcat(serial_output, serial_mode_buf);
  Serial.println(serial_output);
}

void setup()
{
  // Initialize temperature sensors and set resolution to 9 bit
  sensors.begin();
  sensors.setResolution(caseThermometer, 9);
  sensors.setResolution(heaterThermometer, 9);
  sensors.setWaitForConversion(false);

  // Initialize Celsius or Fahrenheit values

  if(TEMPERATURE_SCALE_C == true) {
    caseovertemplimit = CASE_TEMP_OVERTEMP_LIMIT_C;
    heaterovertemplimit = HEATER_TEMP_OVERTEMP_LIMIT_C;
    target = INITIAL_TARGET_C;
    maxtemp = MAX_TEMP_SETTING_C;
    undertemp = SENSOR_ERROR_UNDERTEMP_C;
    tolerance = TOLERANCE_C;
    scale = 'C';
  }
  else {
    caseovertemplimit = CASE_TEMP_OVERTEMP_LIMIT_F;
    heaterovertemplimit = HEATER_TEMP_OVERTEMP_LIMIT_F;
    target = INITIAL_TARGET_F;
    maxtemp = MAX_TEMP_SETTING_F;
    undertemp = SENSOR_ERROR_UNDERTEMP_F;
    tolerance = TOLERANCE_F;
    scale = 'F';
  }

  // Initialize display and print basic information
  display.begin();
  display_prepare();

  // Define pins as input/output
  pinMode(HEATER_PIN, OUTPUT);
  pinMode(FAN_PIN, OUTPUT);

  pinMode(BUTTON_SELECT, INPUT_PULLUP);
  pinMode(BUTTON_UP, INPUT_PULLUP);
  pinMode(BUTTON_DOWN, INPUT_PULLUP);

  // Initialize servo and move around
  servoOpen();

  // Initialize serial for logging purposes.
  if(SERIAL_LOGGING == true)
  {
    Serial.begin(115200);
    // Since attaching a serial monitor resets the Arduino,
    // we can use setup() to insert the column headers and the 
    // monitor program will capture it.
    Serial.println(F("Target Temp,Case Temp,Heater Temp,Mode"));
  }

  enableWatchdog();
}

void loop()
{
  resetWatchdog();
  // Check if any of the buttons were pressed
  buttonPressed = checkButtons();

  // Read current temperatures from sensors, if set time has passed since last
  // check As this operation takes quite some time, we'll only do it every few
  // seconds. The temperature in the enclosure doesnt't change that fast
  // anyways.
  if (millis() - previousMillis_temp >= TEMPERATURE_DELAY)
  {
    previousMillis_temp = millis();

    measureTemperatures();  // get new temperatures from sensors
    checkForTempProblems(); // check if temperatures are in safe range

    // This displays the current millis() on the display. It does not serve any
    // function besides telling me that the system is still alive. I needed this
    // for debugging, because I ran into some weird RAM-overflow situations
    // which resultet in crashes of the controller.
    // I kept the code in here, so that I could easily define PRINT_MILLIS to
    // get this functionality. You probably do not want to define it, as it does
    // not serve any purpose for normal use.
#ifdef PRINT_MILLIS
    display.setCursor(90, 40);
    display.fillRect(90, 40, 37, 8, SH110X_BLACK);
    display.print(millis());
#endif
    display.display();
  }

  if(millis() - previousMillis_serial >= SERIAL_RATE_MS && SERIAL_LOGGING == true) {

    // Write a new line of serial data if the timer has elapsed.
    // This allows a different logging rate from the data reporting
    // rate so that logging is not tied to the system's "response rate."
    
    previousMillis_serial = millis();

    writeSerialData();
  }

  // If Select-button is pressed, change modes from 1 to 4
  if (buttonPressed == BUTTON_SELECT)
  {
    operatingMode++;
    if (operatingMode > 3)
    {
      operatingMode = 0;
    }
    buttonPressed = 0;
    changeMode = true;
  }

  /*******************************************************
  ################ IDLE MODE - DO NOTHING ################
  *******************************************************/
  if (operatingMode == MODE_IDLE && changeMode == true)
  {
    changeMode = false;
    drawIdleSymbol();
    servoClose();
    fanOff();
    heaterOff();
  }

  // /*******************************************************
  // ##### HEATING MODE - HEATER ON, FAN ON, VENT CLOSED ####
  // *******************************************************/
  if (operatingMode == MODE_HEATING)
  {
    if (changeMode == true)
    {
      changeMode = false;
      drawHeatingSymbol();
      servoClose();
    }

    // Adjustment of target temperature
    updateTargetTemperature();

    // Check if case temperature is lower or higher than target
    // and turn heater on or off accordingly.
    // The multiple calls of servoClose are a clumsy workaround for
    // keeping the servo quiet (by not keeping it attached the whole time)
    // but also resetting it to the closed position after it sometimes
    // randomly moves when the relays turn on, as they seem to interfere with
    // it.

    if (caseTemp <= lowerlimit)
    {
      if (heaterState == false)
      {
        closeServo = true;
      }
      heaterOn();
      fanOn();
      if (closeServo == true)
      {
        servoClose();
        closeServo = false;
      }
    }
    else if (caseTemp >= upperlimit)
    {
      if (heaterState == true)
      {
        closeServo = true;
      }
      heaterOff();
      fanOff();
      if (closeServo == true)
      {
        servoClose();
        closeServo = false;
      }
    }
  }

  // /*******************************************************
  // ##### COOLING MODE - HEATER OFF, FAN ON, VENT OPEN #####
  // *******************************************************/
  if (operatingMode == MODE_COOLING && changeMode == true)
  {
    changeMode = false;
    drawCoolingSymbol();
    servoOpen();
    fanOn();
    heaterOff();
  }

  // /*******************************************************
  // ###### FAN MODE - HEATER OFF, FAN ON, VENT CLOSED ######
  // *******************************************************/
  if (operatingMode == MODE_FAN && changeMode == true)
  {
    changeMode = false;
    drawFanSymbol();
    servoClose();
    fanOn();
    heaterOff();
  }
}
