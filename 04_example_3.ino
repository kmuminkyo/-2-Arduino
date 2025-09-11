#define PIN_LED 13
unsigned int count, toggle;

void setup() {
  pinMode(PIN_LED, OUTPUT);
  Serial.begin(115200);
  count = toggle = 0;
  digitalWrite(PIN_LED, toggle);
}

void loop() {
  Serial.println(++count);        // 실행 횟수 카운트
  toggle = toggle_state(toggle);  // 토글 함수로 상태 변경
  digitalWrite(PIN_LED, toggle);  // LED를 토글 값으로 제어
  delay(1000);
}

int toggle_state(int toggle) {
  return !toggle;   // 0 1 반전
}
