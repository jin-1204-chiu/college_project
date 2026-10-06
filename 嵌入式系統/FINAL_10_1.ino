#include <Arduino.h>
#include <ESP32_Servo.h>
#include <U8g2lib.h>
#include <SimpleDHT.h>

int TOUCH_RELAY, TOUCH_SERVO, TOUCH_MATCH_1, TOUCH_MATCH_2;

// 初始化變數(測驗)
int score = 0; // 分數
int questionNumber = 0; // 問題編號
String userInput = ""; // 使用者輸入
bool isRestarting = false; // 是否重新開始

// 腳位定義
#define OLED_SCL 22
#define OLED_SDA 21
#define DHT11_PIN 14
#define SERVO_PIN 19
#define BUZZER_PIN 25
#define BUTTON_DRAW 16
#define BUTTON_MATCH 17
#define SCREEN_WIDTH 128 //設定OLED螢幕的寬度像素
#define SCREEN_HEIGHT 64 // 設定OLED螢幕的高度像素
#define OLED_RESET -1 / 如果OLED上沒有RESET腳位，將它設置為-1

#define imgWidth 64   
#define imgHeight 64  
#define imgWidth1 64
#define imgHeight1 48 //這裡只用到48的高度，因為上方要放文字
// 物件宣告
Servo myservo;
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);
SimpleDHT11 dht11;

// 狀態變數
bool isDrawMode = false;//抽籤
bool isMatchMode = false;//戀愛匹配
// 控制是否播放心跳或背景音樂
bool isHeartbeatMode = false;
long long heartbeat = 0;
// 讀取 DHT11 資料
byte temperature = 0; // 溫度變數
byte humidity = 0;    // 濕度變數
//觸碰值
int touchValue_motor;

// 問題及選項
const char* questions[] = {
    "Q1: 你喜歡哪種約會場景？\nA) 戶外\nB) 餐廳\nC) 電影\nD) 其他",
    "Q2: 你更喜歡怎樣的伴侶？\nA) 活潑\nB) 溫柔\nC) 理性\nD) 浪漫",
    "Q3: 在假日時，你更傾向於？\nA) 旅遊\nB) 在家放鬆\nC) 運動\nD) 和朋友聚會",
    "Q4: 你覺得愛情中最重要的是什麼？\nA) 信任\nB) 陪伴\nC) 溝通\nD) 自由",
    "Q5: 如果要為伴侶準備驚喜，你會選？\nA) 禮物\nB) 旅行\nC) DIY 手作\nD) 一頓美食"
};

// 結果對應
const char* results[] = {
    "結果A: 你是個喜歡自由自在的人！",
    "結果B: 你重視家庭和親密關係！",
    "結果C: 你是個現實主義者，追求穩定！",
    "結果D: 你是一個浪漫又感性的人！"
};

// oled顯示圖
static const unsigned char PROGMEM love1[]={ 
 /* 0X20,0X01,0X40,0X00,0X40,0X00, */
0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,
0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,
0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,
0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,
0X00,0X00,0X3F,0X00,0X00,0X00,0X00,0X00,0X00,0XE0,0XFF,0X03,0XC0,0XFF,0X01,0X00,
0X00,0XF8,0XFF,0X0F,0XF0,0XFF,0X0F,0X00,0X00,0XFC,0XFF,0X3F,0XFC,0XFF,0X1F,0X00,
0X00,0XFE,0XFF,0X7F,0XFE,0XFF,0X3F,0X00,0X00,0XFE,0XFF,0XFF,0XFF,0XFF,0X7F,0X00,
0X00,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0X00,0X00,0XFF,0XC3,0XFF,0XFF,0X01,0XFF,0X00,
0X80,0XFF,0X01,0XFF,0X3F,0X00,0XFE,0X01,0X80,0XFF,0X00,0XFC,0X0F,0X00,0XFC,0X01,
0X80,0X7F,0X00,0XF0,0X07,0X00,0XF8,0X01,0X80,0X7F,0X00,0XE0,0X03,0X00,0XF8,0X01,
0X80,0X3F,0X00,0X80,0X01,0X0F,0XF0,0X01,0X80,0X3F,0XE0,0X1F,0XF1,0X1F,0XF0,0X01,
0X80,0X3F,0XF0,0X70,0X1C,0X3C,0XF0,0X01,0X80,0X3F,0X30,0XC0,0X06,0X78,0XF0,0X01,
0X80,0X3F,0X18,0X9C,0X7B,0X70,0XF0,0X01,0X80,0X3F,0X18,0X7E,0XFC,0X70,0XF8,0X01,
0X80,0X3F,0X18,0XFE,0XEE,0X30,0XF8,0X01,0X80,0X7F,0X18,0XC3,0XC7,0X31,0XF8,0X01,
0X00,0X7F,0X18,0X83,0XC3,0X18,0XFC,0X00,0X00,0X7F,0X18,0X03,0XE1,0X08,0XFC,0X00,
0X00,0XFE,0X10,0X06,0X70,0X04,0X7E,0X00,0X00,0XFE,0X30,0XDE,0X3B,0X06,0X7F,0X00,
0X00,0XFC,0X61,0XB8,0X1D,0X83,0X3F,0X00,0X00,0XFC,0XC3,0X70,0X8C,0XC0,0X3F,0X00,
0X00,0XF8,0X07,0XE7,0X46,0XF0,0X1F,0X00,0X00,0XF0,0X1F,0XCC,0X23,0XF8,0X0F,0X00,
0X00,0XE0,0X3F,0X90,0X11,0XFC,0X07,0X00,0X00,0XC0,0XFF,0X20,0X09,0XFF,0X01,0X00,
0X00,0X00,0XFF,0X41,0X84,0XFF,0X00,0X00,0X00,0X00,0XFC,0XC3,0XC2,0X3F,0X00,0X00,
0X00,0X00,0XF0,0X87,0XE2,0X1F,0X00,0X00,0X00,0X00,0XE0,0X0F,0XF1,0X07,0X00,0X00,
0X00,0X00,0X80,0X1F,0XF8,0X01,0X00,0X00,0X00,0X00,0X00,0X3F,0XFC,0X00,0X00,0X00,
0X00,0X00,0X00,0X7E,0X3E,0X00,0X00,0X00,0X00,0X00,0X00,0X7C,0X1F,0X00,0X00,0X00,
0X00,0X00,0X00,0XF8,0X0F,0X00,0X00,0X00,0X00,0X00,0X00,0XF0,0X07,0X00,0X00,0X00,
0X00,0X00,0X00,0XC0,0X03,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,
0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,
0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,
0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,
0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,
0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,
0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,
0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,

};
static const unsigned char PROGMEM love2[]={
 /* 0X20,0X01,0X40,0X00,0X40,0X00, */
0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,
0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,
0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,
0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,
0X00,0X80,0X1F,0X00,0X00,0X00,0X00,0X00,0X00,0XF0,0XE0,0X03,0X00,0X07,0X00,0X00,
0X00,0X18,0X00,0X0C,0X60,0XC0,0X07,0X00,0X00,0X0C,0X00,0X10,0X08,0X00,0X18,0X00,
0X00,0X06,0X00,0X20,0X04,0X00,0X20,0X00,0X00,0X03,0X00,0X40,0X03,0X00,0X40,0X00,
0X80,0X01,0X00,0X80,0X01,0X00,0X80,0X00,0XC0,0X00,0X00,0X00,0X00,0XFE,0X00,0X01,
0XC0,0X00,0XFE,0X00,0XC0,0XFF,0X03,0X01,0X40,0X80,0XFF,0X03,0XF0,0XFF,0X07,0X01,
0X40,0X80,0XFF,0X07,0XF8,0XFF,0X07,0X03,0X40,0XC0,0XFF,0X1F,0XFE,0XFF,0X0F,0X02,
0X20,0XC0,0XFF,0X3F,0XFE,0XFF,0X0F,0X02,0X20,0XE0,0XFF,0X7F,0XFF,0XFD,0X0F,0X02,
0X20,0XE0,0X3F,0XF0,0X0F,0XE0,0X0F,0X02,0X20,0XE0,0X9F,0XCF,0XF7,0XC3,0X0F,0X02,
0X20,0XE0,0XCF,0X3F,0XFD,0XCF,0X0F,0X02,0X20,0XE0,0XE7,0X73,0XCE,0XCF,0X0F,0X02,
0X40,0XE0,0XF7,0XC1,0X03,0XCF,0X07,0X03,0X40,0XC0,0XF7,0X98,0X31,0XEF,0X07,0X01,
0X40,0XC0,0XF7,0X3C,0X3C,0XEF,0X07,0X01,0XC0,0XC0,0XF7,0X7C,0X3E,0XF7,0X03,0X01,
0X80,0X80,0XF7,0XFC,0X9F,0XF7,0X83,0X01,0X80,0X80,0XEF,0XB9,0XCD,0XFB,0X81,0X00,
0X00,0X81,0XEF,0X23,0XE6,0XFD,0XC1,0X00,0X00,0X03,0X9F,0X4F,0XF2,0XFE,0X40,0X00,
0X00,0X02,0X7E,0XDF,0XF9,0X3F,0X20,0X00,0X00,0X04,0XFC,0XBD,0XDD,0X1F,0X30,0X00,
0X00,0X08,0XF8,0X73,0XEE,0X07,0X18,0X00,0X00,0X18,0XE0,0X6F,0XF6,0X03,0X0C,0X00,
0X00,0X30,0XC0,0XDF,0XFF,0X01,0X02,0X00,0X00,0XC0,0X80,0XFF,0X7B,0X80,0X01,0X00,
0X00,0X80,0X03,0XFE,0X3D,0X60,0X00,0X00,0X00,0X00,0X0E,0X7C,0X1E,0X18,0X00,0X00,
0X00,0X00,0X18,0XF8,0X0F,0X0C,0X00,0X00,0X00,0X00,0X70,0XF0,0X07,0X03,0X00,0X00,
0X00,0X00,0XE0,0XE0,0X83,0X01,0X00,0X00,0X00,0X00,0X80,0XC1,0X41,0X00,0X00,0X00,
0X00,0X00,0X00,0X83,0X30,0X00,0X00,0X00,0X00,0X00,0X00,0X84,0X18,0X00,0X00,0X00,
0X00,0X00,0X00,0X18,0X04,0X00,0X00,0X00,0X00,0X00,0X00,0X30,0X02,0X00,0X00,0X00,
0X00,0X00,0X00,0XC0,0X01,0X00,0X00,0X00,0X00,0X00,0X00,0X80,0X00,0X00,0X00,0X00,
0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,
0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,
0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,
0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,
0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,
0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,

};
static const unsigned char PROGMEM logo_bmp[] = { /* 0X20,0X01,0X3F,0X00,0X40,0X00, */
0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,
0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,
0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,
0X00,0X00,0X00,0XFC,0XFF,0X03,0X00,0X00,0X00,0X00,0XC0,0X07,0X80,0X1F,0X00,0X00,
0X00,0X00,0X78,0X00,0X00,0X78,0X00,0X00,0X00,0X00,0X1E,0X00,0X00,0XF0,0X00,0X00,
0X00,0X80,0X03,0X00,0X00,0XC0,0X01,0X00,0X00,0XE0,0X00,0X00,0X70,0X80,0X03,0X00,
0X00,0X18,0X00,0X00,0X30,0X00,0X03,0X00,0X00,0X0C,0X00,0X00,0X38,0X00,0X06,0X00,
0X00,0X02,0X00,0X00,0XF4,0X00,0X04,0X00,0X00,0X03,0X00,0X08,0X9B,0X01,0X0C,0X00,
0X00,0X01,0X00,0X84,0X0D,0X03,0X0C,0X00,0X80,0X01,0X00,0X63,0X06,0X06,0X08,0X00,
0X80,0X00,0XE0,0X18,0X03,0X04,0X18,0X00,0XC0,0X60,0X1E,0X87,0X01,0X04,0X18,0X00,
0XC0,0XE0,0XFF,0X60,0X00,0X08,0X10,0X00,0X40,0X20,0XF0,0X1F,0X00,0X10,0X10,0X00,
0X60,0X60,0X80,0X00,0X00,0X10,0X10,0X00,0X20,0X60,0X00,0X00,0X00,0X20,0X10,0X00,
0X20,0X60,0X00,0X00,0X00,0X20,0X10,0X00,0X20,0X20,0X00,0X00,0X00,0X60,0X10,0X00,
0X20,0X20,0X00,0X00,0XE0,0XC7,0X10,0X00,0X20,0X20,0X00,0X00,0XFC,0X0F,0XFB,0X00,
0X20,0X10,0XFC,0X07,0X0E,0X00,0X8E,0X00,0X20,0X10,0X0E,0X00,0X00,0X00,0X86,0X00,
0X20,0X08,0X01,0X08,0XFE,0X9F,0X03,0X01,0X60,0X0C,0XE0,0X0F,0X01,0X70,0X00,0X01,
0XE0,0X07,0X0F,0X08,0X01,0X30,0X00,0X01,0XE0,0X3F,0X01,0X09,0XF9,0X13,0X00,0X01,
0X40,0XC4,0XF9,0X79,0XE1,0X10,0X00,0X01,0X60,0X00,0X71,0X08,0X01,0X10,0X00,0X01,
0X20,0X00,0X01,0X08,0X01,0X10,0X00,0X01,0X20,0X00,0X83,0X07,0XFE,0X1F,0X00,0X01,
0X20,0X00,0XFE,0X80,0X00,0X00,0X80,0X00,0X20,0X00,0X00,0XA0,0X00,0X00,0XC0,0X00,
0X60,0X00,0X00,0XA0,0X00,0X00,0X62,0X00,0X40,0X00,0X00,0X30,0X03,0X00,0X3E,0X00,
0XC0,0X00,0X00,0X10,0X44,0X00,0X0C,0X00,0X80,0X14,0X00,0X08,0X84,0X00,0X06,0X00,
0X80,0X0D,0XC0,0X08,0X04,0X01,0X06,0X00,0X00,0X0F,0X40,0X00,0X04,0X01,0X02,0X00,
0X00,0X08,0X20,0X00,0X00,0X01,0X03,0X00,0X00,0X10,0X20,0X00,0X00,0X01,0X01,0X00,
0X00,0X30,0X20,0X00,0X20,0X80,0X00,0X00,0X00,0X60,0X20,0X04,0X1C,0XC0,0X00,0X00,
0X00,0XC0,0X00,0XFC,0X0F,0X60,0X00,0X00,0X00,0X80,0X01,0XF0,0X01,0X30,0X00,0X00,
0X00,0X00,0X03,0X00,0X00,0X18,0X00,0X00,0X00,0X00,0X02,0X00,0X00,0X0C,0X00,0X00,
0X00,0X00,0X0C,0X00,0X00,0X06,0X00,0X00,0X00,0X00,0X38,0X00,0X00,0X03,0X00,0X00,
0X00,0X00,0XE0,0X01,0XF0,0X00,0X00,0X00,0X00,0X00,0X00,0XFF,0X3F,0X00,0X00,0X00,
0X00,0X00,0X00,0XF0,0X01,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,
0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,
0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,
};


void setup() {
  Serial.begin(115200);
  Serial.println("Merry tree,Marry me!");
  delay(1000);
  Serial.println("歡迎來到戀愛測驗！");
  askQuestion();
  //Touch基準值
  TOUCH_SERVO = touchRead(T0);
  TOUCH_MATCH_1 = touchRead(T4);
  TOUCH_MATCH_2 = touchRead(T9);
  
  // 伺服馬達
  myservo.attach(SERVO_PIN); // 連接伺服馬達到指定腳位
  myservo.write(0); // 初始化伺服馬達位置為 0 度
  delay(500);       // 等待伺服馬達穩定

  //按鈕
  pinMode(BUTTON_DRAW, INPUT_PULLUP);
  pinMode(BUTTON_MATCH, INPUT_PULLUP);
  
  // OLED 初始化
  u8g2.begin();
  u8g2.enableUTF8Print();
  u8g2.setFont(u8g2_font_unifont_t_chinese1);
  u8g2.firstPage();

}

void loop() {
  //檢測戀愛匹配Touch值
    int touchMValue1 = abs(TOUCH_MATCH_1 - touchRead(T4));
    int touchMValue2 = abs(TOUCH_MATCH_2 - touchRead(T9));

  // 監聽用戶輸入(測驗)
  if (Serial.available()) {
    char c = Serial.read(); // 讀取輸入
    if (c == '\n') { // 用戶按下 Enter
      if (isRestarting) {
        processRestartAnswer(userInput); // 處理是否重新開始
      } else {
        processAnswer(userInput); // 處理測驗答案
      }
      userInput = ""; // 清空輸入
    } else {
      userInput += c; // 累積輸入字元
    }
  }
  
  // 讀取溫度顯示初始畫面
    if (!isDrawMode && !isMatchMode) {
      dht11.read(DHT11_PIN, &temperature, &humidity, NULL);
      do {
         u8g2.setCursor(0, 15);
        u8g2.print("- - - - - - - -");
        u8g2.setCursor(20, 30);
        u8g2.print("☆溫度 " + String((int)temperature) + " C");
        u8g2.setCursor(20, 45);
        u8g2.print("☆濕度 " + String((int)humidity) + " %");
        u8g2.setCursor(0, 60);
        u8g2.print("- - - - - - - -");
      } while (u8g2.nextPage());
    }
  
  // 處理按鈕
    if (digitalRead(BUTTON_DRAW) == LOW) {
      isDrawMode = true;
      isMatchMode = false;
    } else if (digitalRead(BUTTON_MATCH) == LOW) {
      isMatchMode = true;
      isDrawMode = false;
    }

  // 抽籤模式
    if (isDrawMode) {
      touchValue_motor = TOUCH_SERVO - touchRead(T0); // 讀取觸摸感測器的值
      u8g2.firstPage();
      do {
        u8g2.setCursor(23, 30);
        u8g2.print("★抽籤中...");
      } while (u8g2.nextPage());
      if (abs(touchValue_motor) > 8) { // 觸碰時
         u8g2.firstPage();
          do {
            u8g2.drawXBMP(30,0, imgWidth, imgHeight, logo_bmp);  //繪圖
          } while (u8g2.nextPage());
            noTone(BUZZER_PIN);
            int currentAngle=myservo.read();
            myservo.attach(SERVO_PIN);
            myservo.write(currentAngle);
            myservo.write(180);  // 馬達轉到 180 度
            delay(500);          // 短暫延遲 0.1 秒
            myservo.write(0);    // 馬達回到 0 度
            long Time3=millis();
            while(millis()-Time3 <= 3750);
            myservo.write(myservo.read());
      }
    }

  // 戀愛匹配模式
    else if (isMatchMode) {
      isHeartbeatMode = false;
      //還沒都觸碰 閃撲通撲通的心
          int a=6;
          while(a--){
            if(a%2==0){
              u8g2.firstPage();
              do {
              u8g2.setCursor(40, 14);
              u8g2.print("匹配中");
              u8g2.drawXBMP(30,16, imgWidth1, imgHeight1, love1);  //繪圖
              }while (u8g2.nextPage());
            }else{
              u8g2.firstPage();
              do {
              u8g2.setCursor(40, 14);
              u8g2.print("匹配中");
               u8g2.drawXBMP(30,16, imgWidth1, imgHeight1, love2);  //繪圖
              }while (u8g2.nextPage());
              
            }
          }
          
      //Touch都觸碰到了
        if (abs(TOUCH_MATCH_1 - touchRead(T4)) > 20 && abs(TOUCH_MATCH_2 - touchRead(T9)) > 20) { 
            long Time=millis(); 
            int love=random(-87,88);
            u8g2.firstPage();
            do {   
              while(millis()-Time<=3000){
                u8g2.setCursor(10, 38);
                u8g2.print("匹配度❤:"+String((int)love)+"%");
                const int beatFreq[]= {60,0,80,0};  // 第一個心跳的頻率 (低音)
                const int beatDurations[] = {100,100,100,600}; // 每個心跳的持續時間 (毫秒)
                for (int i = 0; i < sizeof(beatFreq) / sizeof(beatFreq[0]); i++) {
                  if (digitalRead(BUTTON_DRAW) == LOW) {
                    heartbeat++;
                    break; // 退出循環，停止播放背景音樂
                  }
                // 停止聲音並休息一段時間
                int beatDuration = beatDurations[i];
                tone(BUZZER_PIN, beatFreq[i], beatDuration);
                delay(beatDuration * 1.3);
                noTone(BUZZER_PIN);
                }
              }
          } while (u8g2.nextPage());
           do {// 重複的這段 讓匹配度出現的時間久又不影響歌曲
            while(millis()-Time<=3000){
              u8g2.setCursor(10, 38);
              u8g2.print("匹配度❤:"+String((int)love)+"%");
              //Serial.print("匹配度❤:"+String((int)love)+"%");
              const int beatFreq[]= {60,0,80,0};  // 第一個心跳的頻率 (低音)
              const int beatDurations[] = {100,100,100,600}; // 每個心跳的持續時間 (毫秒)
              for (int i = 0; i < sizeof(beatFreq) / sizeof(beatFreq[0]); i++) {
                if (digitalRead(BUTTON_DRAW) == LOW) {
                  heartbeat++;
                  break; // 退出循環，停止播放背景音樂
                }
              // 停止聲音並休息一段時間
              int beatDuration = beatDurations[i];
              tone(BUZZER_PIN, beatFreq[i], beatDuration);
              delay(beatDuration * 1.3);
              noTone(BUZZER_PIN);
              }
            }
          } while (u8g2.nextPage());
        }
    }
    else {
        // 否則，播放背景音樂
        isHeartbeatMode = false;
      }

      if (heartbeat%2) {
        const int beatFreq[]= {60,0,80,0};  // 第一個心跳的頻率 (低音)
        const int beatDurations[] = {100,100,100,600}; // 每個心跳的持續時間 (毫秒)
        for (int i = 0; i < sizeof(beatFreq) / sizeof(beatFreq[0]); i++) {
          if (digitalRead(BUTTON_DRAW) == LOW) {
            heartbeat++;
            break; // 退出循環，停止播放背景音樂
          }
          // 停止聲音並休息一段時間
          int beatDuration = beatDurations[i];
          tone(BUZZER_PIN, beatFreq[i], beatDuration);
          delay(beatDuration * 1.3);
          noTone(BUZZER_PIN);
        }
      } else if(heartbeat==0){
          //playBackground(); // 播放背景音樂
          const int jingle[] = {1175, 1047, 987, 880, 784, 880, 988, 784, 880, 987, 1046, 880, 987, 880, 784, 740, 784, 1175, 1047, 987, 880, 784, 880, 988, 784, 880, 987, 1046, 880, 987, 880, 784, 740, 784, 880, 987, 1046, 880, 987, 1046, 1174, 880, 987, 1046, 1174, 1319, 1480, 1568, 1480, 1319, 1174};
          const int jingleDurations[] = {3, 8, 4, 4, 4, 4, 4, 4, 8, 8, 8, 8, 3, 8, 4, 4, 2, 3, 8, 4, 4, 4, 4, 4, 4, 8, 8, 8, 8, 3, 8, 4, 4, 2, 3, 8, 4, 4, 3, 8, 4, 4, 8, 8, 4, 8, 8, 4, 4, 4, 2};
          for (int i = 0; i < sizeof(jingle) / sizeof(jingle[0]); i++) {
            if (digitalRead(BUTTON_DRAW) == LOW) {
              heartbeat+=2;
              break; // 退出循環，停止播放背景音樂
            }
            if (digitalRead(BUTTON_MATCH) == LOW) {
              heartbeat++;
              break; // 退出循環，停止播放背景音樂
            }
            int jingleDuration = 1000 / jingleDurations[i];
            tone(BUZZER_PIN, jingle[i]*0.499, jingleDuration);
            delay(jingleDuration * 1.3);
            noTone(BUZZER_PIN);
          }
      }
      else if(heartbeat%2==0 && abs(touchValue_motor) <= 8){
        myservo.write(myservo.read());
        // 豆豆先生音符頻率與持續時間（以毫秒計）66123 66532 16123 6650 66123 66532 16161 6-00 
        int mrmelody[] = {220,220,262,294,330,440,440,392,330,294,262,220,262,294,330,440,440,392,0 };
        int mrDurations[] = { 4,4,6,8,4,4,5,4,5,8,4,4,6,8,4,4,5,6,4};
        for(int i=0;i<sizeof(mrmelody)/sizeof(mrmelody[0]);i++){
          if (digitalRead(BUTTON_MATCH) == LOW) {
            heartbeat++;
            break; // 退出循環，停止播放背景音樂
          }
          int mrDuration=1500/mrDurations[i];
          tone(BUZZER_PIN, mrmelody[i],mrDuration);
          delay(mrDuration*1.3);
          noTone(BUZZER_PIN);
        }
      }
      delay(500);
    }

// 發送問題
void askQuestion() {
  if (questionNumber < 5) {
    Serial.println(questions[questionNumber]);
    Serial.print("請輸入你的選擇 (A/B/C/D): \n");
  } else {
    showResult(); // 顯示結果
  }
}

// 處理用戶輸入的答案
void processAnswer(String answer) {
  answer.trim(); // 去除空白
  if (answer == "A" || answer == "a") {
    score += 1;
  } else if (answer == "B" || answer == "b") {
    score += 2;
  } else if (answer == "C" || answer == "c") {
    score += 3;
  } else if (answer == "D" || answer == "d") {
    score += 4;
  } else {
    Serial.println("無效的輸入，請輸入 A/B/C/D");
    return; // 不增加問題編號
  }
  questionNumber++;
  askQuestion();
}

// 根據得分顯示結果
void showResult() {
  Serial.println("\n測驗結束！你的得分是：" + String(score));
  if (score <= 7) {
    Serial.println(results[0]);
  } else if (score <= 12) {
    Serial.println(results[1]);
  } else if (score <= 17) {
    Serial.println(results[2]);
  } else {
    Serial.println(results[3]);
  }

  // 問是否重新測試
  Serial.println("\n是否再進行一次測驗？\n輸入 Y 繼續，輸入 N 結束：");
  isRestarting = true; // 啟動重新開始流程
}

// 處理是否重新開始測驗的答案
void processRestartAnswer(String answer) {
  answer.trim(); // 去除空白
  if (answer == "Y" || answer == "y") {
    // 清空變數並重新開始
    score = 0;
    questionNumber = 0;
    isRestarting = false;
    Serial.println("\n重新開始測驗！");
    askQuestion();
  } else if (answer == "N" || answer == "n") {
    Serial.println("\n謝謝參與測驗！再見！");
    isRestarting = false;
    while (true) {} // 停止執行
  } else {
    Serial.println("無效的輸入，請輸入 Y 或 N：");
  }
}
    
