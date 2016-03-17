#include <QTRSensors.h>
#include <ZumoReflectanceSensorArray.h>
#include <ZumoMotors.h>
#include <ZumoBuzzer.h>
#include <Pushbutton.h>


ZumoBuzzer buzzer;
ZumoReflectanceSensorArray reflectanceSensors;
ZumoMotors motors;
Pushbutton button(ZUMO_BUTTON);



int last_proportional;
int integral;
int control = 1;
int near = 0;
// SENSOR_THRESHOLD is a value to compare reflectance sensor
// readings to to decide if the sensor is over a black line
#define SENSOR_THRESHOLD 200

// ABOVE_LINE is a helper macro that takes returns
// 1 if the sensor is over the line and 0 if otherwise
// white line <, black line >
#define ABOVE_LINE(sensor)((sensor) < SENSOR_THRESHOLD)

// Motor speed when turning. TURN_SPEED should always
// have a positive value, otherwise the Zumo will turn
// in the wrong direction.
#define TURN_SPEED 275

// Motor speed when driving straight. SPEED should always
// have a positive value, otherwise the Zumo will travel in the
// wrong direction.
#define SPEED 325


// FollowLine daki max hız
const int MAX_SPEED = 300;

int oneLine = 0;



void setup()
{
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
      motors.setSpeeds(-200, 200);
    else
      motors.setSpeeds(200, -200);
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


char selectTurn(unsigned char found_left, unsigned char found_straight,
                unsigned char found_right)
{

  // buzzer.play(">>c32");
  Serial.print("left :");
  Serial.println(found_left);
  Serial.print("right :");
  Serial.println(found_right);
  Serial.print("straight :");
  Serial.println(found_straight);

  Serial.println("//");
  if (oneLine == 1) {
    if (found_right)
    {
      return 'R';
    }
    else if (found_left)
    {
      return 'L';
    }
    else
    {
      return 'S';
    }
  } else {
    if (found_straight)
    {
      return 'S';
    } else if (found_right)
    {
      return 'R';
    }
    else if (found_left)
    {
      return 'L';
    } else
    {
      return 'S';
    }
  }
}

unsigned char last = 'S';
int proksimiti = 0;
unsigned long time1;
unsigned long time2;
void loop()
{

  //start following line
  followLine();

  // initilize turn signals 
  unsigned char found_left = 0;
  unsigned char found_straight = 0;
  unsigned char found_right = 0;

  // buzzer.play(">>b32");//dıt



  // Now read the sensors and check the intersection type.
  unsigned int sensors[6];



  // Drive straight a bit more, until we are
  // approximately in the middle of intersection.
  // This should help us better detect if we
  // have left or right segments.
  motors.setSpeeds(SPEED, SPEED);
  delay(28);

  // read sensors again
  reflectanceSensors.readLine(sensors, 1, 1);


  //check if zumo on oneLine, else check turn signals
  if (ABOVE_LINE(sensors[0]) && ABOVE_LINE(sensors[1]) && ABOVE_LINE(sensors[2]) && ABOVE_LINE(sensors[3]) && ABOVE_LINE(sensors[4]) && ABOVE_LINE(sensors[5]))
  {
    
    oneLine++;

    // save current oneLine time
    time1 = millis();


    // reset oneLine if its second turn on loop
    if (oneLine == 2)
    {
      oneLine = 0;

      //dıt

    }

    //check last and current time to see if its a twoLine
    if (abs(time1 - time2) < 180)
    {

      //buzzer.play(">g32>>c32");

      found_left = 0;
      found_straight = 1;
      found_right = 0;
      oneLine = 0;

      // drive straight to pass shortcut
      motors.setSpeeds(300, 300); //CHECK: this needs to calibrated
      delay(800); //CHECK: this needs to calibrated


    }
    time2 = time1; //save last time
  }
  else {
    //straight if
    if ((ABOVE_LINE(sensors[2]) || ABOVE_LINE(sensors[3])) && (!ABOVE_LINE(sensors[0]) && !ABOVE_LINE(sensors[5])))
    {
      found_straight = 1;

    }
    //left if
    if ((ABOVE_LINE(sensors[0]) ) )
    {
      found_left = 1;
      //  buzzer.play(">>b32");
    }
    //right if
    if ((ABOVE_LINE(sensors[5]) ) )
    {
      found_right = 1;
    }

  }


  //select a turn based on oneLine
  unsigned char dir = selectTurn(found_left, found_straight, found_right);

  //if on loop and if turn not straight save turn
  if (dir != 'S' && oneLine)
    last = dir;
  //if turn and last turn is same zumo is on enterence of loop, pgo straight and pass enterance
  else if (dir == last && !ABOVE_LINE(sensors[2]) && !ABOVE_LINE(sensors[3])) {
    dir = 'S';
    control = 0;
  }
  // if proksimity sensor reads value turn given direction
  if (proksimiti == 1)
  {
    dir = 'L';//CHECK: object turn is left?
    proksimiti = 0;
  }

  //turns given direction
  turn(dir);

}


void followLine()
{
  while (1) {

    unsigned int sensors[6];

    // Get the position of the line.  Note that we *must* provide
    // the "sensors" argument to read_line() here, even though we
    // are not interested in the individual sensor readings.
    unsigned int position = reflectanceSensors.readLine(sensors, 1, 1);

    // The "proportional" term should be 0 when we are on the line.
    int proportional = ((int)position) - 2500;

    // Compute the derivative (change) and integral (sum) of the
    // position.
    int derivative = proportional - last_proportional;
    integral += proportional;

    // Remember the last position.
    last_proportional = proportional;

    // proportional*kp+integral*ki+derivative*kd
    int speedDifference = proportional / 2  + integral / 10000 + derivative * (3 / 2);


    // Get individual motor speeds.  The sign of speedDifference
    // determines if the robot turns left or right.
    int m1Speed = MAX_SPEED + speedDifference;
    int m2Speed = MAX_SPEED - speedDifference;

    // Here we constrain our motor speeds to be between 0 and MAX_SPEED.
    // Generally speaking, one motor will always be turning at MAX_SPEED
    // and the other will be at MAX_SPEED-|speedDifference| if that is positive,
    // else it will be stationary.  For some applications, you might want to
    // allow the motor speed to go negative so that it can spin in reverse.
    if (m1Speed < 0)
      m1Speed = 0;
    if (m2Speed < 0)
      m2Speed = 0;
    if (m1Speed > MAX_SPEED)
      m1Speed = MAX_SPEED;
    if (m2Speed > MAX_SPEED)
      m2Speed = MAX_SPEED;

    motors.setSpeeds(m1Speed, m2Speed);

    //proksimity sensor value
    //CHECK: needs to calibrated to object
    if (analogRead(A1) < 200 && analogRead(A1) > 190 )
    {
      proksimiti = 1;
      buzzer.play(">>b32");

    }
    //zumo lost the line go straight 
    if (!ABOVE_LINE(sensors[0]) && !ABOVE_LINE(sensors[1]) && !ABOVE_LINE(sensors[2]) && !ABOVE_LINE(sensors[3]) && !ABOVE_LINE(sensors[4]) && !ABOVE_LINE(sensors[5]))
    {
      oneLine = 0;//TODO: problem when zumo lost the line while in loop

      motors.setSpeeds(307, 300);//CHECK:  needs to calibrated
      //return;
    }
    //break while when there is a turn or oneLine;
    else if ((ABOVE_LINE(sensors[0]) && ABOVE_LINE(sensors[1]) || (ABOVE_LINE(sensors[4]) && ABOVE_LINE(sensors[5])) ))
    {
      return;
    }

  }
}

// Turns according to the parameter dir, which should be
// 'L' (left), 'R' (right), 'S' (straight), or 'B' (back).
void turn(char dir)
{

  // count and last_status help
  // keep track of how much further
  // the Zumo needs to turn.
  unsigned short count = 0;
  unsigned short last_status = 0;
  unsigned int sensors[6];
  Serial.print("Direciton : ");
  Serial.println(dir);
  // dir tests for which direction to turn
  switch (dir)
  {

    // Since we're using the sensors to coordinate turns instead of timing them,
    // we can treat a left turn the same as a direction reversal: they differ only
    // in whether the zumo will turn 90 degrees or 180 degrees before seeing the
    // line under the sensor. If 'B' is passed to the turn function when there is a
    // left turn available, then the Zumo will turn onto the left segment.
    case 'L':
    case 'B':
      // Turn left.
      motors.setSpeeds(-TURN_SPEED , TURN_SPEED);
      delay(30);
      // This while loop monitors line position
      // until the turn is complete.
      while (count < 1)
      {
        reflectanceSensors.readLine(sensors, 1, 1);
        // Increment count whenever the state of the sensor changes
        // (white->black and black->white) since the sensor should
        // pass over 1 line while the robot is turning, the final
        // count should be 2
        count += ABOVE_LINE(sensors[1]) ^ last_status;
        last_status = ABOVE_LINE(sensors[1]);
        //   buzzer.play(">>a32");
      }
      proksimiti = 0;

      break;

    case 'R':
      // Turn right.
      motors.setSpeeds(TURN_SPEED, -TURN_SPEED );
      // This while loop monitors line position
      // until the turn is complete.
      delay(30);
      while (count < 1)
      {
        reflectanceSensors.readLine(sensors, 1, 1);
        count += ABOVE_LINE(sensors[4]) ^ last_status;
        last_status = ABOVE_LINE(sensors[4]);
      }
      proksimiti = 0;
      break;

    case 'S':
      // Don't do anything!
      break;
  }
}
