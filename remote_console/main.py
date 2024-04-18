import cmd
import time
from crtp_driver import UdpDriver


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
            recv = bytes(self.driver.receive_packet(raw=True)[1:])
            recv = int.from_bytes(recv, byteorder="little")
            print(recv)

    def do_exit(self, line):
        "Exit the console"
        if self.connected:
            self.driver.close()
        return True


if __name__ == "__main__":

    console = DroneConsole()
    console.cmdloop()
