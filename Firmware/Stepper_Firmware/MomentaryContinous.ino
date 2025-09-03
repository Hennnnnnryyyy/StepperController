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