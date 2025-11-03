#include <Servo.h>

// === 사용자 설정 부분 ===
#define PIN_SERVO 10          // 서보모터 핀 번호 (PWM 핀 사용)
#define _DUTY_MIN  300        // 0도에 해당하는 듀티값 (μs)
#define _DUTY_MAX  2500       // 180도에 해당하는 듀티값 (μs)
#define _SERVO_SPEED 3.0      // 초당 회전 속도 (deg/sec) → 실험 2에서는 0.3으로 변경
#define START_ANGLE 0         // 시작 각도
#define END_ANGLE   180       // 목표 각도 (실험 2에서는 90으로 변경)
#define INTERVAL    20        // 각도 갱신 주기 (ms)

// === 내부 변수 ===
Servo myservo;
float duty_curr;
float duty_target;
float duty_delta;

void setup() {
  myservo.attach(PIN_SERVO);

  duty_curr = mapAngleToDuty(START_ANGLE);
  duty_target = mapAngleToDuty(END_ANGLE);
  myservo.writeMicroseconds((int)duty_curr);

  Serial.begin(57600);
  Serial.println("=== 서보 저속 구동 실험 ===");
  Serial.print("속도: "); Serial.print(_SERVO_SPEED);
  Serial.print(" deg/sec, 목표 각도: "); Serial.print(END_ANGLE);
  Serial.println(" 도");
}

void loop() {
  static unsigned long last_time = 0;
  unsigned long now = millis();

  if (now - last_time >= INTERVAL) {
    last_time = now;

    duty_delta = ((_DUTY_MAX - _DUTY_MIN) * _SERVO_SPEED / 180.0) * (INTERVAL / 1000.0);

    duty_curr += duty_delta;

    if (duty_curr >= duty_target) {
      duty_curr = duty_target;
    }

    myservo.writeMicroseconds((int)duty_curr);

    // 시리얼 출력
    float currentAngle = mapDutyToAngle(duty_curr);
    Serial.print("Duty: ");
    Serial.print(duty_curr);
    Serial.print(" us, Angle: ");
    Serial.println(currentAngle);

    if (duty_curr >= duty_target) {
      Serial.println("🎉 이동 완료!");
      while (1); // 이동 완료 후 정지
    }
  }
}

float mapAngleToDuty(float angle) {
  return _DUTY_MIN + ((angle / 180.0) * (_DUTY_MAX - _DUTY_MIN));
}

float mapDutyToAngle(float duty) {
  return ((duty - _DUTY_MIN) / (_DUTY_MAX - _DUTY_MIN)) * 180.0;
}
