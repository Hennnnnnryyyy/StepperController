#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1305.h>
#include <Preferences.h>
#include <AccelStepper.h>
#include <esp_task_wdt.h>
#include <TMCStepper.h>

// -------------------------------- Pin Definition --------------------------------
#define STEP_PIN 15
#define DIR_PIN 16
#define EN_PIN 17
#define STEP_UART_PIN 18
#define STEP_DIAG_PIN 19

#define SCLK_PIN 6
#define MOSI_PIN 4
#define OLED_CS_PIN 8
#define OLED_DC_PIN 10
#define OLED_RESET_PIN 9

#define BTN_FWD_PIN 38
#define BTN_STOP_PIN 39
#define BTN_REV_PIN 40
#define BTN_MODE_PIN 42       // I fucked up the schematic, these are flipped
#define BTN_CURRENT_PIN 41  // I fucked up the schematic, these are flipped

#define ENC_A_PIN 35
#define ENC_B_PIN 36
#define ENC_BTN_PIN 37

#define STEPS_PER_REV 1600
#define ENCODER_POS_TO_REV 0.05

// -------------------------------- Variables --------------------------------
// floats give me the ick
float stepper_setSpeed = 1;
float stepper_setAccel = 10;
float stepper_setPos = 0;
float stepper_setMulti = 1;
float stepper_setDist = 1;

int stepper_mode = 0;  // 0 = momentary, 1 = continous, 2 = manJog, 3 = Reciprocate
int stepper_current = 1000;
int cycleCount = 0;

// Encoder wheel stuff
int encoderMode = 0;  // Current mode
struct EncoderParametersStruct {
  float* variable;       // Pointer to the variable we want to modify
  float minVal;          // Minimum value for the variable
  float maxVal;          // Maximum value for the variable
  float fineStepSize;    // Step size for each increment/decrement
  float midStepSize;     // Step size for each increment/decrement
  float coarseStepSize;  // Step size for each increment/decrement
};
EncoderParametersStruct encoderParameter[] = {
  // variable, minVal, maxVal, fine, mid, coarse step size
  { &stepper_setSpeed, 0.01, 25, 0.01, 0.1, 1 },
  { &stepper_setAccel, 0.01, 9999.9, 0.1, 1, 10 },
  { &stepper_setPos, -9999999, 9999999, 1, 1, 1 },
  { &stepper_setMulti, 0.01, 999.99, 0.01, 0.1, 1 },
  { &stepper_setDist, 0.01, 999.99, 0.01, 0.1, 1 },
};

// -------------------------------- Classes --------------------------------
// Edge detector
class edgeDetector {  // Returns 1 on rising edge. How there is no built in function for detecting rising edge that's not ISR baffles me...
public:
  edgeDetector(int pin)
    : pin(pin), lastState(0) {
    pinMode(pin, INPUT);
  }
  bool risingEdge() {
    int currentState = digitalRead(pin);
    bool isRisingEdge = (lastState == 0 && currentState == 1);
    lastState = currentState;
    return isRisingEdge;
  }

private:
  int pin;
  int lastState;
};

edgeDetector btn_fwd(BTN_FWD_PIN);
edgeDetector btn_stop(BTN_STOP_PIN);
edgeDetector btn_rev(BTN_REV_PIN);
edgeDetector btn_mode(BTN_MODE_PIN);
edgeDetector btn_current(BTN_CURRENT_PIN);
edgeDetector btn_enc(ENC_BTN_PIN);


// Stepper Driver Header
class TMCstepperDriver {  // Configures TMC stepper driver
private:
  TMC2209Stepper drv;
public:
  TMCstepperDriver()
    : drv(&Serial2, 0.1f, 0b00) {  // Serial port, sense resistor, driver address
  }
  void begin();                  // Initialize stepper driver
  void setMode(bool mode);       // Set driver to 1 = StealthChop (for homing) or 0 = SpreadCycle (for printing) mode
  void setCurrent(int current);  // Set motor current
  void disable(bool disable);    // 1 to disable motor coils
};

TMCstepperDriver stepperDriver;  // Stepper Driver

// Rest of the stuff
SPIClass myFSPI(FSPI);  // SPI bus for OLED
Adafruit_SSD1305 display(128, 64, &myFSPI, OLED_DC_PIN, OLED_RESET_PIN, OLED_CS_PIN);  // OLED Display
AccelStepper stepper(1, STEP_PIN, DIR_PIN);
// -------------------------------- Main --------------------------------
void setup() {
  Serial.begin(115200);
  pinMode(STEP_PIN, OUTPUT);
  pinMode(DIR_PIN, OUTPUT);
  pinMode(EN_PIN, OUTPUT);
  pinMode(STEP_UART_PIN, OUTPUT);
  pinMode(STEP_DIAG_PIN, INPUT);

  pinMode(SCLK_PIN, OUTPUT);
  pinMode(MOSI_PIN, OUTPUT);
  pinMode(OLED_CS_PIN, OUTPUT);
  pinMode(OLED_DC_PIN, OUTPUT);
  pinMode(OLED_RESET_PIN, OUTPUT);

  pinMode(BTN_FWD_PIN, INPUT);
  pinMode(BTN_STOP_PIN, INPUT);
  pinMode(BTN_REV_PIN, INPUT);
  pinMode(BTN_MODE_PIN, INPUT);
  pinMode(BTN_CURRENT_PIN, INPUT);

  pinMode(ENC_A_PIN, INPUT_PULLDOWN);
  pinMode(ENC_B_PIN, INPUT_PULLDOWN);
  pinMode(ENC_BTN_PIN, INPUT);

  myFSPI.begin(SCLK_PIN, -1, MOSI_PIN, -1);
  stepperDriver.begin();

  stepper.setMaxSpeed(99999999);
  stepper.setAcceleration(99999999);

  xTaskCreatePinnedToCore(DisplayTask, "DisplayTask", 50000, NULL, 0, NULL, 0);
  xTaskCreatePinnedToCore(EncoderTask, "EncoderTask", 5000, NULL, 1, NULL, 0);

  xTaskCreatePinnedToCore(StepperTask, "StepperTask", 5000, NULL, 100, NULL, 1);
}

void loop() {
  vTaskDelay(1000 / portTICK_PERIOD_MS);
}



////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
//    Input Reading

void EncoderTask(void* parameter) {
  static bool ENC_A_lastState = 0;
  bool ENC_A = 0;
  bool ENC_B = 0;

  float incrementValue = 0;
  uint32_t currPulseTime = millis();
  uint32_t lastPulseTime = millis();

  while (1) {
    // Read Current Button
    if (btn_current.risingEdge()) {
      if (stepper_current + 200 <= 2000) {
        stepper_current += 200;
      } else {
        stepper_current = 0;
      }
      stepperDriver.setCurrent(stepper_current);
      digitalWrite(EN_PIN, stepper_current == 0);
    }


    // Read Encoder
    ENC_A = digitalRead(ENC_A_PIN);
    ENC_B = digitalRead(ENC_B_PIN);

    if (ENC_A != ENC_A_lastState && ENC_A) {  // Triggered during rising edge
      currPulseTime = millis();  // encoder increment based on speed
      float stepSize;
      if (currPulseTime - lastPulseTime > 40) {
        stepSize = encoderParameter[encoderMode].fineStepSize;
      } else if (currPulseTime - lastPulseTime > 10) {
        stepSize = encoderParameter[encoderMode].midStepSize;
      } else {
        stepSize = encoderParameter[encoderMode].coarseStepSize;
      }
      incrementValue = stepSize;
      lastPulseTime = currPulseTime;

      // Get a reference to the current variable for easier readability
      float& currentValue = *encoderParameter[encoderMode].variable;

      // Snap the value to the nearest multiple of incrementValue
      if (stepSize > encoderParameter[encoderMode].fineStepSize) {
        currentValue = round(currentValue / stepSize) * stepSize;
      }

      // Adjust the variable based on encoder direction, ensuring it remains within bounds
      if (ENC_B) {
        if (currentValue - incrementValue >= encoderParameter[encoderMode].minVal) {
          currentValue -= incrementValue;
        } else {
          currentValue = encoderParameter[encoderMode].minVal;
        }
      } else {
        if (currentValue + incrementValue <= encoderParameter[encoderMode].maxVal) {
          currentValue += incrementValue;
        } else {
          currentValue = encoderParameter[encoderMode].maxVal;
        }
      }
    }

    ENC_A_lastState = ENC_A;
    vTaskDelay(1 / portTICK_PERIOD_MS);
  }
}



////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
//    Display.ino

byte squareSymbolBitmap[] = {
  0b11100000,
  0b00100000,
  0b11100000,
  0b10000000,
  0b11100000,
};  // bitmap image of up arrow

byte upArrow[] = { 0x20, 0x50, 0x88 };    // bitmap image of up arrow
byte downArrow[] = { 0x88, 0x50, 0x20 };  // bitmap image of down arrow


void DisplayTask(void* parameter) {
  display.begin();
  display.clearDisplay();
  display.display();
  display.setTextColor(1);
  display.setTextSize(1.5);

  drawSplashScreen();
  delay(500);


  while (1) {
    screenMomentaryContinous();
    screenManJog();
    screenReciprocate();
  }
}


void displayDrawArrow() {
  int position = 0;
  if ((millis() % 500) > 125) position = 1;
  if ((millis() % 500) > 250) position = 2;
  if ((millis() % 500) > 375) position = 3;

  if (stepper.speed() > 0)
    display.drawBitmap(81, 24 - position, upArrow, 5, 3, 1);
  if (stepper.speed() < 0)
    display.drawBitmap(81, 21 + position, downArrow, 5, 3, 1);
}


void drawSplashScreen() {
  display.clearDisplay();

  display.setCursor(5, 23);
  random(0, 100) == 0 ? display.print("micronics") : display.print("Formlabs");
  display.setCursor(5, 33);
  display.print("Henry Chan 2024");

  display.display();
}

////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
//    Momentary Continuous.ino

void screenMomentaryContinous() {
  stepper_mode = 0;
  int screenCycleCounter = 0;  // 0 = momentary, 1 = continous, 2= move to next screen
  encoderMode = 0;
  int cursorPos = 0;
  while (1) {
    if (btn_mode.risingEdge()) {
      screenCycleCounter++;
      stepper_mode = 1;
    }

    if (btn_enc.risingEdge()) {
      encoderMode = !encoderMode;
      cursorPos = !cursorPos;
    }

    // ---- status -----
    display.clearDisplay();
    display.setTextSize(2);
    display.setCursor(5, 3);
    display.print("R/s " + String(stepper.speed() / STEPS_PER_REV, 2));
    display.setTextSize(1.5);
    display.setCursor(5, 20);
    screenCycleCounter == 0 ? display.print("Momentary") : display.print("Continous");
    display.setCursor(99, 20);
    if (stepper_current > 100 || (millis() % 1000) > 500) {  // Flash at user if current is 0 so they aren't confused as to why the stepper isn't turning
      stepper_current == 0 ? display.print("OFF") : display.print(String(stepper_current * 0.001, 1) + "A");
    }
    display.drawLine(0, 29, 127, 29, WHITE);
    display.drawLine(93, 19, 93, 29, WHITE);
    displayDrawArrow();


    // ---- settings -----

    display.setCursor(5, 34);
    display.print("Speed: " + String(stepper_setSpeed, 2));
    display.setCursor(93, 34);
    display.print("R/s");
    display.setCursor(5, 44);
    stepper_setAccel <0.05? display.print("Accel: inf" ): display.print("Accel: " + String(stepper_setAccel, 1));
    display.setCursor(93, 44);
    display.print("R/s");
    display.drawBitmap(111, 43, squareSymbolBitmap, 3, 5, 1);
    display.drawRect(43, cursorPos * 10 + 32, 45, 11, WHITE);
    display.display();
    vTaskDelay(10 / portTICK_PERIOD_MS);
    if (screenCycleCounter > 1) return;
  }
}

////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
//    Man Jog.ino

void screenManJog() {
  stepper_mode = 2;
  float motorAngle = 0;
  int cursorPos = -1;  // Initially hide cursor since encoder is used for manual jog control

  while (1) {
    display.clearDisplay();
    display.setTextSize(2);
    display.setCursor(5, 3);

    if (stepper.currentPosition() * 0.225 < -99999) {
      display.print("POS   --");
    } else if (stepper.currentPosition() * 0.225 > 999999) {
      display.print("POS   ++");
    } else {
      abs(stepper.currentPosition() * 0.225) < 1000 ? display.print("POS " + String(stepper.currentPosition() * 0.225, 1)) : display.print("POS " + String(stepper.currentPosition() * 0.225, 0));
    }



    display.setTextSize(1.5);
    display.setCursor(5, 20);
    display.print("Manual Jog");
    display.setCursor(99, 20);
    if (stepper_current > 100 || (millis() % 1000) > 500) {  // Flash at user if current is 0 so they aren't confused as to why the stepper isn't turning
      stepper_current == 0 ? display.print("OFF") : display.print(String(stepper_current * 0.001, 1) + "A");
    }
    display.drawLine(0, 29, 127, 29, WHITE);
    display.drawLine(93, 19, 93, 29, WHITE);
    displayDrawArrow();

    // ---- status -----
    if (btn_enc.risingEdge()) {
      cursorPos++;
      if (cursorPos > 2) cursorPos = -1;
    }

    switch (cursorPos) {
      case -1: encoderMode = 2; break;
      case 0: encoderMode = 3; break;
      case 1: encoderMode = 0; break;
      case 2: encoderMode = 1; break;
    }


    // ---- settings -----
    display.setCursor(5, 34);
    display.print("Multi: " + String(stepper_setMulti, 2));
    display.setCursor(93, 34);
    display.print("X");
    display.setCursor(5, 44);
    display.print("Speed: " + String(stepper_setSpeed, 2));
    display.setCursor(93, 44);
    display.print("R/s");
    display.setCursor(5, 54);
    stepper_setAccel < 0.05 ? display.print("Accel: inf") : display.print("Accel: " + String(stepper_setAccel, 1));
    display.setCursor(93, 54);
    display.print("R/s");
    display.drawBitmap(111, 53, squareSymbolBitmap, 3, 5, 1);
    if (cursorPos >= 0) display.drawRect(43, cursorPos * 10 + 32, 45, 11, WHITE);
    display.display();
    vTaskDelay(10 / portTICK_PERIOD_MS);
    if (btn_mode.risingEdge()) return;
  }
}



////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
//    Reciprocate.ino

void screenReciprocate() {
  stepper_mode = 3;
  int cursorPos = 0;

  while (1) {
    display.clearDisplay();
    display.setTextSize(2);
    display.setCursor(5, 3);
    display.print("# " + String(cycleCount));
    display.setTextSize(1.5);
    display.setCursor(5, 20);
    display.print("Reciprocate");
    display.setCursor(99, 20);
    if (stepper_current > 100 || (millis() % 1000) > 500) {  // Flash at user if current is 0 so they aren't confused as to why the stepper isn't turning
      stepper_current == 0 ? display.print("OFF") : display.print(String(stepper_current * 0.001, 1) + "A");
    }
    display.drawLine(0, 29, 127, 29, WHITE);
    display.drawLine(93, 19, 93, 29, WHITE);
    displayDrawArrow();

    // ---- status -----
    if (btn_enc.risingEdge()) {
      cursorPos++;
      if (cursorPos > 2) cursorPos = 0;
    }

    switch (cursorPos) {
      case 0: encoderMode = 4; break;
      case 1: encoderMode = 0; break;
      case 2: encoderMode = 1; break;
    }


    // ---- settings -----
    display.setCursor(5, 34);
    display.print("Dist : " + String(stepper_setDist, 2));
    display.setCursor(93, 34);
    display.print("Rev");
    display.setCursor(5, 44);
    display.print("Speed: " + String(stepper_setSpeed, 2));
    display.setCursor(93, 44);
    display.print("R/s");
    display.setCursor(5, 54);
    stepper_setAccel <0.05? display.print("Accel: inf" ): display.print("Accel: " + String(stepper_setAccel, 1));
    display.setCursor(93, 54);
    display.print("R/s");
    display.drawBitmap(111, 53, squareSymbolBitmap, 3, 5, 1);
    display.drawRect(43, cursorPos * 10 + 32, 45, 11, WHITE);
    display.display();
    vTaskDelay(10 / portTICK_PERIOD_MS);
    if (btn_mode.risingEdge()) return;
  }
}


////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
//    Stepper.ino

void StepperTask(void* parameter) {
  esp_task_wdt_delete(NULL);         // Unsubscribe this task from watchdog timeout
  esp_task_wdt_init(999999, false);  // Tell task watchdog to fuck off

  int lastStepperMode = stepper_mode;

  while (1) {
    switch (stepper_mode) {
      case 0: stepperMomentary(lastStepperMode); break;
      case 1: stepperContinous(lastStepperMode); break;
      case 2: stepperJog(lastStepperMode); break;
      case 3: stepperReciprocate(lastStepperMode); break;
    }
    lastStepperMode = stepper_mode;
  }
}


void stepperMomentary(int& lastMode) {
  while (1) {
    stepper.setMaxSpeed(stepper_setSpeed * STEPS_PER_REV);
    stepper_setAccel < 0.05 ? stepper.setAcceleration(99999 * STEPS_PER_REV) : stepper.setAcceleration(stepper_setAccel * STEPS_PER_REV);

    if (digitalRead(BTN_FWD_PIN)) {
      stepper.move(STEPS_PER_REV * 1000);  // Move a large distance forward
    } else if (digitalRead(BTN_REV_PIN)) {
      stepper.move(STEPS_PER_REV * -1000);  // Move a large distance backward
    } else {
      stepper.move(0);  // Stop the motor when you aren't hitting any button
    }
    if (digitalRead(BTN_STOP_PIN)) {
      stepper.setMaxSpeed(0);
      stepper.move(0);  // Stop the motor when you hit the stop button
    }
    stepper.run();

    if (stepper_mode != lastMode) {
      stepper.setMaxSpeed(0);
      stepper.move(0);  // Stop the motor when the mode changes
      return;
    }
  }
}


void stepperContinous(int& lastMode) {
  int btnCommand = 0;  // -1 = reverse, 0 = stop, 1 = forward
  while (1) {
    // Button latch
    if (btn_fwd.risingEdge()) {
      btnCommand == 0 ? btnCommand = 1 : btnCommand = 0;
    }
    if (btn_rev.risingEdge()) {
      btnCommand == 0 ? btnCommand = -1 : btnCommand = 0;
    }

    stepper.setMaxSpeed(stepper_setSpeed * STEPS_PER_REV);
    stepper_setAccel < 0.05 ? stepper.setAcceleration(99999 * STEPS_PER_REV) : stepper.setAcceleration(stepper_setAccel * STEPS_PER_REV);

    if (btnCommand == 1) {
      stepper.move(STEPS_PER_REV * 1000);  // Move a large distance forward
    } else if (btnCommand == -1) {
      stepper.move(STEPS_PER_REV * -1000);  // Move a large distance backward
    } else {
      stepper.move(0);  // Stop the motor when there is no btn command
    }
    if (digitalRead(BTN_STOP_PIN)) {
      btnCommand = 0;
      stepper.setMaxSpeed(0);
      stepper.move(0);  // Stop the motor when you hit the stop button
    }
    stepper.run();

    if (stepper_mode != lastMode) {
      stepper.setMaxSpeed(0);
      stepper.move(0);  // Stop the motor when the mode changes
      return;
    }
  }
}


void stepperJog(int& lastMode) {
  int lastEncoderMode = 0;  // If user pushes encoder button to change settings, reset position. Otherwise if scaling changes, it will rotate erratically.
  stepper.setCurrentPosition(0);
  stepper_setPos = 0;
  while (1) {
    stepper.setMaxSpeed(stepper_setSpeed * STEPS_PER_REV);
    stepper_setAccel < 0.05 ? stepper.setAcceleration(99999 * STEPS_PER_REV) : stepper.setAcceleration(stepper_setAccel * STEPS_PER_REV);
    stepper.moveTo(stepper_setPos * ENCODER_POS_TO_REV * STEPS_PER_REV * stepper_setMulti);
    stepper.run();

    if (digitalRead(BTN_STOP_PIN) || lastEncoderMode != encoderMode) {
      stepper.setCurrentPosition(0);
      stepper_setPos = 0;
      stepper.setMaxSpeed(0);
      stepper.move(0);  // Stop the motor when you hit the stop button
    }
    lastEncoderMode = encoderMode;
    if (stepper_mode != lastMode) {
      stepper.setMaxSpeed(0);
      stepper.move(0);  // Stop the motor when the mode changes
      return;
    }
  }
}


void stepperReciprocate(int& lastMode) {
  cycleCount = 0;
  stepper.setCurrentPosition(0);
  bool stopNextCycle = 0;
  int move = 0;

  while (1) {
    move = 0;
    if (btn_fwd.risingEdge()) move = 1;
    if (btn_rev.risingEdge()) move = -1;

    if (move != 0) {
      while (stopNextCycle == 0) {
        stepper.moveTo(move * stepper_setDist * STEPS_PER_REV);
        while (stepper.distanceToGo() != 0) {
          stepper.setMaxSpeed(stepper_setSpeed * STEPS_PER_REV);
          stepper_setAccel < 0.05 ? stepper.setAcceleration(99999 * STEPS_PER_REV) : stepper.setAcceleration(stepper_setAccel * STEPS_PER_REV);
          stepper.run();
          if (btn_fwd.risingEdge() || btn_rev.risingEdge()) stopNextCycle = 1;
          if (digitalRead(BTN_STOP_PIN) || stepper_mode != lastMode) goto here; //Stops the motion immediately when hitting the stop button or changing modes
        }

        stepper.moveTo(0);
        while (stepper.distanceToGo() != 0) {
          stepper.setMaxSpeed(stepper_setSpeed * STEPS_PER_REV);
          stepper_setAccel < 0.05 ? stepper.setAcceleration(99999 * STEPS_PER_REV) : stepper.setAcceleration(stepper_setAccel * STEPS_PER_REV);
          stepper.run();
          if (btn_fwd.risingEdge() || btn_rev.risingEdge()) stopNextCycle = 1;
          if (digitalRead(BTN_STOP_PIN) || stepper_mode != lastMode) goto here; //Stops the motion immediately when hitting the stop button or changing modes
        }

        cycleCount++;
      }
    }
here: //Stops the motion immediately when hitting the stop button or changing modes
    stopNextCycle = 0;
    stepper.setCurrentPosition(0);
    stepper.moveTo(0);
    stepper.setMaxSpeed(0);

    if (stepper_mode != lastMode) {
      stepper.setCurrentPosition(0);
      stepper.moveTo(0);
      stepper.setMaxSpeed(0); // Stop the motor when the mode changes
      return;
    }
  }
}


////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
//    Stepper Driver.ino

void TMCstepperDriver::begin() {  // Mode 1 for homing, mode 0 for printing
  Serial2.begin(115200, SERIAL_8N1, NULL, STEP_UART_PIN);

  drv.begin();
  drv.toff(4);
  drv.blank_time(24);
  drv.rms_current(1000);
  drv.microsteps(8);
  drv.TCOOLTHRS(0xFFFFF);  // 20bit max
  drv.semin(0);
  drv.semax(2);
  drv.shaft(false);
  drv.pwm_autoscale(1);   // Needed for stealthChop
  drv.en_spreadCycle(0);  // 1 = StealthChop for homing/ 0 = SpreadCycle for printing
  drv.sedn(0b01);
  drv.SGTHRS(100);  // Stepper stall threshold
}

void TMCstepperDriver::setMode(bool mode) {
  drv.en_spreadCycle(mode);  // 1 = StealthChop for homing/ 0 = SpreadCycle for printing
}

void TMCstepperDriver::setCurrent(int current) {
  drv.rms_current(current);
}

void TMCstepperDriver::disable(bool disable) {
  disable ? drv.toff(0) : drv.toff(4);
}


