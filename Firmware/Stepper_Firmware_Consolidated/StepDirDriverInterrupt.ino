
// Dominic Panzino, August 13th, 2025
// Interrupt-based step/dir driver with trapezoidal motion profile

#include <math.h>

class stepDirPlanner{ 
    public:
        stepDirPlanner(int stepPin, int dirPin)
            : stepPin(stepPin), dirPin(dirPin), stepInterval(0.0), accel(0.0), maxSpeed(0.0), currentPosition(0), targetPosition(0), directionPOS(true) {
                pinMode(stepPin,OUTPUT);
                pinMode(dirPin,OUTPUT);
            }

        void setAcceleration(float newAccel) {
            accel = newAccel;
            acceleratingSteps = maxSpeed*maxSpeed / (2 * accel); //This is calculating how many steps it takes to get up to the maximum speed
        }

        void setMaxSpeed(float newMaxSpeed) {
            maxSpeed = newMaxSpeed;
            minInterval = 1000000 / maxSpeed;
        }

        void setCurrentPosition(long newPosition) {
            currentPosition = newPosition;
        }

        void calculateNextStepInterval() {
            //This function tries to calculate what the next step interval should be based on the current step interval...
            //First see if we need to be slowing down:
            long stoppingDist = 1000000000000 / (2*accel*stepInterval); //I think this determins how long it takes us to stop from our current speed
            long distToGo = abs(targetPosition - currentPosition);

            if(distToGo<=stoppingDist) {
                //In this case we need to be stopping.
                stepInterval = stepInterval + (2*stepInterval)/(4*(targetPosition-currentPosition)+1);
            } else if(distToGo>=acceleratingSteps) {
                //In this case we can maybe accelerate?
                if(stepInterval!=minInterval) {
                    //Try accelerating if we aren't already at max speed
                    stepInterval = stepInterval - (2*stepInterval)/(4*(targetPosition-currentPosition)+1);
                    if(stepInterval<minInterval) {stepInterval=minInterval;} //clamp to max speed
                }
            }
        }

        void update() {
            //first check direction
            if(targetPosition==currentPosition) {
                return;
            } else if(targetPosition<currentPosition) {
                directionPOS=false;
                digitalWrite(dirPin,LOW);
            } else if(targetPosition>currentPosition) {
                directionPOS=true;
                digitalWrite(dirPin,HIGH);
            }
            step();
            calculateNextStepInterval();
            scheduleNextStep(stepInterval);
        }

        void scheduleNextStep(long stepInterval) {
            //this function sets up the interrupt timer to call the update() function again after stepInterval us
        }

        void step() {
            //this function just pulses the step pin
        }

        void deadStop() { //Instantly stops everything. Motor will likely stall if at speed.
            targetPosition=currentPosition;
        }
    
    private:
        int stepPin;
        int dirPin;
        long accel;
        long stepInterval;
        long minInterval;
        float maxSpeed;
        long currentPosition;
        long targetPosition;
        bool directionPOS;
        long acceleratingSteps;

}
