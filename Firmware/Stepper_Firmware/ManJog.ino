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