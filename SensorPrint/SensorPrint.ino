#include <QTRSensors.h>
#include <ZumoReflectanceSensorArray.h>
#include <ZumoMotors.h>
#include <ZumoBuzzer.h>
#include <Pushbutton.h>


ZumoBuzzer buzzer;
ZumoReflectanceSensorArray reflectanceSensors;
ZumoMotors motors;
Pushbutton button(ZUMO_BUTTON);

void setup() {
  motors.flipLeftMotor(true);
  motors.flipRightMotor(true);
  Serial.begin(9600);


  unsigned int sensors[6];
  // Play a little welcome song
  buzzer.play(">g32>>c32");

  // Initialize the reflectance sensors module
  reflectanceSensors.init();

  // Wait for the user button to be pressed and released
  button.waitForButton();

  // Turn on LED to indicate we are in calibration mode
  pinMode(13, OUTPUT);
  digitalWrite(13, HIGH);

  // Wait 1 second and then begin automatic sensor calibration
  // by rotating in place to sweep the sensors over the line
  delay(1000);
  int i;
  for (i = 0; i < 80; i++)
  {

    if ((i > 10 && i <= 30) || (i > 50 && i <= 70))
       motors.setSpeeds(-240, 240);
     else
     motors.setSpeeds(240,-240);
    reflectanceSensors.calibrate();

    // Since our counter runs to 80, the total delay will be
    // 80*20 = 1600 ms.
    delay(20);
  }
  motors.setSpeeds(0, 0);

  // Turn off LED to indicate we are through with calibration
  digitalWrite(13, LOW);
  buzzer.play(">g32>>c32");

  // Wait for the user button to be pressed and released
  button.waitForButton();

  // Play music and wait for it to finish before we start driving.
  buzzer.play("L16 cdegreg4");
  while (buzzer.isPlaying());

}

void loop() {
  unsigned int sensors[6];
  int position = reflectanceSensors.readLine(sensors,1,1);
  Serial.print("position: ");Serial.print(position);
  Serial.print(" Sensor 0: ");Serial.print(sensors[0]);
  Serial.print(" Sensor 1: ");Serial.print(sensors[1]);
  Serial.print(" Sensor 2: ");Serial.print(sensors[2]);
  Serial.print(" Sensor 3: ");Serial.print(sensors[3]);
  Serial.print(" Sensor 4: ");Serial.print(sensors[4]);
  Serial.print(" Sensor 5: ");Serial.println(sensors[5]);

  delay(500);

}
