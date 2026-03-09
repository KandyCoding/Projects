#!/usr/bin/env python3
"""
Simple IMU Publisher for Jetson Nano
Based on your reference ROS2Topic-Publisher.py
"""

import rclpy
from rclpy.node import Node
import serial
from std_msgs.msg import String

class IMUPublisher(Node):
    def __init__(self):
        super().__init__("imu_publisher")

        # Create publisher
        self.publisher_ = self.create_publisher(String, 'imu/euler_data', 10)

        # Connect to Arduino serial port
        # Common Jetson Nano ports: /dev/ttyUSB0, /dev/ttyACM0
        self.serial_port = serial.Serial('/dev/ttyUSB0', 9600, timeout=1)
        self.get_logger().info("✓ Connected to Arduino on /dev/ttyUSB0")

        # Create timer (10 Hz = 0.1 seconds)
        timer_period = 0.1
        self.timer = self.create_timer(timer_period, self.timer_callback)
        self.count = 0

    def timer_callback(self):
        """Read IMU data from Arduino and publish"""
        if self.serial_port.in_waiting > 0:
            # Read line from Arduino
            line = self.serial_port.readline().decode('utf-8').strip()
            
            # Expected format: "Orientation X: 123.45\tY: 67.89\tZ: 234.56"
            parts = line.split("\t")

            if len(parts) == 3:
                try:
                    # Extract X, Y, Z values
                    X = float(parts[0].split(": ")[1])
                    Y = float(parts[1].split(": ")[1])
                    Z = float(parts[2].split(": ")[1])

                    # Create message
                    imu_msg = String()
                    imu_msg.data = f"X: {X}, Y: {Y}, Z: {Z}"

                    # Publish
                    self.publisher_.publish(imu_msg)
                    self.count += 1
                    
                    # Log every 10 messages to avoid spam
                    if self.count % 10 == 0:
                        self.get_logger().info(f"IMU Data: {imu_msg.data}")
                
                except (ValueError, IndexError) as e:
                    self.get_logger().warn(f"Parse error: {e}")

    def destroy_node(self):
        """Close serial port when done"""
        self.serial_port.close()
        self.get_logger().info("Closed serial port")
        super().destroy_node()


def main(args=None):
    rclpy.init(args=args)

    imu_publisher = IMUPublisher()
    rclpy.spin(imu_publisher)

    imu_publisher.destroy_node()
    rclpy.shutdown()


if __name__ == "__main__":
    main()
