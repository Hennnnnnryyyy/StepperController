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
