#define PIN_LED 7

unsigned long pwmPeriod = 1000;  // PWM 주기: 기본 1ms
int pwmDuty = 0;                // 원하는 밝기: 0~100%

unsigned long pwmStart;
unsigned long fadeStart;

void set_period(int period) {
  pwmPeriod = constrain(period, 100, 10000);
}

void set_duty(int duty) {
  pwmDuty = constrain(duty, 0, 100);
}

void update_pwm(unsigned long now) {
  unsigned long elapsed = now - pwmStart;

  if (elapsed >= pwmPeriod) {
    unsigned long cycles = elapsed / pwmPeriod;
    pwmStart += cycles * pwmPeriod;
    elapsed = now - pwmStart;
  }

  unsigned long onTime = pwmPeriod * pwmDuty / 100;

  if (elapsed < onTime) {
    digitalWrite(PIN_LED, LOW);
  } else {
    digitalWrite(PIN_LED, HIGH);
  }
}

void setup() {
  digitalWrite(PIN_LED, HIGH);
  pinMode(PIN_LED, OUTPUT);

  set_period(100);
  set_duty(0);

  pwmStart = micros();
  fadeStart = pwmStart;
}

void loop() {
  unsigned long now = micros();
  unsigned long elapsed = now - fadeStart;

  if (elapsed >= 1000000UL) {
    unsigned long cycles = elapsed / 1000000UL;
    fadeStart += cycles * 1000000UL;
    elapsed = now - fadeStart;
  }

  int duty;

  if (elapsed < 500000UL) {
    duty = elapsed / 5000UL;
  } else {
    duty = (1000000UL - elapsed) / 5000UL;
  }

  set_duty(duty);
  update_pwm(now);

  delayMicroseconds(1);
}
