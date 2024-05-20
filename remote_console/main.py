import cmd
import time
from crtp_driver import UdpDriver
import struct


class DroneConsole(cmd.Cmd):
    intro = "\033[95mWelcome to the ESP_Drone shell.   Type help or ? to list commands.\033[0m"
    prompt = "\033[94mESP_Drone > \033[0m"
    driver = UdpDriver()
    listening = False
    stop_listening = False
    connected = False

    def do_connect(self, line):
        "Connect the console to the drone"
        if self.connected:
            print("Already connected.")
            return False

        try:
            self.driver.connect()
            self.connected = True
            print(self.driver.receive_packet())
        except TimeoutError:
            self.connected = False
            print("Timeout error. Check the WiFi connection with the drone.")

    def do_close(self, line):
        "Close the connection with the drone."
        self.driver.close()
        self.connected = False

    def do_listen(self, line):
        "Listen the drone port"
        listening = True
        if not self.connected:
            print(
                "Drone is not connected, please connect the drone before trying to listen the communication."
            )
            return False

        print("Listening the drone port. Press [space] to stop.")
        while True:
            recv = self.driver.receive_packet()
            print(recv)

    def do_sensors(self, line):
        print("Reading sensors")
        while True:
            packet = self.driver.receive_packet(raw=True)
            print(packet)
            if packet[0] == 0x3E:
                drone_data = struct.unpack("<d" * 4, packet[1:-1])
                print(
                    f"Altitude: {drone_data[0]}, Pitch: {drone_data[1]}, Roll: {drone_data[2]}, Yaw: {drone_data[3]}"
                )
            else:
                print("Invalid packet")

    def do_update_pid(self, line):
        "Update the PID values. Usage: update_pid <PID_num>[pitch, roll, yaw] <P> <I> <D>"

        if not self.__check_connection():
            return False

        line = line.split(" ")
        if len(line) != 4:
            print(
                "Invalid number of arguments. Usage: update_pid <PID_num>[pitch, roll, yaw] <P> <I> <D>"
            )
            return False

        try:
            pid_num = int(line[0])
            p = float(line[1])
            i = float(line[2])
            d = float(line[3])
        except ValueError:
            print("Invalid values for P, I or D. Must be float numbers.")
            return False

        print(f"Updating PID {pid_num} with values P={p}, I={i}, D={d}")
        self.driver.pid_update(pid_num, p, i, d)

    def do_exit(self, line):
        "Exit the console"
        if self.connected:
            self.driver.close()
        return True

    def __check_connection(self):
        if not self.connected:
            print("Drone is not connected. Please connect the drone first.")
            return False
        return True


if __name__ == "__main__":

    console = DroneConsole()
    console.cmdloop()
