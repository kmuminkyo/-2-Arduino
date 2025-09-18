// 실습 3: GPIO를 통한 LED 제어

int ledPin = 7; // LED가 연결된 디지털 핀 번호

void setup() {
  pinMode(ledPin, OUTPUT); // 핀을 출력 모드로 설정
}

void loop() {
  // 1. 처음 1초 동안 LED 켜기
  digitalWrite(ledPin, LOW); // LED ON
  delay(1000); // 1초 대기

  // 2. 다음 1초 동안 LED를 5번 깜빡이기 (0.2초 주기)
  for (int i = 0; i < 5; i++) {
    digitalWrite(ledPin, HIGH);  // OFF
    delay(100); // 0.1초
    digitalWrite(ledPin, LOW); // ON
    delay(100); // 0.1초
  }

  // 3. LED 끄고 무한 루프 상태
  digitalWrite(ledPin, HIGH); // 꺼짐
  while (1) {
    // 무한 루프: 아무것도 하지 않음
  }
}
