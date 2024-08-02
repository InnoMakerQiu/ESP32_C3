 #include <Wire.h>  // Only needed for Arduino 1.6.5 and earlier
#include "SSD1306Wire.h" // legacy include: `#include "SSD1306.h"`
#include "provinceBitMap.h"
#include "string.h"
#include <WiFiClient.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include <Arduino.h>
#include "driver/ledc.h"
 // Initialize the OLED display using Wire library
 SSD1306Wire  display(0x3c, 5, 4);
 // SH1106 display(0x3c, D3, D5);

//下面定义了oled屏幕的一些基础选项
#define LICENSE_HEIGHT 5 //这里标注了车牌号展示的高度
#define FONT_SIZE 12 //这里展示字体的大小
#define WELCOME_HEIGHT 4 //这里展示了Welcome字体显示的高度



// 定义舵机控制参数
#define SERVO_PIN 12  // 将舵机信号线连接到 ESP32-C3 的引脚
#define SERVO_CHANNEL LEDC_CHANNEL_0
#define SERVO_FREQUENCY 50  // PWM 频率 (Hz)
#define SERVO_RESOLUTION 12  // PWM 分辨率 (位)

// const char* ssid = "xiaocaiji";
// const char* password = "freshman";

// const char* ssid = "ACCOMPANY_206";
// const char* password = "206206206";

const char* ssid = "627";
const char* password = "2020090921627";

//这里我们设置网络服务器为80端口
WebServer server(80);

// 设置 JSON 文档容量
StaticJsonDocument<400> doc;

//这里设置字符转为整形数字的函数
int stringToInteger(const char* str) {
    int result = 0;
    int sign = 1; // 符号位，默认为正数

    // 检查是否为负数
    if (*str == '-') {
        sign = -1;
        str++; // 跳过负号字符
    }

    // 遍历字符串每个字符，计算对应的数字
    while (*str != '\0') {
        if (*str >= '0' && *str <= '9') {
            result = result * 10 + (*str - '0');
        } else {
            // 如果遇到非数字字符，返回0
            return 0;
        }
        str++; // 移动到下一个字符
    }

    return result * sign; // 返回最终结果，包括符号位
}

//这里通过调用setLog函数可以实现scroll
void printWelcome(){
  display.clear();
  for(int index =0;index<4;index++){
    display.drawFastImage(4+index*12,WELCOME_HEIGHT, FONT_SIZE,FONT_SIZE, welcomeText[index]);
  }
  display.display();
}

// void printWelcomeToUestc(){
//   int size = sizeof(WelcomeToUestc)/sizeof(WelcomeToUestc[0]);
//   for()
// }


//在oled屏上显示车牌号对应的省份
void showLicensePlate_Province(uint8_t index){
  display.drawFastImage(4, LICENSE_HEIGHT, FONT_SIZE, FONT_SIZE, provinceText[index]);
}

//在oled屏上显示车牌号其余的字符串
void showLicensePlate_Characters(const char* latterPlate,uint8_t length){
  uint8_t i,index;
  index = latterPlate[0]-'A';
  display.drawFastImage(4+12*1, LICENSE_HEIGHT, FONT_SIZE, FONT_SIZE, capitalLetter[index]);
  display.drawFastImage(4+12*2, LICENSE_HEIGHT, FONT_SIZE, FONT_SIZE, dotLetter);
  i = 1;
  while(i<length){
    Serial.println(latterPlate[i]);
    if(latterPlate[i]<='9'&&latterPlate[i]>='0'){
      index = latterPlate[i] - '0'; 
      display.drawFastImage(2*12+4+12*i, LICENSE_HEIGHT, FONT_SIZE, FONT_SIZE, numText[index]);
    }
    else{
      index = latterPlate[i] - 'A'; 
      display.drawFastImage(2*12+4+12*i, LICENSE_HEIGHT, FONT_SIZE, FONT_SIZE, capitalLetter[index]);
    }
    i++;
  }
}

void executeControlStick(void){
  const char* controlStick = doc["controlStick"];
  if(!strcmp(controlStick,"up")){
    upControlStick(); // Turn the LED off
  }else{
    downControlStick();
  }
}

void upControlStick(){
  ledcWrite(SERVO_CHANNEL,400);
}

void downControlStick(){
  ledcWrite(SERVO_CHANNEL,200);
  delay(3000);
  printWelcome();
}

void executeOutputLicensePlate(void){
  // 获取并打印 licensePlate_Province
  int licensePlate_Province = doc["licensePlate_Province"];
  Serial.print("licensePlate_Province: ");
  Serial.println(licensePlate_Province);

  // 获取并打印 licensePlate_Remaining
  const char* licensePlate_Remaining = doc["licensePlate_Remaining"];
  Serial.print("licensePlate_Remaining: ");
  Serial.println(licensePlate_Remaining);

  //下面进行oled屏展示
  display.clear();
  showLicensePlate_Province(licensePlate_Province);
  showLicensePlate_Characters(licensePlate_Remaining,6);
  display.display();
}

void parseCommand(const char*  jsonString){
  Serial.print("Received JSON data:");
  Serial.println(jsonString);

    // 解析 JSON 数据
  DeserializationError error = deserializeJson(doc, jsonString);

  // 检查是否出现解析错误
  if (error) {
    Serial.print("Failed to parse JSON: ");
    Serial.println(error.c_str());
    return;
  }

  const char* command = doc["command"];
  Serial.println(command);
  if(!strcmp(command,"outputLicensePlate")){
    Serial.println("executeOutputLicensePlate");
    executeOutputLicensePlate();
  }
  else if(!strcmp(command,"controlStick")){
    Serial.println("executeControlStick");
    executeControlStick();
  }
  else{
    Serial.println("The format of the command is error");
  }
}


void handleRoot() {
  server.send(200, "text/plain", "Hello from ESP32C3!");
}

void handlePOST() {
  // 处理POST请求
  String jsonString = server.arg("plain");

  parseCommand(jsonString.c_str()); 

  // 返回响应给客户端
  server.send(200, "text/plain", "JSON data received!");
}

void setup() {
  //初始化串口
  Serial.begin(9600);
  Serial.println("Serial init success");
  
  //初始化OLED
  display.init();
  display.setContrast(255);
  display.flipScreenVertically();
  Serial.println("oled init success");

  //初始化控制杆
  // 配置舵机控制引脚
  ledcSetup(SERVO_CHANNEL, SERVO_FREQUENCY, SERVO_RESOLUTION);
  ledcAttachPin(SERVO_PIN, SERVO_CHANNEL);

  //开始连接wifi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }
  Serial.println("Connected to WiFi");

  server.on("/", HTTP_GET, handleRoot); // 处理根路径的GET请求
  server.on("/post", HTTP_POST, handlePOST); // 处理/post路径的POST请求

  server.begin(); // 开始HTTP服务器
  Serial.println("HTTP server started");
  Serial.println(WiFi.localIP());

//  测试OLED效果
  printWelcome();
  delay(1000);

}

void loop() {
  server.handleClient(); // 处理客户端请求
  delay(2);
}
