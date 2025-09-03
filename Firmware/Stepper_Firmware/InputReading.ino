void EncoderTask(void* parameter) {
  static bool ENC_A_lastState = 0;
  bool ENC_A = 0;
  bool ENC_B = 0;

  float incrementValue = 0;
  uint32_t currPulseTime = millis();
  uint32_t lastPulseTime = millis();

  while (1) {
    // Read Freewheeling
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