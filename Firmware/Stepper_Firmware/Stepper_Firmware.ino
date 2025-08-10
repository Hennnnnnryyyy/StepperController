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


// Stepper Driver
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
