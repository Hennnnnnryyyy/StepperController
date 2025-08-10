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