// Arduino pin assignment
#define PIN_LED  9
#define PIN_TRIG 12
#define PIN_ECHO 13

// configurable parameters
#define SND_VEL 346.0     // sound velocity at 24 celsius degree (unit: m/sec)
#define INTERVAL 25       // sampling interval (unit: msec)
#define PULSE_DURATION 10 // ultra-sound Pulse Duration (unit: usec)
#define _DIST_MIN 100     // minimum distance to be measured (unit: mm)
#define _DIST_MAX 300     // maximum distance to be measured (unit: mm)

#define TIMEOUT ((INTERVAL / 2) * 1000.0) // maximum echo waiting time (unit: usec)
#define SCALE (0.001 * 0.5 * SND_VEL)     // coefficent to convert duration to distance

#define N_MEDIAN 3      // Median filter window size (change to 3, 10, 30 for experiments)

// global variables
unsigned long last_sampling_time;   // unit: msec
float dist_median = 0.0;            // Median filter result
float dist_ema = 0.0;               // For format only, not calculated

// Median filter buffer
float samples[N_MEDIAN];
int sample_index = 0;
bool samples_filled = false;

// --- Median filter function ---
float getMedian(float arr[], int size) {
  float sorted[size];
  memcpy(sorted, arr, sizeof(float) * size);

  for (int i = 0; i < size - 1; i++) {
    for (int j = i + 1; j < size; j++) {
      if (sorted[i] > sorted[j]) {
        float temp = sorted[i];
        sorted[i] = sorted[j];
        sorted[j] = temp;
      }
    }
  }

  return sorted[size / 2];
}

void setup() {
  // initialize GPIO pins
  pinMode(PIN_LED, OUTPUT);
  pinMode(PIN_TRIG, OUTPUT);
  pinMode(PIN_ECHO, INPUT);
  digitalWrite(PIN_TRIG, LOW);

  // initialize serial port
  Serial.begin(57600);
}

void loop() {
  float dist_raw;

  // wait until next sampling time
  if (millis() < last_sampling_time + INTERVAL)
    return;

  // get raw distance
  dist_raw = USS_measure(PIN_TRIG, PIN_ECHO);

  // --- 중위수 필터 샘플 저장 ---
  samples[sample_index] = dist_raw;
  sample_index++;

  if (sample_index >= N_MEDIAN) {
    sample_index = 0;
    samples_filled = true;
  }

  // --- 중위수 계산 ---
  if (samples_filled) {
    dist_median = getMedian(samples, N_MEDIAN);
  } else {
    dist_median = dist_raw;
  }

  // --- 시리얼 플로터 출력 ---
  Serial.print("Min:");     Serial.print(_DIST_MIN);
  Serial.print(",raw:");    Serial.print(min(dist_raw, _DIST_MAX + 100)); 
  Serial.print(",ema:");    Serial.print(dist_ema);  // 값은 0.0 (슬라이드 20번째의 출력 형식 맞춤용으로 추가하였습니다.)
  Serial.print(",median:"); Serial.print(min(dist_median, _DIST_MAX + 100));
  Serial.print(",Max:");    Serial.print(_DIST_MAX);
  Serial.println("");

  // --- LED 제어: 중위수 기준 ---
  if ((dist_median < _DIST_MIN) || (dist_median > _DIST_MAX))
    digitalWrite(PIN_LED, 1);  // OFF
  else
    digitalWrite(PIN_LED, 0);  // ON

  last_sampling_time += INTERVAL;
}

// get a distance reading from USS. return value is in millimeter.
float USS_measure(int TRIG, int ECHO)
{
  digitalWrite(TRIG, HIGH);
  delayMicroseconds(PULSE_DURATION);
  digitalWrite(TRIG, LOW);

  return pulseIn(ECHO, HIGH, TIMEOUT) * SCALE;
}


  // Pulse duration to distance conversion example (target distance = 17.3m)
  // - pulseIn(ECHO, HIGH, timeout) returns microseconds (음파의 왕복 시간)
  // - 편도 거리 = (pulseIn() / 1,000,000) * SND_VEL / 2 (미터 단위)
  //   mm 단위로 하려면 * 1,000이 필요 ==>  SCALE = 0.001 * 0.5 * SND_VEL
  //
  // - 예, pusseIn()이 100,000 이면 (= 0.1초, 왕복 거리 34.6m)
  //        = 100,000 micro*sec * 0.001 milli/micro * 0.5 * 346 meter/sec
  //        = 100,000 * 0.001 * 0.5 * 346
  //        = 17,300 mm  ==> 17.3m
