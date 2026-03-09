/*
 * ============================================================================
 * BLUETOOTH SMART CAR - FULL FEATURED
 * WITH PID + ENCODERS + ULTRASONIC
 * ============================================================================
 * 
 * ============================================================================
 */

#include <PID_v1.h>

// ============================================================================
// PIN DEFINITIONS
// ============================================================================

// Wheel Encoders 
const int ENCODER_LEFT = 2;
const int ENCODER_RIGHT = 3;

// Ultrasonic Sensor 
const int ULTRASONIC_TRIG = 12;
const int ULTRASONIC_ECHO = 11;

// Motor Driver (L298N)
const int MOTOR_LEFT_PWM = 5;
const int MOTOR_LEFT_IN1 = 6;
const int MOTOR_LEFT_IN2 = 7;
const int MOTOR_RIGHT_PWM = 10;
const int MOTOR_RIGHT_IN3 = 8;
const int MOTOR_RIGHT_IN4 = 9;

// ============================================================================
// CONFIGURATION
// ============================================================================

// Motor Speeds
const int BASE_SPEED = 180;
const int TURN_SPEED = 150;
const int MAX_SPEED = 255;
const int MIN_SPEED = 0;

// Ultrasonic Settings
const int SAFE_DISTANCE_CM = 20;
const unsigned long ULTRASONIC_INTERVAL = 100;

// PID Settings
double KP = 2.0;
double KI = 0.1;
double KD = 0.5;
const unsigned long PID_INTERVAL = 50;
const double PID_OUTPUT_MIN = -50.0;
const double PID_OUTPUT_MAX = 50.0;

// Serial
const unsigned long SERIAL_BAUD = 9600;
const unsigned long DEBUG_INTERVAL = 500;

// ============================================================================
// PID CONTROLLER
// ============================================================================

double pidSetpoint = 0.0;
double pidInput = 0.0;
double pidOutput = 0.0;

PID motorPID(&pidInput, &pidOutput, &pidSetpoint, KP, KI, KD, DIRECT);

// ============================================================================
// GLOBAL VARIABLES
// ============================================================================

// Motor control
enum MotionCommand {
  CMD_STOP = 'S',
  CMD_FORWARD = 'F',
  CMD_BACKWARD = 'B',
  CMD_LEFT = 'L',
  CMD_RIGHT = 'R'
};

MotionCommand currentCommand = CMD_STOP;
int leftMotorSpeed = BASE_SPEED;
int rightMotorSpeed = BASE_SPEED;

// Ultrasonic
int currentDistance = 999;
bool obstacleDetected = false;

// Encoders
volatile long leftEncoderCount = 0;
volatile long rightEncoderCount = 0;
long lastLeftEncoderCount = 0;
long lastRightEncoderCount = 0;

// Timing
unsigned long lastUltrasonicCheck = 0;
unsigned long lastPidUpdate = 0;
unsigned long lastDebugOutput = 0;

// ============================================================================
// SETUP
// ============================================================================

void setup() {
  Serial.begin(SERIAL_BAUD);
  Serial.println(F("=== Smart Car - Full Featured ==="));
  Serial.println(F("PID + Encoders + Ultrasonic"));
  Serial.println(F("Commands: F/B/L/R/S"));
  Serial.println();
  
  // Motor pins
  pinMode(MOTOR_LEFT_PWM, OUTPUT);
  pinMode(MOTOR_LEFT_IN1, OUTPUT);
  pinMode(MOTOR_LEFT_IN2, OUTPUT);
  pinMode(MOTOR_RIGHT_PWM, OUTPUT);
  pinMode(MOTOR_RIGHT_IN3, OUTPUT);
  pinMode(MOTOR_RIGHT_IN4, OUTPUT);
  
  // Ultrasonic pins
  pinMode(ULTRASONIC_TRIG, OUTPUT);
  pinMode(ULTRASONIC_ECHO, INPUT);
  
  // Encoder pins
  pinMode(ENCODER_LEFT, INPUT_PULLUP);
  pinMode(ENCODER_RIGHT, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(ENCODER_LEFT), leftEncoderISR, RISING);
  attachInterrupt(digitalPinToInterrupt(ENCODER_RIGHT), rightEncoderISR, RISING);
  
  // PID setup
  motorPID.SetMode(AUTOMATIC);
  motorPID.SetOutputLimits(PID_OUTPUT_MIN, PID_OUTPUT_MAX);
  motorPID.SetSampleTime(PID_INTERVAL);
  
  stopMotors();
  
  Serial.println(F("PIN ASSIGNMENTS:"));
  Serial.println(F("  Encoders: Pins 2 & 3"));
  Serial.println(F("  Ultrasonic TRIG: Pin 12"));
  Serial.println(F("  Ultrasonic ECHO: Pin 11"));
  Serial.println(F("System ready!"));
  Serial.println();
}

// ============================================================================
// MAIN LOOP
// ============================================================================

void loop() {
  unsigned long currentMillis = millis();
  
  // Process Bluetooth
  processBluetoothCommands();
  
  // Check obstacles
  if (currentMillis - lastUltrasonicCheck >= ULTRASONIC_INTERVAL) {
    lastUltrasonicCheck = currentMillis;
    checkObstacles();
  }
  
  // Update PID
  if (currentMillis - lastPidUpdate >= PID_INTERVAL) {
    lastPidUpdate = currentMillis;
    updatePidControl();
  }
  
  // Execute commands
  executeMotionCommand();
  
  // Debug output
  if (currentMillis - lastDebugOutput >= DEBUG_INTERVAL) {
    lastDebugOutput = currentMillis;
    printDebugInfo();
  }
}

// ============================================================================
// BLUETOOTH
// ============================================================================

void processBluetoothCommands() {
  if (Serial.available() > 0) {
    char receivedChar = Serial.read();
    receivedChar = toupper(receivedChar);
    
    switch (receivedChar) {
      case CMD_FORWARD:
      case CMD_BACKWARD:
      case CMD_LEFT:
      case CMD_RIGHT:
      case CMD_STOP:
        currentCommand = static_cast<MotionCommand>(receivedChar);
        Serial.print(F("→ "));
        Serial.println((char)currentCommand);
        break;
    }
  }
}

// ============================================================================
// ULTRASONIC
// ============================================================================

int measureDistance() {
  digitalWrite(ULTRASONIC_TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(ULTRASONIC_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(ULTRASONIC_TRIG, LOW);
  
  long duration = pulseIn(ULTRASONIC_ECHO, HIGH, 30000);
  if (duration == 0) return 999;
  return duration / 58;
}

void checkObstacles() {
  currentDistance = measureDistance();
  
  if (currentDistance < SAFE_DISTANCE_CM) {
    if (!obstacleDetected) {
      obstacleDetected = true;
      Serial.print(F("⚠ OBSTACLE at "));
      Serial.print(currentDistance);
      Serial.println(F(" cm"));
    }
  } else {
    if (obstacleDetected) {
      obstacleDetected = false;
      Serial.println(F("✓ Clear"));
    }
  }
}

// ============================================================================
// PID CONTROL
// ============================================================================

void updatePidControl() {
  if (currentCommand != CMD_FORWARD || obstacleDetected) {
    pidInput = 0.0;
    pidOutput = 0.0;
    leftMotorSpeed = BASE_SPEED;
    rightMotorSpeed = BASE_SPEED;
    return;
  }
  
  // Calculate speed from encoders
  long leftSpeed = leftEncoderCount - lastLeftEncoderCount;
  long rightSpeed = rightEncoderCount - lastRightEncoderCount;
  lastLeftEncoderCount = leftEncoderCount;
  lastRightEncoderCount = rightEncoderCount;
  
  // PID input is speed difference
  pidInput = (double)(rightSpeed - leftSpeed);
  
  motorPID.Compute();
  
  leftMotorSpeed = constrain(BASE_SPEED - pidOutput, MIN_SPEED, MAX_SPEED);
  rightMotorSpeed = constrain(BASE_SPEED + pidOutput, MIN_SPEED, MAX_SPEED);
}

// ============================================================================
// ENCODERS
// ============================================================================

void leftEncoderISR() {
  leftEncoderCount++;
}

void rightEncoderISR() {
  rightEncoderCount++;
}

// ============================================================================
// MOTOR CONTROL
// ============================================================================

void executeMotionCommand() {
  if (currentCommand == CMD_FORWARD && obstacleDetected) {
    stopMotors();
    return;
  }
  
  switch (currentCommand) {
    case CMD_FORWARD:  moveForward();  break;
    case CMD_BACKWARD: moveBackward(); break;
    case CMD_LEFT:     turnLeft();     break;
    case CMD_RIGHT:    turnRight();    break;
    case CMD_STOP:
    default:           stopMotors();   break;
  }
}

void moveForward() {
  digitalWrite(MOTOR_LEFT_IN1, HIGH);
  digitalWrite(MOTOR_LEFT_IN2, LOW);
  analogWrite(MOTOR_LEFT_PWM, leftMotorSpeed);
  
  digitalWrite(MOTOR_RIGHT_IN3, HIGH);
  digitalWrite(MOTOR_RIGHT_IN4, LOW);
  analogWrite(MOTOR_RIGHT_PWM, rightMotorSpeed);
}

void moveBackward() {
  digitalWrite(MOTOR_LEFT_IN1, LOW);
  digitalWrite(MOTOR_LEFT_IN2, HIGH);
  analogWrite(MOTOR_LEFT_PWM, BASE_SPEED);
  
  digitalWrite(MOTOR_RIGHT_IN3, LOW);
  digitalWrite(MOTOR_RIGHT_IN4, HIGH);
  analogWrite(MOTOR_RIGHT_PWM, BASE_SPEED);
}

void turnLeft() {
  digitalWrite(MOTOR_LEFT_IN1, LOW);
  digitalWrite(MOTOR_LEFT_IN2, HIGH);
  analogWrite(MOTOR_LEFT_PWM, TURN_SPEED);
  
  digitalWrite(MOTOR_RIGHT_IN3, HIGH);
  digitalWrite(MOTOR_RIGHT_IN4, LOW);
  analogWrite(MOTOR_RIGHT_PWM, TURN_SPEED);
}

void turnRight() {
  digitalWrite(MOTOR_LEFT_IN1, HIGH);
  digitalWrite(MOTOR_LEFT_IN2, LOW);
  analogWrite(MOTOR_LEFT_PWM, TURN_SPEED);
  
  digitalWrite(MOTOR_RIGHT_IN3, LOW);
  digitalWrite(MOTOR_RIGHT_IN4, HIGH);
  analogWrite(MOTOR_RIGHT_PWM, TURN_SPEED);
}

void stopMotors() {
  digitalWrite(MOTOR_LEFT_IN1, LOW);
  digitalWrite(MOTOR_LEFT_IN2, LOW);
  analogWrite(MOTOR_LEFT_PWM, 0);
  
  digitalWrite(MOTOR_RIGHT_IN3, LOW);
  digitalWrite(MOTOR_RIGHT_IN4, LOW);
  analogWrite(MOTOR_RIGHT_PWM, 0);
}

// ============================================================================
// DEBUG
// ============================================================================

void printDebugInfo() {
  Serial.println(F("--- Status ---"));
  Serial.print(F("Cmd: "));
  Serial.println((char)currentCommand);
  Serial.print(F("Dist: "));
  Serial.print(currentDistance);
  Serial.println(F(" cm"));
  Serial.print(F("Speeds L/R: "));
  Serial.print(leftMotorSpeed);
  Serial.print(F("/"));
  Serial.println(rightMotorSpeed);
  Serial.print(F("Encoders L/R: "));
  Serial.print(leftEncoderCount);
  Serial.print(F("/"));
  Serial.println(rightEncoderCount);
  Serial.println();
}
