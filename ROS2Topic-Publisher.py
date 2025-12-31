import rclpy
from rclpy.node import Node
import serial
from std_msgs.msg import String

class IMUPublisher(Node):
    def __init__(self):
        super().__init__("imu_publisher")

        self.publisher_ = self.create_publisher(String, 'imu/euler_data', 10)

        self.serial_port = serial.Serial('/dev/ttyUSB0', 9600, timeout=1)
        self.get_logger().info("Connected to serial port")

        timer_period = 0.1  # seconds
        self.timer = self.create_timer(timer_period, self.timer_callback)
        self.count = 0

    def timer_callback(self):
        if self.serial_port.in_waiting > 0:
            line = self.serial_port.readline().decode('utf-8').strip()
            data = line.split("\t")

            if len(data) == 3:
                X = float(data[0].split(": ")[1])
                Y = float(data[1].split(": ")[1])
                Z = float(data[2].split(": ")[1])

                orientation_msg = String()
                orientation_msg.data = f"X: {X}, Y: {Y}, Z: {Z}"

                self.publisher_.publish(orientation_msg)
                self.count += 1
                self.get_logger().info(f"Publishing Orientation: {orientation_msg.data}")

    def destroy_node(self):
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