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