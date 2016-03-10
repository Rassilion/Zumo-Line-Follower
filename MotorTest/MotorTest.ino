#include <ZumoMotors.h>
#include <ZumoBuzzer.h>
#include <Pushbutton.h>
ZumoBuzzer buzzer;
ZumoMotors motors;
Pushbutton button(ZUMO_BUTTON);
void setup() {
  motors.flipLeftMotor(true);
  motors.flipRightMotor(true);
   buzzer.play(">g32>>c32");

  // Wait for the user button to be pressed and released
  button.waitForButton();


}

void loop() {
  button.waitForButton();
  motors.setSpeeds(400, 400);
  delay(100);
  motors.setSpeeds(0, 0);

}
