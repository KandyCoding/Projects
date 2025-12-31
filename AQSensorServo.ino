#include "Adafruit_PM25AQI.h"   // Include Adafruit PM2.5 sensor library
#include <ESP32Servo.h>         // Include ESP32Servo library for servo control
#include <Wire.h>              // Include Wire library for I2C communication

// Create a TwoWire object for custom I2C pins
TwoWire SENSOR = TwoWire(0);

// Create an instance of the Adafruit PM2.5 sensor class
Adafruit_PM25AQI aqi = Adafruit_PM25AQI();

// Create an instance of the Servo class
Servo myservo;

// Define variables for servo control and measurement
int pos = 0;          // Current servo position
int readings = 0;    // Number of readings taken
bool runloop = false; // Flag to control loop execution  

// Define the pin connected to the servo
#if defined(CONFIG_IDF_TARGET_ESP32S2) || defined(CONFIG_IDF_TARGET_ESP32S3)
int servoPin = 17; // Use pin 17 for ESP32S2 and ESP32S3
#elif defined(CONFIG_IDF_TARGET_ESP32C3)
int servoPin = 7;  // Use pin 7 for ESP32C3
#else
int servoPin = 18; // Use pin 18 for other ESP32 boards
#endif

// Parameters to control the servo and readings
int max_angle = 45;    // Maximum angle to move the servo
int num_readings = 5;  // Number of readings to take per cycle

void setup() {
  // Initialize PWM timers for servo control
  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  ESP32PWM::allocateTimer(2);
  ESP32PWM::allocateTimer(3);

  // Set the servo frequency and attach it to the specified pin
  myservo.setPeriodHertz(50); // Standard frequency for servo
  myservo.attach(servoPin, 500, 2500); // Attach servo to pin with min/max pulse width

  // Initialize serial communication at 115200 baud for debugging
  Serial.begin(115200);

  // Wait for the sensor to boot up
  delay(1000);

  // Initialize I2C communication with custom SDA and SCL pins
  SENSOR.begin(4, 5);

  // Connect to the PM2.5 sensor using I2C
  if (!aqi.begin_I2C(&SENSOR)) {
    Serial.println("Could not find Air Quality sensor!"); // Print error if sensor not found
    while (1) delay(10); // Halt execution
  }

  Serial.println("Air Quality Sensor found!"); // Print success message

  // Print CSV headers
  //Serial.println("Reading Index,Particles > 0.3µm/0.1L,Particles > 0.5µm/0.1L,Particles > 1.0µm/0.1L,Particles > 2.5µm/0.1L,Particles > 5.0µm/0.1L,Particles > 10µm/0.1L,PM1.0 (µg/m³),PM2.5 (µg/m³),PM10 (µg/m³)");
}

void loop() {
  if (runloop) {
    // Variable to store sensor data
    PM25_AQI_Data data;

    // Move the servo to the opening position and wait 1 second
    myservo.write(max_angle); // Move servo to the maximum angle
    delay(1000); // Wait 1 second for the servo to reach the position
    Serial.println("Reading Index,Particles > 0.3µm/0.1L,Particles > 0.5µm/0.1L,Particles > 1.0µm/0.1L,Particles > 2.5µm/0.1L,Particles > 5.0µm/0.1L,Particles > 10µm/0.1L,PM1.0 (µg/m³),PM2.5 (µg/m³),PM10 (µg/m³)");
    // Take measurements and print data
    for (readings = 0; readings < num_readings; readings++) {
      if (!aqi.read(&data)) {
        Serial.println("Could not read from AQI"); // Print error if read fails
        delay(500); // Wait before retrying
        return;
      }
      
      // Print sensor data in CSV format
      Serial.print(readings + 1); // Index for the reading
      Serial.print(", ");
      Serial.print(data.particles_03um); // Particles > 0.3µm per 0.1L air
      Serial.print(", ");
      Serial.print(data.particles_05um); // Particles > 0.5µm per 0.1L air
      Serial.print(", ");
      Serial.print(data.particles_10um); // Particles > 1.0µm per 0.1L air
      Serial.print(", ");
      Serial.print(data.particles_25um); // Particles > 2.5µm per 0.1L air
      Serial.print(", ");
      Serial.print(data.particles_50um); // Particles > 5.0µm per 0.1L air
      Serial.print(", ");
      Serial.print(data.particles_100um); // Particles > 10µm per 0.1L air
      Serial.print(", ");
      Serial.print(data.pm10_env); // PM10 concentration (µg/m³)
      Serial.print(", ");
      Serial.print(data.pm25_env); // PM2.5 concentration (µg/m³)
      Serial.print(", ");
      Serial.print(data.pm100_env); // PM100 concentration (µg/m³)
      Serial.println(); // End of the line

      delay(2000); // Wait 2 seconds before the next reading
    }

    // Move the servo back to the initial position and wait 1 second
    myservo.write(0); // Move servo to the initial position
    delay(1000); // Wait 1 second for the servo to reach the position

    readings = 0; // Reset the readings counter
    delay(1000); // Wait 1 second before starting the next cycle
  }
  
  runloop = false; // Reset the runloop flag

  // Check for input from the serial port
  if (Serial.available() > 0) {
    char input = Serial.read(); // Read the input character
    if (input == ' ') {
      runloop = true; // Set flag to start the loop
    }
  }
}
