int ref;

// GPIO 設定
#define RELAY_PIN 15           // 繼電器接到 GPIO 15
#define LED_1 17
#define LED_2 27
#define LED_3 9


void setup() {
  Serial.begin(115200);
  ref = (touchRead(T0)+touchRead(T0)+touchRead(T0))/3;
  pinMode(LED_1, OUTPUT);
  pinMode(LED_2, OUTPUT);
  pinMode(LED_3, OUTPUT);
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW); // 初始關閉風扇
}

void loop() {
  digitalWrite(LED_3,HIGH);
  int value = touchRead(T0);
  value = abs(ref - value);
  Serial.println(value);
  Serial.println(value > 15);
  Serial.println("\n");
  if (value > 15) {
    digitalWrite(RELAY_PIN, HIGH);
  }else{
    digitalWrite(RELAY_PIN, LOW);
  }
  delay(100);
  digitalWrite(LED_1,HIGH);
  digitalWrite(LED_2,LOW);
  delay(500);
  digitalWrite(LED_2,HIGH);
  digitalWrite(LED_1,LOW);
  delay(500);
}
