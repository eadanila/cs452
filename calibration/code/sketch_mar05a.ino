#define NUM_LASERS 2
#define LASER_LOW 500
#define LASER_HIGH 700

int laser_pin[NUM_LASERS];
int laser_tripped[NUM_LASERS];
int state[NUM_LASERS];
int velocity[NUM_LASERS];
int timer[NUM_LASERS];
int new_timer[NUM_LASERS];
int laser[NUM_LASERS];

byte incomingByte[2];

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  laser_pin[0] = 0;
  laser_pin[1] = 1;
}

void loop() {
  // put your main code here, to run repeatedly:

  for (int i = 0; i < NUM_LASERS; i++) {
    laser[i] = analogRead(i);
    new_timer[i] = millis();

    if (laser[i] < LASER_LOW and state[i] == 1) {
      timer[i] = new_timer[i];
      state[i] = 0;
      laser_tripped[i] = 1;;
    } else if (laser[i] > LASER_HIGH and state[i] == 0) {
      velocity[i] = millis() - timer[i];
      state[i] = 1;
    }
  }
  
  if (Serial.available() > 0) {
    Serial.readBytes(incomingByte, 2);

    if (incomingByte[1] == 't') {
      Serial.println(laser_tripped[incomingByte[0]]);
      laser_tripped[incomingByte[0]] = 0;
    }
    else if (incomingByte[1] == 'r') {
      Serial.println(analogRead(incomingByte[0]));
    }
    else if (incomingByte[1] == 'v') {
      Serial.println(velocity[incomingByte[0]]);
    }
    else if (incomingByte[1] == 'c') {
      Serial.println(timer[incomingByte[0]]);
    }
  }
}
