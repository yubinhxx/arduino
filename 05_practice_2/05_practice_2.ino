#define PIN_LED 7
unsigned int count, toggle;

void setup() {
  pinMode(PIN_LED, OUTPUT);
  Serial.begin(115200); // Initialize serial port
  while (!Serial) {
    ; // wait for serial port to connect.
  }
  count = 0;
  toggle = 0;
  digitalWrite(PIN_LED, toggle); // turn off LED.
  delay(1000);
}

void loop() {
  Serial.println(++count);
  toggle = toggle_state(toggle); // toggle LED value.
  digitalWrite(PIN_LED, toggle); // update LED state.
  delay(200); // wait for 1,000 milliseconds
  if (count == 10) {
    digitalWrite(PIN_LED, 1); // turn off LED
    while (1) {
    }
  }
}

int toggle_state(int toggle) {
  return !toggle;
}
