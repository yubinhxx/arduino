#include <Servo.h>

// Arduino pin assignment
#define PIN_SERVO 8

#define _DUTY_MIN 900 // servo full clock-wise position (0 degree)
#define _DUTY_NEU 1500 // servo neutral position (90 degree)
#define _DUTY_MAX 2100 // servo full counter-clockwise position (180 degree)

#define _SERVO_SPEED 0.3
#define _TARGET_ANGLE 90.0

#define _INTERVAL_SERVO 20 // servo interval (unit: msec)
#define _INTERVAL_SERIAL 1000 // serial interval (unit: msec)

Servo myservo;
unsigned long start_time;
unsigned long last_sampling_time_servo;
unsigned long last_sampling_time_serial;
float angle_curr = 0.0;
bool finished = false;

void setup() {
  Serial.begin(57600);
  myservo.attach(PIN_SERVO);
  myservo.writeMicroseconds(_DUTY_MIN);
  delay(3000);

  start_time = millis();
  last_sampling_time_servo = start_time;
  last_sampling_time_serial = start_time;
  Serial.println("START");
  Serial.print("Speed(deg/s):"); Serial.println(_SERVO_SPEED);
  Serial.print("Target(deg):"); Serial.println(_TARGET_ANGLE);
}

void loop() {
  unsigned long time_curr = millis();
  unsigned long elapsed = time_curr - start_time;

  if (!finished &&
      (time_curr - last_sampling_time_servo >= _INTERVAL_SERVO)) {
    last_sampling_time_servo = time_curr;

    angle_curr = _SERVO_SPEED * (elapsed / 1000.0);
    if (angle_curr >= _TARGET_ANGLE) {
      angle_curr = _TARGET_ANGLE;
      finished = true;
    }

    float duty = _DUTY_MIN +
                 (_DUTY_MAX - _DUTY_MIN) * (angle_curr / 180.0);
    myservo.writeMicroseconds((int)(duty + 0.5));

    if (finished) {
      Serial.print("DONE,Time(s):"); Serial.print(elapsed / 1000.0, 3);
      Serial.print(",CommandAngle(deg):"); Serial.println(angle_curr, 3);
    }
  }

  if (time_curr - last_sampling_time_serial >= _INTERVAL_SERIAL) {
    last_sampling_time_serial = time_curr;
    Serial.print("Time(s):"); Serial.print(elapsed / 1000.0, 1);
    Serial.print(",CommandAngle(deg):"); Serial.print(angle_curr, 3);
    Serial.print(",Pulse(us):"); Serial.print(myservo.readMicroseconds());
    Serial.print(",Finished:"); Serial.println(finished);
  }
}
