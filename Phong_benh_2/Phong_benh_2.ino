#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
const uint64_t pipeOut = 0xE8E8F0F0E2LL;
const uint64_t pipeIn = 0xE8E8F0F0E4LL; // Pipe for receivingacknowledgment
RF24 radio(9, 10);
const int button1Pin = 2;
const int button2Pin = 3;
const int ledPin = 4;
void setup() {
pinMode(button1Pin, INPUT_PULLUP);
pinMode(button2Pin, INPUT_PULLUP);
pinMode(ledPin, OUTPUT);
radio.begin();
radio.openWritingPipe(pipeOut);
radio.openReadingPipe(1, pipeIn);
radio.setPALevel(RF24_PA_HIGH);
radio.stopListening();
}
void loop() {
if (digitalRead(button1Pin) == LOW) {
sendSignal(3);
}
if (digitalRead(button2Pin) == LOW) {
sendSignal(4);
}
}
void sendSignal(int buttonNumber) {
if (radio.write(&buttonNumber, sizeof(buttonNumber))) {
int receivedSignal;
radio.startListening();
unsigned long startedWaiting = millis();
bool timeout = false;
while (!radio.available()) {
if (millis() - startedWaiting > 2000) {
timeout = true;
break;
}
}
if (!timeout) {
radio.read(&receivedSignal, sizeof(receivedSignal));
if (receivedSignal == buttonNumber) {
digitalWrite(ledPin, HIGH);
}
}
radio.stopListening();
}
}
