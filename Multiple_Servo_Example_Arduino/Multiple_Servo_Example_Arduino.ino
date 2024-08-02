#include <Arduino.h>
#include "driver/ledc.h"

// 定义舵机控制参数
#define SERVO_PIN 13  // 将舵机信号线连接到 ESP32-C3 的引脚
#define SERVO_CHANNEL LEDC_CHANNEL_0
#define SERVO_FREQUENCY 50  // PWM 频率 (Hz)
#define SERVO_RESOLUTION 12  // PWM 分辨率 (位)

#define SERVO_PIN2 12
#define SERVO_CHANNEL2 LEDC_CHANNEL_1

void setup() {
  // 初始化串口通信
  Serial.begin(9600);

  // 配置舵机控制引脚
  ledcSetup(SERVO_CHANNEL, SERVO_FREQUENCY, SERVO_RESOLUTION);
  ledcAttachPin(SERVO_PIN, SERVO_CHANNEL);
  ledcSetup(SERVO_CHANNEL2, SERVO_FREQUENCY, SERVO_RESOLUTION);
  ledcAttachPin(SERVO_PIN2, SERVO_CHANNEL2);
}

void loop() {
  // // 控制舵机转到 0 度
  // for(int d=0;d<2000;d+=10){
  //   ledcWrite(SERVO_CHANNEL,d);
  //   delay(1000); // 等待 1 秒
  // }
  for(int d=200;d<430;d+=10){
    ledcWrite(SERVO_CHANNEL2,d);
    Serial.println(d);
    delay(1000);
  }
}

// void setServoPosition(int degrees) {
//   // 将角度转换为 PWM 脉冲宽度
//   int dutyCycle = map(degrees, 0, 180, 500, 2500); // 500~2500 是舵机的 PWM 范围

//   // 发送 PWM 信号控制舵机
//   ledcWrite(SERVO_CHANNEL, dutyCycle);
// }

