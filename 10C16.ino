#include <Servo.h>

// 핀 설정
#define PIN_TRIG 8
#define PIN_ECHO 9
#define PIN_SERVO 10

Servo myServo;

// 서보 각도 설정
int angleClosed = 0;     // 차단기 내림 위치
int angleOpened = 90;    // 차단기 올림 위치

// 동작 설정
int distanceThreshold = 15; // 차량 감지 거리 (cm)
unsigned long motionDuration = 2000; // 바가 움직이는 데 걸리는 시간 (ms)
unsigned long motionStartTime;
bool isOpening = false;
bool isClosing = false;

// 상태
bool isBarUp = false;

void setup() {
  Serial.begin(9600);
  pinMode(PIN_TRIG, OUTPUT);
  pinMode(PIN_ECHO, INPUT);
  myServo.attach(PIN_SERVO);
  myServo.write(angleClosed);
}

void loop() {
  long distance = getDistanceCM();

  Serial.print("거리(cm): ");
  Serial.println(distance);

  // 차량 감지
  if (distance < distanceThreshold && !isBarUp && !isOpening) {
    isOpening = true;
    motionStartTime = millis();
  }

  // 차량 사라짐
  if (distance >= distanceThreshold && isBarUp && !isClosing) {
    isClosing = true;
    motionStartTime = millis();
  }

  // 열기 동작
  if (isOpening) {
    unsigned long t = millis() - motionStartTime;
    if (t <= motionDuration) {
      float p = (float)t / motionDuration;
      float eased = sigmoid(p); // 부드러운 곡선
      int angle = angleClosed + (angleOpened - angleClosed) * eased;
      myServo.write(angle);
    } else {
      myServo.write(angleOpened);
      isOpening = false;
      isBarUp = true;
    }
  }

  // 닫기 동작
  if (isClosing) {
    unsigned long t = millis() - motionStartTime;
    if (t <= motionDuration) {
      float p = (float)t / motionDuration;
      float eased = sigmoid(p); // sigmoid 또는 linear 사용 가능
      int angle = angleOpened - (angleOpened - angleClosed) * eased;
      myServo.write(angle);
    } else {
      myServo.write(angleClosed);
      isClosing = false;
      isBarUp = false;
    }
  }

  delay(50);
}

// 초음파 거리 측정 함수
long getDistanceCM() {
  digitalWrite(PIN_TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(PIN_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(PIN_TRIG, LOW);

  long duration = pulseIn(PIN_ECHO, HIGH);
  long cm = duration * 0.034 / 2;
  return cm;
}

// sigmoid 함수
float sigmoid(float x) {
  x = x * 12.0 - 6.0;  // 0~1 범위를 -6~6으로 확장
  return 1.0 / (1.0 + exp(-x));
}

// 선형 움직임 함수 (비교용)
float linear(float x) {
  return x;
}
