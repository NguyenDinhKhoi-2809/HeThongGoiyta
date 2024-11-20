// CHƯƠNG TRÌNH CHO PHÒNG TRỰC CỦA Y TÁ - BÁC SĨ
// Author: Nguyễn Đình Khôi - Engineering at Hshop.vn

#include <SPI.h>                  // Thư viện hỗ trợ giao thức SPI, được sử dụng bởi module nRF24L01.
#include <nRF24L01.h>             // Thư viện điều khiển module nRF24L01, cho phép giao tiếp không dây với các thiết bị khác sử dụng RF.
#include <RF24.h>                 // Thư viện điều khiển module nRF24L01, cho phép giao tiếp không dây với các thiết bị khác sử dụng RF.
#include <Wire.h>                 // Thư viện hỗ trợ giao thức I2C
#include <LiquidCrystal_I2C.h>    // Thư viện giúp điều khiển màn hình LCD qua giao thức I2C.
#include "SoftwareSerial.h"       // Thư viện tạo cổng nối tiếp phần mềm, dùng cho giao tiếp với module DFPlayer Mini (đọc thẻ MP3).
#include "DFRobotDFPlayerMini.h"  // Thư viện hỗ trợ điều khiển module DFPlayer Mini, phát nhạc từ thẻ microSD.

// Địa chỉ các kênh RF24
const uint64_t pipeIn1 = 0xE8E8F0F0E1LL;   // Nhận tín hiệu từ phòng 1
const uint64_t pipeIn2 = 0xE8E8F0F0E2LL;   // Nhận tín hiệu từ phòng 2
const uint64_t pipeOut1 = 0xE8E8F0F0E3LL;  // Gửi phản hồi về phòng 1
const uint64_t pipeOut2 = 0xE8E8F0F0E4LL;  // Gửi phản hồi về phòng 2

RF24 radio(9, 10);  // Khởi tạo đối tượng radio sử dụng các chân 9 và 10 trên Arduino để giao tiếp với module RF24.
int coi = 6;         // Còi báo động nối chân số 6 của Arduino

LiquidCrystal_I2C lcd(0x27, 20, 4);     // Khởi tạo màn hình LCD với địa chỉ I2C là 0x27 và kích thước màn hình là 20x4.
SoftwareSerial mySoftwareSerial(8, 7);  // Khởi tạo cổng nối tiếp phần mềm để giao tiếp với DFPlayer Mini qua các chân 8 (RX) và 7 (TX).
DFRobotDFPlayerMini myDFPlayer;         // Khởi tạo đối tượng myDFPlayer để điều khiển module DFPlayer Mini.

// Biến cho còi tít tít
unsigned long previousMillis = 0;       // Biến lưu thời gian trước đó cho chu kỳ tít tít
unsigned long coiStartMillis = 0;       // Biến lưu thời gian bắt đầu của còi
const unsigned long coiDuration = 50000000000; // Tổng thời gian còi hoạt động (ms)
const unsigned long beepInterval = 150;  // Thời gian bật/tắt mỗi chu kỳ tít (ms)
bool coiActive = false;                  // Trạng thái còi
bool coiState = false;                   // Trạng thái bật/tắt của chu kỳ còi

void setup() {
  // Cấu hình chân còi làm output và tắt còi ban đầu
  pinMode(coi, OUTPUT);
  //digitalWrite(coi, LOW); // Tắt còi ban đầu

  // Khởi động giao tiếp I2C và LCD
  Wire.begin();
  lcd.init();
  lcd.backlight();
  lcd.begin(20, 4);
  lcd.setCursor(0, 0);
  lcd.print(" He thong goi y ta ");
  lcd.setCursor(0, 1);
  lcd.print(" Benh vien da khoa ");
  lcd.setCursor(0, 2);
  lcd.print(" Xin chao quy khach ");
  lcd.setCursor(0, 3);
  lcd.print("      Welcome ! ");

  // Khởi động RF24
  radio.begin();
  radio.openReadingPipe(1, pipeIn1);  // Mở kênh nhận tín hiệu từ pipeIn1
  radio.openReadingPipe(2, pipeIn2);  // Mở kênh nhận tín hiệu từ pipeIn2
  radio.setPALevel(RF24_PA_HIGH);     // Cấu hình mức công suất cao
  radio.startListening();             // Bắt đầu lắng nghe tín hiệu từ các phòng
}

void loop() {
  // Kiểm tra xem có tín hiệu từ RF24 không
  if (radio.available()) {
    int buttonNumber;
    radio.read(&buttonNumber, sizeof(buttonNumber)); // Đọc tín hiệu nút nhấn
delay(100);
    // Gửi phản hồi về phòng tương ứng
    uint64_t responsePipe = (buttonNumber <= 2) ? pipeOut1 : pipeOut2; // Chọn kênh phản hồi
    radio.stopListening();                                              // Dừng lắng nghe để gửi phản hồi
    radio.openWritingPipe(responsePipe);                                // Mở kênh phản hồi
    radio.write(&buttonNumber, sizeof(buttonNumber));                   // Gửi tín hiệu phản hồi về các phòng tương ứng
    radio.startListening();                                             // Bắt đầu lắng nghe lại

    // Kích hoạt còi với chu kỳ tít tít
    coiActive = true;
    coiStartMillis = millis();
    previousMillis = millis();
    coiState = true; // Bắt đầu với còi bật
    

    // Hiển thị thông tin và phát âm thanh
    displayAndPlay(buttonNumber);
  }

  // Điều khiển chu kỳ tít tít của còi
  if (coiActive) {
    unsigned long currentMillis = millis();
    
    // Kiểm tra nếu hết thời gian bật/tắt mỗi chu kỳ
    if (currentMillis - previousMillis >= beepInterval) {
      coiState = !coiState; // Đảo trạng thái bật/tắt
      digitalWrite(coi, coiState ? HIGH : LOW);
      previousMillis = currentMillis;
    }

    // Tắt còi sau thời gian tổng
    if (currentMillis - coiStartMillis >= coiDuration) {
      digitalWrite(coi, LOW); // Tắt còi
      coiActive = false;      // Dừng trạng thái còi
    }
  }
}

// Hàm xử lý hiển thị trên LCD và phát âm thanh
void displayAndPlay(int buttonNumber) {
  lcd.clear();
  mySoftwareSerial.begin(9600);
  
  // Khởi động DFPlayer Mini
  if (!myDFPlayer.begin(mySoftwareSerial, true, false)) { // true: reset khi bắt đầu, false: không kiểm tra chỗ để lưu trữ
    while (true) { delay(0); } // Dừng chương trình nếu không kết nối được DFPlayer
  }
  
  myDFPlayer.volume(30); // Điều chỉnh âm lượng (0 đến 30)

  // Xử lý theo nút nhấn
  switch (buttonNumber) {
    case 1:
      lcd.setCursor(1, 0);
      lcd.print("GIUONG 1 - Phong 1 ");
      lcd.setCursor(0, 2);
      lcd.print(" Can su giup do ");
      myDFPlayer.play(1); // Phát âm thanh số 001 
      break;
    case 2:
      lcd.setCursor(1, 0);
      lcd.print("GIUONG 2 - Phong 1 ");
      lcd.setCursor(0, 2);
      lcd.print(" Can su giup do ");
      myDFPlayer.play(2); // Phát âm thanh số 002
      break;
    case 3:
      lcd.setCursor(1, 0);
      lcd.print("GIUONG 1 - Phong 2 ");
      lcd.setCursor(0, 2);
      lcd.print(" Can su giup do ");
      myDFPlayer.play(3); // Phát âm thanh số 003
      break;
    case 4:
      lcd.setCursor(1, 0);
      lcd.print("GIUONG 2 - Phong 2 ");
      lcd.setCursor(0, 2);
      lcd.print(" Can su giup do ");
      myDFPlayer.play(4); // Phát âm thanh số 004
      break;
    default:
      // Xử lý trường hợp không xác định nếu cần
      break;
  }
}
