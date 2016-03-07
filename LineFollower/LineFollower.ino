#include <QTRSensors.h>
#include <ZumoReflectanceSensorArray.h>
#include <ZumoMotors.h>
#include <ZumoBuzzer.h>
#include <Pushbutton.h>


ZumoBuzzer buzzer;
ZumoReflectanceSensorArray reflectanceSensors;
ZumoMotors motors;
Pushbutton button(ZUMO_BUTTON);

int lastError = 0;
bool flag = true;

// SENSOR_THRESHOLD is a value to compare reflectance sensor
// readings to to decide if the sensor is over a black line
#define SENSOR_THRESHOLD 300

// ABOVE_LINE is a helper macro that takes returns
// 1 if the sensor is over the line and 0 if otherwise
#define ABOVE_LINE(sensor)((sensor) > SENSOR_THRESHOLD)

// Motor speed when turning. TURN_SPEED should always
// have a positive value, otherwise the Zumo will turn
// in the wrong direction.
#define TURN_SPEED 250

// Motor speed when driving straight. SPEED should always
// have a positive value, otherwise the Zumo will travel in the
// wrong direction.
#define SPEED 250

// Thickness of your line in inches
#define LINE_THICKNESS .75

// This is the maximum speed the motors will be allowed to turn.
// (400 lets the motors go at top speed; decrease to impose a speed limit)
const int MAX_SPEED = 250;

int oneLine = 0;

int control_speed=250;

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


char selectTurn(unsigned char found_left, unsigned char found_straight,
                unsigned char found_right)
{

  // buzzer.play(">>c32");
  if (oneLine == 1) {
    if (found_right)
    {
      control_speed=170;
      return 'R';
    }
    else if (found_left)
    {
      control_speed=170;
      return 'L';
    }

    else if (found_straight)
    {

      return 'S';
    }
    else
    {
      return 'S';
    }
  }
  else if (found_straight)
  {
    return 'S';
  } else if (found_right)
  {control_speed=170;
    return 'R';
  }
  else if (found_left)
  {control_speed=170;
    return 'L';
  } else
  {
    return 'S';
  }
}
int c = 0;
unsigned char prev_found_right = 0;
unsigned char prev_found_left = 0;
void loop()
{


    followLine();



  // buzzer.play(">>b32");//dıt
  unsigned char found_left = 0;
  unsigned char found_straight = 0;
  unsigned char found_right = 0;


  // Now read the sensors and check the intersection type.
  unsigned int sensors[6];



  // Drive straight a bit more, until we are
  // approximately in the middle of intersection.
  // This should help us better detect if we
  // have left or right segments.
  motors.setSpeeds(SPEED, SPEED);
  delay(14);

  reflectanceSensors.readLine(sensors);

  if (c == 0) {
    if (oneLine == 1)
    {
      if (ABOVE_LINE(sensors[5]))
      {
        prev_found_right = 1;
        c++;

      }

      if (ABOVE_LINE(sensors[0])) {
        prev_found_left = 1;
        c++;

      }

    }
    else{
       prev_found_right = 0;
        prev_found_left = 0;
      }

  }

  // Check for the ending spot.
  // If all four middle sensors are on dark black, we have
  // solved the maze.
  if (ABOVE_LINE(sensors[0]) && ABOVE_LINE(sensors[1]) && ABOVE_LINE(sensors[2]) && ABOVE_LINE(sensors[3]) && ABOVE_LINE(sensors[4])&&ABOVE_LINE(sensors[5]))
  {
      buzzer.play(">>a32");
    oneLine++;
    if (oneLine == 2)
    {
      oneLine = 0;
      found_straight = 1;

      //buzzer.play(">>c32");
    }
  }





Serial.println("Start");
Serial.println(prev_found_left);
Serial.println(prev_found_right);
Serial.println(oneLine);

Serial.println("Finish");


  // After driving a little further, we
  // should have passed the intersection
  // and can check to see if we've hit the
  // finish line or if there is a straight segment
  // ahead.




  // Check for left and right exits.
  if (ABOVE_LINE(sensors[0]))
  {
    if(oneLine==1){
        found_left=prev_found_left;
      }
      else{
    found_left = 1;
      }
  }
  if (ABOVE_LINE(sensors[5])){
    if(oneLine==1)
    {
      found_right = prev_found_right;
      }
      else{
    found_right = 1;}
  }
  if (ABOVE_LINE(sensors[2]) || ABOVE_LINE(sensors[3]))
    found_straight = 1;

 /* if (ABOVE_LINE(sensors[0]) || ABOVE_LINE(sensors[5]))
  {
    if (oneLine == 1)
    {
      found_left = prev_found_left;
      found_right = prev_found_right;
    }
  }*/

  /*Serial.println(found_right);
   Serial.println(found_left);
    Serial.println(found_straight);*/

  // Intersection identification is complete.
  unsigned char dir = selectTurn(found_left, found_straight, found_right);

  // Make the turn indicated by the path.
  turn(dir);

}


void followLine()
{
  while (1) {

    unsigned int sensors[6];
    // Get the position of the line.  Note that we *must* provide the "sensors"
    // argument to readLine() here, even though we are not interested in the
    // individual sensor readings
    int position = reflectanceSensors.readLine(sensors);


    // Our "error" is how far we are away from the center of the line, which
    // corresponds to position 2500.
    int error = position - 2500;

    // Get motor speed difference using proportional and derivative PID terms
    // (the integral term is generally not very useful for line following).
    // Here we are using a proportional constant of 1/4 and a derivative
    // constant of 6, which should work """"""""decently for many Zumo motor choices.
    // You probably want to use trial and error to tune these constants for
    // your particular Zumo and line course.
    int speedDifference = error / 4 + 6 * (error - lastError);

    lastError = error;

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

    //
    if(control_speed<250){
        control_speed+=20;
       motors.setSpeeds(control_speed,control_speed);
    }else{
        motors.setSpeeds(m1Speed, m2Speed);
    }

    if (!ABOVE_LINE(sensors[0]) && !ABOVE_LINE(sensors[1]) && !ABOVE_LINE(sensors[2]) && !ABOVE_LINE(sensors[3]) && !ABOVE_LINE(sensors[4]) && !ABOVE_LINE(sensors[5]))
    {
      // There is no line visible ahead, and we didn't see any
      // intersection.  Must be a dead end.
      oneLine=0;
      return;
    }
    else if ((ABOVE_LINE(sensors[0]) || ABOVE_LINE(sensors[5])))
    {
      // Found an intersection.
             if (ABOVE_LINE(sensors[1]) && ABOVE_LINE(sensors[0])&& ABOVE_LINE(sensors[2])&&(flag == true))
            {
                 motors.setSpeeds(-250,250);
                 delay(300);
                 flag=false;

            }
            else if (ABOVE_LINE(sensors[5]) && ABOVE_LINE(sensors[4])&& ABOVE_LINE(sensors[3])&&(flag == true))
            {
                 motors.setSpeeds(250,-250);
                 delay(300);
                 flag=false;
            }

      Serial.println("intersection");
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

      // This while loop monitors line position
      // until the turn is complete.
      while (count < 1)
      {
        reflectanceSensors.readLine(sensors);

        // Increment count whenever the state of the sensor changes
        // (white->black and black->white) since the sensor should
        // pass over 1 line while the robot is turning, the final
        // count should be 2
        count += ABOVE_LINE(sensors[1]) ^ last_status;
        last_status = ABOVE_LINE(sensors[1]);
        //   buzzer.play(">>a32");
      }


      break;

    case 'R':
      // Turn right.
      motors.setSpeeds(TURN_SPEED, -TURN_SPEED );

      // This while loop monitors line position
      // until the turn is complete.
      while (count < 1)
      {
        reflectanceSensors.readLine(sensors);
        count += ABOVE_LINE(sensors[4]) ^ last_status;
        last_status = ABOVE_LINE(sensors[4]);
      }

      break;

    case 'S':
      // Don't do anything!
      break;
  }
}
