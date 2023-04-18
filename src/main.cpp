#include <Arduino.h>
#include <Stepper.h>

// Define FSR pin:
#define fsrpin A0

//Define variable to store sensor readings:
int fsrreading; //Variable to store FSR value

// Define number of steps per revolution:
const int stepsPerRevolution = 200;

// Initialize the stepper library on pins 8 through 11:
Stepper myStepper = Stepper(stepsPerRevolution, 8, 9, 10, 11);

//-------------------------------------------------------------
// Settings
int target_force {12}; // N
float conversion_factor {0.02}; // N/bit(/8)
int prop_gain {3};
float tolerance_window {0.15};

int min_step_size {5};
int max_step_size {200};
//-------------------------------------------------------------

// Function prototypes
float readForce ();
int calculateStep (float current_force);



void setup () {
  // Begin serial communication at a baud rate of 9600:
  Serial.begin(9600);

  // Set the motor speed (RPMs):
  myStepper.setSpeed(250);
}



void loop () {
  // Read the FSR pin and store the output as fsrreading:
  float current_force = readForce();

  // Calculate and enact update
  int step_size = calculateStep(current_force);
  myStepper.step(step_size);
  
  delay(50); //Delay 500 ms.
}



float readForce ()
{
  fsrreading = analogRead(fsrpin);
  float current_force = fsrreading  * conversion_factor;

  Serial.print("Reading = ");
  Serial.print(fsrreading);
  Serial.print(",   ");
  Serial.println(current_force);

  return current_force;
}



int calculateStep (float current_force)
{
  int step_size {};
  if (((current_force < (1 - tolerance_window) * target_force) && (current_force < target_force)) || ((current_force > (1 + tolerance_window) * target_force) && (current_force > target_force))) {
    // step_size = prop_gain * (target_force - current_force);
    step_size = ((target_force / conversion_factor) - (fsrreading - 0)) * (max_step_size - min_step_size) / (1023 - 0);
    Serial.println("Actuating...");
  }
  return step_size;
}
