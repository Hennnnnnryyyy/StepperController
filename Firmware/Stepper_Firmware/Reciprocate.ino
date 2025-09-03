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