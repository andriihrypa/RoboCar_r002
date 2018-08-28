import smbus


class I2C:
	def __init__(self, bus):
		self.bus = smbus.SMBus(bus)
		pass

	def write_numbers(self, i2c_slave_address, first_byte_address, value_for_send):
		self.bus.write_block_data(i2c_slave_address, first_byte_address, value_for_send)

	def read_number(self, i2c_slave_address):
		# number = bus.read_byte(address)
		number = self.bus.read_byte_data(i2c_slave_address, 1)
		return number


class Car:
	def __init__(self, i2c_address):
		self.i2c = I2C(1)
		self.i2c_address = i2c_address
		self.left_side_direction = "0"
		self.left_side_pwm = 0
		self.right_side_direction = "0"
		self.right_side_pwm = 0

	def send_parcel(self):
		self.i2c.write_numbers(self.i2c_address, 0, [ord(self.left_side_direction), self.left_side_pwm, ord(self.right_side_direction), self.right_side_pwm])
