#include <Arduino.h>

void setup() {
        Serial.begin(9600);
        pinMode(2, INPUT_PULLUP);
        pinMode(13, OUTPUT);
}

void loop() {
        uint8_t value = digitalRead(2);
        Serial.println(value);

        if (value == HIGH) {
                digitalWrite(13, LOW);
        } else {
                digitalWrite(13, HIGH);
        }
}