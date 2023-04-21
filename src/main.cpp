#include <Arduino.h>
#include <Stepper.h>

// Define FSR pin:
#define fsrpin1 A0
#define fsrpin2 A1
#define potpin1 A2

// Define number of steps per revolution:
const int stepsPerRevolution = 200;

// Initialize the stepper library on pins 8 through 11:
Stepper myStepper = Stepper(stepsPerRevolution, 3, 4, 5, 6);

//-------------------------------------------------------------
// Settings
int target_force {12}; // N
float conversion_factor {0.02}; // N/bit(/8)
int prop_gain {1};
float tolerance_window {0.15};

int min_step_size {5};
int max_step_size {200};

bool activeFeedback {0};
signed long currentRotation {0};

//-------------------------------------------------------------
// Function prototypes
bool setControlMode (); // todo Caleb
int adjustSpeed (); // todo Jared
float readForce ();
int calculateStep (float current_force); // todo include derivative control
int potentiometerControl (); // todo Steven
bool checkLimits (); // todo Steven
bool checkIntent (); // todo Steven

//-------------------------------------------------------------
void setup () {
  // Begin serial communication at a baud rate of 9600:
  Serial.begin(9600);

  // Set the motor speed (RPMs):
  myStepper.setSpeed(1);
}

void loop () {
  // Check button to determine whether should be using remote control or autonomous
  bool activeFeedback {setControlMode()};

  int targetSpeed {adjustSpeed()};

  bool canContinue {checkLimits()}; 

  if (activeFeedback == true) {
    // read in force
    float current_force = readForce();

    // use it to determine proportional update
    int step_size = calculateStep(current_force);

    canContinue = checkIntent();
    if (canContinue == true) {
      // execute update
      myStepper.step(step_size);
    }
    delay(10);

  } else {
    // int step_size = potentiometerControl();

    // canContinue = checkIntent();
    // if (canContinue == true) {
    //   // execute update
    //   myStepper.step(step_size);
    // }
    myStepper.step(10);
    delay(10);
  }
  delay(10);
}

//-------------------------------------------------------------
// todo Jared
int adjustSpeed ()
{
  int speed {}; // 250 was max we could use before, but now we have a 12v power supply instead of the 9v battery so we can probably crank that up if desired
  myStepper.setSpeed(speed);
  return speed;
}

// todo Caleb
bool setControlMode ()
{
  bool state {};
  return state;
}

// todo Steven
int potentiometerControl ()
{
  int target = analogRead(potpin1);
  Serial.println(target);
  return target;
}

// - DONE
float readForce ()
{
  int fsrreading1 = analogRead(fsrpin1);
  int fsrreading2 = analogRead(fsrpin2);
  float current_force = conversion_factor * ((fsrreading1 + fsrreading2) / 2);

  Serial.print("Reading = ");
  Serial.print(current_force);

  return current_force;
}

 // todo ADD DERIVATIVE CONTROL!
int calculateStep (float current_force)
{
  int step_size {};
  if (((current_force < (1 - tolerance_window) * target_force) && (current_force < target_force)) || ((current_force > (1 + tolerance_window) * target_force) && (current_force > target_force))) {
    // step_size = prop_gain * (target_force - current_force);
    step_size = prop_gain * ((target_force / conversion_factor) - (current_force / conversion_factor - 0)) * (max_step_size - min_step_size) / (1023 - 0); // ! not strict prop control currently
  }
  return step_size;
}
