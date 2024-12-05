#include "PhoenixJack_lora_v0.h"
#define LORA Serial1

PhoenixJack_LORA lora(22, &LORA);

void setup() {
  Serial.begin(9600);
  delay(2000);
  Serial.println("\n\n\n\n\nSTARBOARD HC12 ONLINE");
  if (lora.initialize()) {
    Serial.println("HC12 ready");
  } else {
    Serial.println("HC12 FAILED");
  }
}

void loop() {
  // put your main code here, to run repeatedly:
}
