#include <Servo.h>

// Arduino pin assignment
#define PIN_LED   9    // LED active-low
#define PIN_TRIG  12   // sonar sensor TRIGGER
#define PIN_ECHO  13   // sonar sensor ECHO
#define PIN_SERVO 10   // servo motor

// sonar configurable parameters
#define SND_VEL 346.0     // sound velocity at 24°C (m/s)
#define INTERVAL 25       // sampling interval (ms)
#define PULSE_DURATION 10 // ultra-sound pulse duration (usec)
#define _DIST_MIN 180.0   // 18cm
#define _DIST_MAX 360.0   // 36cm
#define TIMEOUT ((INTERVAL / 2) * 1000.0) // (usec)
#define SCALE (0.001 * 0.5 * SND_VEL)     // convert duration→distance(mm)

// EMA filter parameter
#define _EMA_ALPHA 0.3    // weight (0~1)

// servo duty settings (adjusted if needed)
#define _DUTY_MIN 1000   // 0°
#define _DUTY_NEU 1500   // 90°
#define _DUTY_MAX 2000   // 180°

// global variables
float dist_raw = 0.0, dist_filtered = 0.0, dist_prev = _DIST_MAX, dist_ema = _DIST_MAX;
unsigned long last_sampling_time = 0;
Servo myservo;

void setup() {
  pinMode(PIN_LED, OUTPUT);
  pinMode(PIN_TRIG, OUTPUT);
  pinMode(PIN_ECHO, INPUT);
  digitalWrite(PIN_TRIG, LOW);

  myservo.attach(PIN_SERVO);
  myservo.writeMicroseconds(_DUTY_NEU);

  Serial.begin(57600);
  Serial.println("=== Ultrasonic Sensor Servo Control (Assignment) ===");
}

void loop() {
  // 샘플링 주기 제어
  if (millis() < last_sampling_time + INTERVAL)
    return;

  // 거리 측정
  dist_raw = USS_measure(PIN_TRIG, PIN_ECHO);

  // 범위 필터 적용
  if ((dist_raw == 0.0) || (dist_raw > _DIST_MAX) || (dist_raw < _DIST_MIN)) {
    dist_filtered = dist_prev; // 이전값 유지
    digitalWrite(PIN_LED, HIGH); // 범위 밖 → LED 꺼짐
  } else {
    dist_filtered = dist_raw;
    dist_prev = dist_raw;
    digitalWrite(PIN_LED, LOW); // 유효 범위 안 → LED 켜짐 (active-low)
  }

  // EMA 필터 적용
  dist_ema = _EMA_ALPHA * dist_filtered + (1 - _EMA_ALPHA) * dist_prev;

  // ---- 거리 기반 서보 각도 제어 ----
  int servo_us;
  if (dist_ema <= _DIST_MIN) {
    servo_us = _DUTY_MIN; // 0°
  } else if (dist_ema >= _DIST_MAX) {
    servo_us = _DUTY_MAX; // 180°
  } else {
    // 거리 18~36cm → 0~180° 선형 보간
    float ratio = (dist_ema - _DIST_MIN) / (_DIST_MAX - _DIST_MIN);
    servo_us = _DUTY_MIN + (int)((_DUTY_MAX - _DUTY_MIN) * ratio);
  }
  myservo.writeMicroseconds(servo_us);

  // ---- 시리얼 출력 (플로터용) ----
  Serial.print("Min:");  Serial.print(_DIST_MIN);
  Serial.print(",dist:"); Serial.print(min(dist_raw, _DIST_MAX + 100));
  Serial.print(",ema:");  Serial.print(min(dist_ema, _DIST_MAX + 100));
  Serial.print(",Servo:"); Serial.print(myservo.read());
  Serial.print(",Max:");  Serial.print(_DIST_MAX);
  Serial.println("");

  // 샘플링 시간 갱신
  last_sampling_time += INTERVAL;
}

// 초음파 거리 측정 함수 (mm 단위)
float USS_measure(int TRIG, int ECHO) {
  digitalWrite(TRIG, HIGH);
  delayMicroseconds(PULSE_DURATION);
  digitalWrite(TRIG, LOW);

  float duration = pulseIn(ECHO, HIGH, TIMEOUT);
  return duration * SCALE; // mm
}
