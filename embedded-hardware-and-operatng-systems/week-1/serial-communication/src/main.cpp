#include <Arduino.h>

constexpr uint8_t RED_PIN = 3;
constexpr uint8_t GREEN_PIN = 5;
constexpr uint8_t BLUE_PIN = 6;

void setup() {
        Serial.begin(9600);

        pinMode(RED_PIN, OUTPUT);
        pinMode(GREEN_PIN, OUTPUT);
        pinMode(BLUE_PIN, OUTPUT);
}

void loop() {
        while (Serial.available() > 0) {
                int red = Serial.parseInt();
                int green = Serial.parseInt();
                int blue = Serial.parseInt();

                if (Serial.read() == '\n') {
                        red = 255 - constrain(red, 0, 255);
                        green = 255 - constrain(green, 0, 255);
                        blue = 255 - constrain(blue, 0, 255);

                        analogWrite(RED_PIN, red);
                        analogWrite(GREEN_PIN, green);
                        analogWrite(BLUE_PIN, blue);

                        Serial.print(red, HEX);
                        Serial.print(green, HEX);
                        Serial.println(blue, HEX);
                }
        }
}