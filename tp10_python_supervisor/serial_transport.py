import serial


class SerialTransport:
    def __init__(self, port, baudrate=115200, timeout=0.5):
        self.port = port
        self.baudrate = baudrate
        self.timeout = timeout
        self.serial = None

    def open(self):
        self.serial = serial.Serial(port=self.port, baudrate=self.baudrate, timeout=self.timeout)

    def close(self):
        if self.serial is not None:
            self.serial.close()
            self.serial = None

    def send(self, data):
        self.serial.write(data)

    def receive(self, max_length=64):
        return self.serial.read(max_length)
