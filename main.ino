#define BLUEPIN 3
#define GREENPIN 1
#define REDPIN 21

#define LIGHTBARRIER_INDOOR 13 // Sensor "inside" the room
#define LIGHTBARRIER_OUTDOOR 12 // Sensor "outside" the room

void setColor(int r, int g, int b) {
  analogWrite(REDPIN, r);
  analogWrite(GREENPIN, g);
  analogWrite(BLUEPIN, b);
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  pinMode(BLUEPIN, OUTPUT);
  pinMode(GREENPIN, OUTPUT);
  pinMode(REDPIN, OUTPUT);
  
  pinMode(LIGHTBARRIER_INDOOR, INPUT);
  pinMode(LIGHTBARRIER_OUTDOOR, INPUT);

  reset_lightbarrier_sensor();
}

void loop() {
    changeState(is_barrier_open(LIGHTBARRIER_INDOOR, 10), is_barrier_open(LIGHTBARRIER_OUTDOOR, 10));

    switch(get_sensor_state()) {
      case 0:
        setColor(0, 0, 0);
        break;
      case 1:
        setColor(255, 0, 0);
        break;
      case 2:
        setColor(0, 255, 0);
        break;
      case 3:
        setColor(0, 0, 255);
        break;
    }
    delay(100);
}
