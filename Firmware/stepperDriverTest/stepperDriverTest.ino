#include <math.h>

class StepperMotion {
public:
    StepperMotion(uint8_t stepPin, uint8_t dirPin)
        : STEP_PIN(stepPin), DIR_PIN(dirPin) {}

    void begin(float accelStepsPerSec2, float maxStepsPerSec) {
        accel = accelStepsPerSec2;
        max_speed = maxStepsPerSec;

        pinMode(STEP_PIN, OUTPUT);
        pinMode(DIR_PIN, OUTPUT);

        noInterrupts();
        TCCR1A = 0;
        TCCR1B = 0;
        TCCR1B |= (1 << WGM12);  // CTC mode
        TCCR1B |= (1 << CS11);   //8x prescale, so each tick is 0.5us
        OCR1A = 16000;           // initial compare
        TIMSK1 |= (1 << OCIE1A); // enable compare interrupt
        instance = this;
        interrupts();
    }

    void move(long steps, bool dir) {
        if (running) return; // busy

        digitalWrite(DIR_PIN, dir ? HIGH : LOW);

        steps_remaining = steps;
        accel_steps = (max_speed * max_speed) / (2.0 * accel);
        Serial.print(accel_steps);
        Serial.println("steps to accelerate");
        if (accel_steps * 2 > steps) { 
          Serial.println("Triangle Move Detected !");
          accel_steps = steps / 2;
        }

        min_interval_ns = (1e9 / max_speed);
        Serial.print("min:");
        Serial.print(min_interval_ns);
        Serial.print("ns, max:");

        step_interval_ns = 2*0.8e9/(sqrt(2*accel)); // need to figure out initial interval based on desired acceleration

        Serial.print(step_interval_ns); 
        Serial.println("ns");
        running = true;
        step_count = 0;
    }

    bool isRunning() const { return running; }

    long trigged() {
      return actual_accel_steps;
      //if(justTriggered) {
      //  justTriggered=false;
       // return true;
      //}
      //return false;
    }

    long currentTime() const { return OCR1A; }

    // Interrupt handler entry point
    static void isr() {
        if (instance) instance->stepISR(); //idk what this does
    }

private:
    void stepISR() {
        static bool step_state = false;
        justTriggered = true;

        if (!running) return;

        if (step_state) {
            digitalWrite(STEP_PIN, LOW);
            step_state = false;
            return;
        }

        // Step high
        digitalWrite(STEP_PIN, HIGH);
        step_state = true;
        step_count++;
        steps_remaining--;


        // Accel
        if (step_count < accel_steps) {
            interval_delta_ns = (2*step_interval_ns)/(4*step_count+1);
            step_interval_ns -= interval_delta_ns;
            if(interval_delta_ns<=10) { //At this point we need to keep track of fractional ns increments
              interval_error_term += ((2*error_factor*step_interval_ns)/(4*step_count+1))-interval_delta_ns*error_factor;
              if(interval_error_term>error_factor) {
                interval_error_term-=error_factor;
                step_interval_ns-=1; //decrement the interval by 1ns every time the error term rolls over
              }
            }            
            if (step_interval_ns < min_interval_ns) {
              step_interval_ns = min_interval_ns;
              if(actual_accel_steps==0) {actual_accel_steps=step_count;}
            }
        }
        // Decel
        else if (steps_remaining <= accel_steps) {
            interval_delta_ns = (2*step_interval_ns)/(4*steps_remaining+1);
            step_interval_ns += interval_delta_ns;
            if(interval_delta_ns<=10) { //At this point we need to keep track of fractional ns increments
              interval_error_term += ((2*error_factor*step_interval_ns)/(4*steps_remaining+1))-interval_delta_ns*error_factor;
              if(interval_error_term>error_factor) {
                interval_error_term-=error_factor;
                step_interval_ns+=1; //increment the interval by 1ns every time the error term rolls over
              }
            }
        } else { interval_error_term=0;}



        OCR1A =  step_interval_ns/1000; // ns to ticks.  //Multiply by 2 to get to ticks but then I added a divide by 2 because only ever other call to this function actually pulses the pin.

        if (steps_remaining <= 0) {
            running = false;
        }
    }

    const int STEP_PIN;
    const int DIR_PIN;


    volatile long steps_remaining = 0;
    volatile bool running = false;
    volatile long step_count = 0;

    float accel = 0;
    float max_speed = 0;
    volatile uint32_t step_interval_ns =0;
    volatile uint32_t min_interval_ns = 0;
    volatile uint32_t interval_delta_ns =0;
    volatile uint32_t interval_error_delta=0;
    volatile uint32_t interval_error_term=0;

    volatile bool justTriggered=false;

    long accel_steps = 0;

    volatile long actual_accel_steps=0;

    int error_factor = 10000;

    static StepperMotion* instance;
    
};

// static member definition
StepperMotion* StepperMotion::instance = nullptr;

// Timer1 ISR
ISR(TIMER1_COMPA_vect) {
    StepperMotion::isr();
}

// Example usage
StepperMotion stepper(12, 3);

long val;

void setup() {
    stepper.begin(100, 4000); // accel, max speed
    Serial.begin(112500);
}

void loop() {
    if (!stepper.isRunning()) {
        stepper.move(1600000, true);
        while(stepper.isRunning()){
          val=stepper.trigged();
          if(val>0) {Serial.println(val);}
          //if(stepper.trigged()) {
            //Serial.print("Interupt Fired. Will fire next at ");
            //Serial.println(OCR1A);
          //}
        }
        delay(2000);
    }
}
