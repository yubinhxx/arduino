/*
 * Created by ArduinoGetStarted.com
 *
 * This example code is in the public domain
 *
 * Tutorial page: https://arduinogetstarted.com/faq/how-to-control-speed-of-servo-motor
 */



#include <Servo.h>
#include <math.h>

#define PIN_SERVO 8
#define PIN_TRIG 12
#define PIN_ECHO 13

const int CLOSED_ANGLE = 0;
const int OPEN_ANGLE = 90;
const unsigned long MOVING_TIME = 1500; // moving time is 3 seconds
const unsigned long SAMPLE_INTERVAL = 70;
const unsigned long CLEAR_TIME = 800;
const unsigned long ECHO_TIMEOUT = 25000;
const float DETECT_MM = 200.0f;
const float CLEAR_MM = 260.0f;
const float SND_VEL = 346.0f;
const float SIGMOID_K = 10.0f;

enum GateState { CLOSED, OPENING, OPEN, CLOSING };
enum CurveType { SIGMOID, SMOOTHSTEP };

float sigmoidEase(float t);
float smoothstepEase(float t);
float measureDistance();
void beginMove(float target, GateState nextState, unsigned long now);
void updateServo(unsigned long now);
Servo myServo;
GateState gateState = CLOSED;
CurveType curveType = SIGMOID;
CurveType movingCurve = SIGMOID;

unsigned long moveStartTime = 0;
unsigned long moveDuration = MOVING_TIME;
unsigned long lastSamplingTime = 0;
unsigned long clearStartTime = 0;
float startAngle = CLOSED_ANGLE; // 30°
float stopAngle = CLOSED_ANGLE; // 90°
float currentAngle = CLOSED_ANGLE;
float distanceMM = -1.0f;
bool clearTiming = false;
unsigned int nearCount = 0;


float sigmoidEase(float t) {
  float low = 1.0f / (1.0f + expf(SIGMOID_K * 0.5f));
  float high = 1.0f / (1.0f + expf(-SIGMOID_K * 0.5f));
  float value = 1.0f / (1.0f + expf(-SIGMOID_K * (t - 0.5f)));
  return (value - low) / (high - low);
}


float smoothstepEase(float t) {
  return t * t * (3.0f - 2.0f * t);
}

float measureDistance() {
  digitalWrite(PIN_TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(PIN_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(PIN_TRIG, LOW);
  unsigned long duration = pulseIn(PIN_ECHO, HIGH, ECHO_TIMEOUT);
  if (duration == 0) return -1.0f;
  float distance = duration * 0.001f * 0.5f * SND_VEL;
  if (distance < 20.0f || distance > 4000.0f) return -1.0f;
  return distance;
}

void beginMove(float target, GateState nextState, unsigned long now) {
  startAngle = currentAngle;
  stopAngle = target;
  float fraction = fabsf(stopAngle - startAngle) /
                   fabsf((float)OPEN_ANGLE - CLOSED_ANGLE);
  moveDuration = (unsigned long)(MOVING_TIME * fraction);
  if (moveDuration < 1) moveDuration = 1;
  moveStartTime = now; // reset start time for next movement
  movingCurve = curveType;
  gateState = nextState;
  clearTiming = false;
}

void updateServo(unsigned long now) {
  if (gateState != OPENING && gateState != CLOSING) return;
  unsigned long progress = now - moveStartTime;
  if (progress >= moveDuration) {
    currentAngle = stopAngle;
    myServo.write((int)currentAngle);
    gateState = (gateState == OPENING) ? OPEN : CLOSED;
    clearTiming = false;
    return;
  }
  // while moving
  float t = (float)progress / moveDuration;
  float eased = (movingCurve == SIGMOID) ? sigmoidEase(t) : smoothstepEase(t);
  currentAngle = startAngle + (stopAngle - startAngle) * eased;
  myServo.write((int)(currentAngle + 0.5f));
}

void setup() {
  Serial.begin(115200);
  pinMode(PIN_TRIG, OUTPUT);
  pinMode(PIN_ECHO, INPUT);
  digitalWrite(PIN_TRIG, LOW);
  myServo.attach(PIN_SERVO);
  myServo.write(CLOSED_ANGLE); // Set position
  // start moving
  delay(500);
  Serial.println(F("1: sigmoid / 2: smoothstep"));
  Serial.println(F("distance_mm,angle,state,curve (0:CLOSED 1:OPENING 2:OPEN 3:CLOSING)"));
}

void loop() {
  while (Serial.available() > 0) {
    char command = Serial.read();
    if (command == '1') curveType = SIGMOID;
    if (command == '2') curveType = SMOOTHSTEP;
  }
  unsigned long now = millis();
  updateServo(now);

  if (now - lastSamplingTime >= SAMPLE_INTERVAL) {
    lastSamplingTime = now;
    distanceMM = measureDistance();
    now = millis();
    updateServo(now);
    bool near = distanceMM >= 0.0f && distanceMM <= DETECT_MM;
    bool far = distanceMM >= CLEAR_MM || distanceMM < 0.0f;

    if (near) {
      if (nearCount < 2) ++nearCount;
    } else {
      nearCount = 0;
    }
    if (nearCount >= 2 && (gateState == CLOSED || gateState == CLOSING)) {
      beginMove(OPEN_ANGLE, OPENING, now);
    }

    if (gateState == OPEN && far) {
      if (!clearTiming) {
        clearStartTime = now;
        clearTiming = true;
      } else if (now - clearStartTime >= CLEAR_TIME) {
        // movement finished -> reverse direction
        beginMove(CLOSED_ANGLE, CLOSING, now);
      }
    } else {
      clearTiming = false;
    }

    Serial.print(distanceMM);
    Serial.print(',');
    Serial.print(currentAngle);
    Serial.print(',');
    Serial.print((int)gateState);
    Serial.print(',');
    Serial.println((int)((gateState == OPENING || gateState == CLOSING)
                         ? movingCurve : curveType));
  }
}
