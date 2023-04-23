#include <Arduino.h>
#include <Stepper.h>

// Define FSR pin:
#define fsrpin1 A0
#define fsrpin2 A1
#define potpin1 A2
#define potpin2 A3

const int dirPin = 2;
const int stepPin = 3;

int potMax {660};
int potMin {0};

int LEDState=0;
int LEDPinBlue=7;
int LEDPinRed=8;
int buttonPin=12;
int buttonNew;
int buttonOld=1;
int dt=20;

// Define number of steps per revolution:
const int stepsPerRevolution = 200;

// Initialize the stepper library on pins 8 through 11:
Stepper myStepper = Stepper(stepsPerRevolution, 3, 4, 5, 6);

//-------------------------------------------------------------
// Settings
int target_force {12}; // N
float conversion_factor {0.02}; // N/bit(/8)
int prop_gain {1};
float tolerance_window {0.3};
float pot_tol {0.8};

int min_step_size {5};
int max_step_size {200};
int speedMax {250};

bool activeFeedback {0};
signed long currentRotation {0};

bool dir {};

//-------------------------------------------------------------
// Function prototypes
bool setControlMode ();
int adjustSpeed ();
float readForce ();
int calculateStep (float current_force);
int potentiometerControl ();
int adjustDirection ();
void stepMotor (int steps);

//-------------------------------------------------------------
void setup () {
  pinMode(LEDPinBlue, OUTPUT);
  pinMode(LEDPinRed, OUTPUT);
  pinMode(buttonPin, INPUT);

  pinMode(stepPin, OUTPUT);
	pinMode(dirPin, OUTPUT);

  Serial.begin(9600);
}

void loop () {
  // - Check button to determine whether should be using remote control or autonomous
  bool activeFeedback {setControlMode()};

  // * If closed-loop control should be active
  if (activeFeedback == true) {
    // - read in force
    float current_force {readForce()};

    // - use it to determine update in motor step
    int step_size {calculateStep(current_force)};

    // - execute update
    stepMotor(step_size);

  // * If remote control should be active
  } else {
    // - determine direction from user control
    signed int direction {adjustDirection()};

    // - execute update
    stepMotor(direction * 10);
  }

  delay(10);
}

//-------------------------------------------------------------
void stepMotor(int steps)
{
  Serial.println(steps);
  if (steps > 0) {
    digitalWrite(dirPin, HIGH);
  } else if (steps < 0) {
    digitalWrite(dirPin, LOW);
  }
  if (steps != 0) {
    for(int x = 0; x < stepsPerRevolution; x++)
    {
      digitalWrite(stepPin, LOW);
      delayMicroseconds(2000);
      digitalWrite(stepPin, HIGH);
      delayMicroseconds(2000);
    }
  }
}

int adjustSpeed () {
  int potVal = analogRead(potpin2);
  int speed = (map(potVal, potMin, potMax, 1, speedMax));
  return speed;
}

int adjustDirection () {
  int potVal = analogRead(potpin2);
  int potMed {((potMin + potMax) / 2)};
  Serial.println();
  if (potVal > potMed && potVal > (1 + pot_tol) * potMed) {
    return 1;
  } else if ((potVal < potMed && potVal < (1 - pot_tol) * potMed)) {
    return -1;
  } else {
    return 0;
  }
  
}

bool setControlMode ()
{
  buttonNew=digitalRead(buttonPin);
  if(buttonOld==0 && buttonNew==1){
    if (LEDState==0){
      digitalWrite(LEDPinBlue,HIGH);
      digitalWrite(LEDPinRed,LOW);
      LEDState=1;
    }
    else{
      digitalWrite(LEDPinBlue, LOW);
      digitalWrite(LEDPinRed,HIGH);
      LEDState=0;
    }
  }
  buttonOld=buttonNew;
  delay(dt);
  return LEDState;
}

float readForce ()
{
  int fsrreading1 = analogRead(fsrpin1);
  int fsrreading2 = analogRead(fsrpin2);
  float current_force = conversion_factor * ((fsrreading1 + fsrreading2) / 2);

  Serial.print("Reading = ");
  Serial.println(current_force);

  return current_force;
}

int calculateStep (float current_force)
{
  int step_size {};
  if (((current_force < (1 - tolerance_window) * target_force) && (current_force < target_force)) || ((current_force > (1 + tolerance_window) * target_force) && (current_force > target_force))) {
    // step_size = prop_gain * (target_force - current_force);
    step_size = prop_gain * ((target_force / conversion_factor) - (current_force / conversion_factor - 0)) * (max_step_size - min_step_size) / (1023 - 0); // ! not strict prop control currently
  }
  return step_size;
}
