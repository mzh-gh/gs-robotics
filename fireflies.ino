/**
   Mark Zhou
   Fireflies
   9.14.22
   Intensive Physical Computing + Robotics Arr. 4
**/

void setup() {
  for (int i = 0; i < 8; i++) {
    pinMode(i, OUTPUT);
  }
  randomSeed(micros());
}

int timer[8], len[8];
bool on[8];

void loop() {
  for (int i = 0; i < 8; i++) {
    if (timer[i] == len[i]) {
      timer[i] = 0;
      len[i] = random(64, 256) * 2;
      on[i] = random(0, 2);
    }

    // 0 to len[i] / 2 - 1, then len[i] / 2 - 1 back to 0 (smooth)
    if (on[i]) {
      analogWrite(i, timer[i] < len[i] / 2 ? timer[i] : len[i] - timer[i] - 1);
    } else {
      digitalWrite(i, LOW);
    }
    timer[i]++;
    delay(1);
  }
}
