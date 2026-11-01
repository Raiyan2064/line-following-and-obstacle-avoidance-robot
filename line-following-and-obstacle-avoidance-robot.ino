// Line Follower with Obstacle Avoiding Robot
// Uses original pin layout with video-style servo sweep & obstacle logic

#define IR_SENSOR_RIGHT 11
#define IR_SENSOR_LEFT  12
#define MOTOR_SPEED     110

// Right motor
int enableRightMotor = 6;
int rightMotorPin1   = 7;
int rightMotorPin2   = 8;

// Left motor
int enableLeftMotor  = 5;
int leftMotorPin1    = 9;
int leftMotorPin2    = 10;

// Servo + HC-SR04
#define SERVO_PIN   3
#define TRIG_PIN    13
#define ECHO_PIN    4

#define OBSTACLE_DIST_CM 5 // Set distance threshold (cm)

int distance_L, distance_F, distance_R;

void setup()
{
  Serial.begin(9600);

  TCCR0B = TCCR0B & B11111000 | B00000010;

  pinMode(enableRightMotor, OUTPUT);
  pinMode(rightMotorPin1,   OUTPUT);
  pinMode(rightMotorPin2,   OUTPUT);

  pinMode(enableLeftMotor,  OUTPUT);
  pinMode(leftMotorPin1,    OUTPUT);
  pinMode(leftMotorPin2,    OUTPUT);

  pinMode(IR_SENSOR_RIGHT, INPUT);
  pinMode(IR_SENSOR_LEFT,  INPUT);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  pinMode(SERVO_PIN, OUTPUT);

  // Sweep servo to center (70 degrees) on startup
  for (int angle = 0;   angle <= 50;  angle += 5) { servoPulse(SERVO_PIN, angle); }
  for (int angle = 50;  angle <= 100; angle += 5) { servoPulse(SERVO_PIN, angle); }
  for (int angle = 100; angle >= 50;  angle -= 5) { servoPulse(SERVO_PIN, angle); }

  distance_F = Ultrasonic_read();

  rotateMotor(0, 0);
  delay(500);
}

void loop()
{
  //==================================================
  //      Line Follower and Obstacle Avoiding
  //==================================================

  distance_F = Ultrasonic_read();
  Serial.print("D F="); Serial.println(distance_F);

  int rightIR = digitalRead(IR_SENSOR_RIGHT);
  int leftIR  = digitalRead(IR_SENSOR_LEFT);

  // Both sensors on white line -> go forward (check obstacle first)
  if (rightIR == LOW && leftIR == LOW)
  {
    if (distance_F > OBSTACLE_DIST_CM) { rotateMotor(MOTOR_SPEED, MOTOR_SPEED); }
    else                               { Check_side(); }
  }
  // Right sensor on black -> turn right
  else if (rightIR == HIGH && leftIR == LOW)
  {
    rotateMotor(-MOTOR_SPEED, MOTOR_SPEED);
  }
  // Left sensor on black -> turn left
  else if (rightIR == LOW && leftIR == HIGH)
  {
    rotateMotor(MOTOR_SPEED, -MOTOR_SPEED);
  }
  else
  {
    rotateMotor(0, 0);
  }

  delay(100);
}

// -------------------------------------------------------
// Servo pulse (software servo, no Servo.h needed)
// -------------------------------------------------------
void servoPulse(int pin, int angle)
{
  int pwm = (angle * 11) + 500; // Convert angle to microseconds
  digitalWrite(pin, HIGH);
  delayMicroseconds(pwm);
  digitalWrite(pin, LOW);
  delay(500); // Refresh cycle of servo
}

// -------------------------------------------------------
// Ultrasonic distance read
// -------------------------------------------------------
long Ultrasonic_read()
{
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  long duration = pulseIn(ECHO_PIN, HIGH, 25000);
  if (duration == 0) return 999;
  return duration * 0.034 / 2;
}

// -------------------------------------------------------
// Scan left & right, then decide which way to turn
// -------------------------------------------------------
void Check_side()
{
  rotateMotor(0, 0);
  delay(1000);

  // Sweep right (70 -> 140)
  for (int angle = 50; angle <= 100; angle += 5) { servoPulse(SERVO_PIN, angle); }
  delay(3000);
  distance_R = Ultrasonic_read();
  Serial.print("D R="); Serial.println(distance_R);
  delay(1000);

  // Sweep left (140 -> 0)
  for (int angle = 100; angle >= 0; angle -= 5) { servoPulse(SERVO_PIN, angle); }
  delay(5000);
  distance_L = Ultrasonic_read();
  Serial.print("D L="); Serial.println(distance_L);
  delay(1000);

  // Return to center (0 -> 70)
  for (int angle = 0; angle <= 50; angle += 5) { servoPulse(SERVO_PIN, angle); }
  delay(3000);

  compareDistance();
}

// -------------------------------------------------------
// Compare left vs right distance and maneuver around
// -------------------------------------------------------
void compareDistance()
{
  if (distance_L > distance_R)
  {
    // More space on left -> turn left and go around
    rotateMotor(MOTOR_SPEED, -MOTOR_SPEED); delay(500);  // turn left
    rotateMotor(MOTOR_SPEED,  MOTOR_SPEED); delay(600);  // forward
    rotateMotor(-MOTOR_SPEED, MOTOR_SPEED); delay(500);  // turn right
    rotateMotor(MOTOR_SPEED,  MOTOR_SPEED); delay(600);  // forward
    rotateMotor(-MOTOR_SPEED, MOTOR_SPEED); delay(400);  // turn right
  }
  else
  {
    // More space on right -> turn right and go around
    rotateMotor(-MOTOR_SPEED, MOTOR_SPEED); delay(500);  // turn right
    rotateMotor( MOTOR_SPEED, MOTOR_SPEED); delay(600);  // forward
    rotateMotor( MOTOR_SPEED,-MOTOR_SPEED); delay(500);  // turn left
    rotateMotor( MOTOR_SPEED, MOTOR_SPEED); delay(600);  // forward
    rotateMotor( MOTOR_SPEED,-MOTOR_SPEED); delay(400);  // turn left
  }
}

// -------------------------------------------------------
// Motor control
// positive = forward, negative = backward, 0 = stop
// -------------------------------------------------------
void rotateMotor(int rightMotorSpeed, int leftMotorSpeed)
{
  if (rightMotorSpeed < 0)
  {
    digitalWrite(rightMotorPin1, LOW);
    digitalWrite(rightMotorPin2, HIGH);
  }
  else if (rightMotorSpeed > 0)
  {
    digitalWrite(rightMotorPin1, HIGH);
    digitalWrite(rightMotorPin2, LOW);
  }
  else
  {
    digitalWrite(rightMotorPin1, LOW);
    digitalWrite(rightMotorPin2, LOW);
  }

  if (leftMotorSpeed < 0)
  {
    digitalWrite(leftMotorPin1, LOW);
    digitalWrite(leftMotorPin2, HIGH);
  }
  else if (leftMotorSpeed > 0)
  {
    digitalWrite(leftMotorPin1, HIGH);
    digitalWrite(leftMotorPin2, LOW);
  }
  else
  {
    digitalWrite(leftMotorPin1, LOW);
    digitalWrite(leftMotorPin2, LOW);
  }

  analogWrite(enableRightMotor, abs(rightMotorSpeed));
  analogWrite(enableLeftMotor,  abs(leftMotorSpeed));
}
