#include <Arduino.h>
#include <Stepper.h>

// Define FSR pin:
#define fsrpin1 A0
#define fsrpin2 A1
#define potpin1 A2
#define potpin2 A3

int potMax {};
int potMin {};

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
int speedMax {250};

bool activeFeedback {0};
signed long currentRotation {0};

//-------------------------------------------------------------
// Function prototypes
bool setControlMode (); // todo Caleb
int adjustSpeed (); // todo Jared
float readForce ();
int calculateStep (float current_force); // todo include derivative control
int potentiometerControl (); // todo Steven
int adjustDirection ();

//-------------------------------------------------------------
void setup () {
  // Begin serial communication at a baud rate of 9600:
  Serial.begin(9600);

  // Set the motor speed (RPMs):
  myStepper.setSpeed(200);
}

void loop () {
  // Check button to determine whether should be using remote control or autonomous
  bool activeFeedback {setControlMode()};

  int targetSpeed {adjustSpeed()};

  if (activeFeedback == true) {
    // read in force
    float current_force {readForce()};

    // use it to determine proportional update
    int step_size {calculateStep(current_force)};

    // execute update
    myStepper.step(step_size);

  } else {
    // int step_size = potentiometerControl();
    int direction {adjustDirection()};
    myStepper.step(direction * 10);
  }

  delay(50);
}

//-------------------------------------------------------------
int adjustSpeed () {
  int potVal = analogRead(potpin2);
  int speed = int(map(potVal, potMin, potMax, 1, speedMax));
  myStepper.setSpeed(speed);
  return speed;
}

int adjustDirection () {
  int potVal = analogRead(potpin1);
  int potMed {((potMin + potMax) / 2)};
  if (potVal > potMed && potVal > (1 + tolerance_window) * potMed) {
    return 1;
  } else if ((potVal < potMed && potVal < (1 - tolerance_window) * potMed)) {
    return -1;
  } else {
    return 0;
  }
}

// todo Caleb
bool setControlMode ()
{
  bool state {};
  return state;
}

// todo Steven
int potentiometerControl (int min_turns, int max_turns)
{
  int target = analogRead(potpin1);
  // Serial.println(target);
  float set_turns = map(target, 0, 1023, min_turns, max_turns);
  return set_turns;
}

// - DONE
float readForce ()
{
  int fsrreading1 = analogRead(fsrpin1);
  int fsrreading2 = analogRead(fsrpin2);
  float current_force = conversion_factor * ((fsrreading1 + fsrreading2) / 2);

  Serial.print("Reading = ");
  Serial.println(current_force);

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
