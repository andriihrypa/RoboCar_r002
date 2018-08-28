from flask import Flask, render_template, Response, json, request, jsonify
from camera_pi import Camera
from i2c_pwm import *
import RPi.GPIO as GPIO

app = Flask(__name__)

GPIO.setmode(GPIO.BCM)

# Create a dictionary called pins to store the pin number, name, and pin state.
# Will be used for Horn button
# pins = {
# 	23: {'name': 'GPIO 23', 'state': GPIO.LOW},
# 	24: {'name': 'GPIO 24', 'state': GPIO.LOW}
# }
#
# # Set each pin as an output and make it low:
# for pin in pins:
# 	GPIO.setup(pin, GPIO.OUT)
# 	GPIO.output(pin, GPIO.LOW)


@app.route('/')
def index():
	"""Video streaming home page."""
	return render_template('index.html')


def gen(camera):
	"""Video streaming generator function."""
	while True:
		frame = camera.get_frame()
		yield (b'--frame\r\n'
			   b'Content-Type: image/jpeg\r\n\r\n' + frame + b'\r\n')


@app.route('/video_feed')
def video_feed():
	"""Video streaming route. Put this in the src attribute of an img tag."""
	return Response(gen(Camera()),
					mimetype='multipart/x-mixed-replace; boundary=frame')


@app.route('/car_state', methods=['POST'])
def car_state():
	data = request.get_json()
	up_btn_pressed_time = int(data['up_btn_pressed_time'])
	left_btn_pressed_time = int(data['left_btn_pressed_time'])
	right_btn_pressed_time = int(data['right_btn_pressed_time'])
	down_btn_pressed_time = int(data['down_btn_pressed_time'])
	space_btn_press_state = bool(data['space_btn_press_state'])
	# horn_btn_press_state = bool(data['horn_btn_press_state'])

	# TODO add validation!!!

	if space_btn_press_state:
		car = Car(5)
		car.left_side_direction = '0'
		car.left_side_pwm = 0
		car.right_side_direction = '0'
		car.right_side_pwm = 0
		car.send_parcel()
	else:
		car = Car(5)
		direction = 'f'
		if (up_btn_pressed_time - down_btn_pressed_time) < 0:
			direction = 'b'
		left_side_parameter = 0
		right_side_parameter = 0
		if direction == 'f':
			left_side_parameter = up_btn_pressed_time - left_btn_pressed_time
			right_side_parameter = up_btn_pressed_time - right_btn_pressed_time
		elif direction == 'b':
			left_side_parameter = down_btn_pressed_time - left_btn_pressed_time
			right_side_parameter = down_btn_pressed_time - right_btn_pressed_time
		if left_side_parameter < 0:
			left_side_parameter = 0
		if right_btn_pressed_time < 0:
			right_side_parameter = 0
		left_side_pwm = calc_pwm(left_side_parameter)
		right_side_pwm = calc_pwm(right_side_parameter)
		car.left_side_direction = direction
		car.left_side_pwm = left_side_pwm
		car.right_side_direction = direction
		car.right_side_pwm = right_side_pwm
		car.send_parcel()
		return jsonify(42)


def calc_pwm(parameter):
	if parameter <= 100:
		return 0
	result = int((parameter * 0.13) + 120)
	if result > 250:
		result = 250
	return result


if __name__ == '__main__':
	app.run(host='0.0.0.0', port=80, debug=True, threaded=True)
