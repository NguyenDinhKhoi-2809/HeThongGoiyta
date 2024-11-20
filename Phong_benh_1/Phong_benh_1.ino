/// CHƯƠNG TRÌNH CHO PHÒNG BỆNH 1
// Author: Nguyễn Đình Khôi - Engineering at Hshop.vn

#include <SPI.h>// Thêm thư viện SPI
#include <nRF24L01.h>// Thêm thư viện NRF24L01
#include <RF24.h>// Thêm thư viện RF24

// Địa chỉ pipe cho truyền và nhận tín hiệu
const uint64_t pipeOut = 0xE8E8F0F0E1LL; // Gửi tín hiệu
const uint64_t pipeIn =  0xE8E8F0F0E3LL; // Nhận phản hồi

RF24 radio(9, 10);// CE=9, CSN=10
const int button1Pin = 2;// Nút nhấn Giường 1
const int button2Pin = 3;// Nút nhấn Giường 2
const int ledPin = 4;   // Led phản hồi Phòng 1

void setup() {
  pinMode(button1Pin, INPUT_PULLUP); // Kéo lên nội bộ cho nút nhấn 1
  pinMode(button2Pin, INPUT_PULLUP); // Kéo lên nội bộ cho nút nhấn 2
  pinMode(ledPin, OUTPUT);// Cấu hình cho đèn báo
  radio.begin();// Khởi tạo module RF24 để bắt đầu hoạt động.
  radio.openWritingPipe(pipeOut);// Thiết lập pipe gửi
  radio.openReadingPipe(1, pipeIn);// Thiết lập pipe nhận
  radio.setPALevel(RF24_PA_HIGH); // Công suất cao
  radio.stopListening();// Chuyển sang chế độ gửi
}
void loop() {
  // Gửi tín hiệu khi nút 1 được nhấn
  if (digitalRead(button1Pin) == LOW) {
    sendSignal(1); // Gửi tín hiệu "1"
  }

  // Gửi tín hiệu khi nút 2 được nhấn
  if (digitalRead(button2Pin) == LOW) {
    sendSignal(2); // Gửi tín hiệu "2"
  }
}
void sendSignal(int buttonNumber) {
  // Gửi tín hiệu qua RF24
  if (radio.write(&buttonNumber, sizeof(buttonNumber))) {
    int receivedSignal;              // Biến lưu tín hiệu phản hồi
    radio.startListening();          // Chuyển sang chế độ nhận
    unsigned long startedWaiting = millis(); // Thời điểm bắt đầu chờ
    bool timeout = false;            // Cờ hết thời gian chờ

    // Chờ phản hồi từ thiết bị nhận
    while (!radio.available()) {
      if (millis() - startedWaiting > 2000) { // Hết thời gian chờ 2 giây
        timeout = true;
        break;
      }
    }

    if (!timeout) {
      radio.read(&receivedSignal, sizeof(receivedSignal)); // Đọc phản hồi
      if (receivedSignal == buttonNumber) { // Kiểm tra phản hồi
        digitalWrite(ledPin, HIGH);         // Bật LED nếu khớp
      }
    }

    radio.stopListening(); // Dừng chế độ nhận, trở về chế độ gửi
  }
}
