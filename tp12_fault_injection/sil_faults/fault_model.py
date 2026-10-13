class FaultModel:
    def __init__(self):
        self.frozen_distance = None

    def sensor_disconnected(self):
        return {
            "valid": False,
            "temperature": 0.0,
            "humidity": 0.0
        }

    def freeze_distance(self, distance):
        if self.frozen_distance is None:
            self.frozen_distance = distance

        return self.frozen_distance

    def drift_distance(self, distance, drift):
        return distance + drift

    def noise_distance(self, distance, noise):
        return distance + noise

    def invalid_distance(self):
        return -1.0

    def servo_slow(self, normal_time_ms):
        return normal_time_ms + 2000

    def servo_blocked(self):
        return False

    def endstop_contradiction(self):
        return {
            "limit_open": True,
            "limit_closed": True
        }

    def uart_loss(self):
        return None

    def uart_delay(self, normal_latency_ms):
        return normal_latency_ms + 1000

    def corrupted_checksum(self, checksum):
        return checksum ^ 0xFF