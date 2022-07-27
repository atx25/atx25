#include <Arduino.h>
#include <TimeLib.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <ArduinoJson.h>
#include <ESP8266HTTPClient.h>

const char ssid[] = "HUAWEI-0409E6";//wifi名称
const char pass[] = "15660090095";//wifi密码

#include "font/ZdyLwFont_20.h"
#include "font/FxLED_32.h"

#include "img/pangzi/i0.h"
#include "img/pangzi/i1.h"
#include "img/pangzi/i3.h"
#include "img/pangzi/i4.h"
#include "img/pangzi/i5.h"
#include "img/pangzi/i6.h"
#include "img/pangzi/i7.h"
#include "img/pangzi/i8.h"
#include "img/pangzi/i9.h"

#include "img/temperature.h"
#include "img/humidity.h"
#include "img/watch_top.h"
#include "img/watch_bottom.h"

#include <TFT_eSPI.h> 
#include <SPI.h>

TFT_eSPI tft = TFT_eSPI();
TFT_eSprite clk = TFT_eSprite(&tft);

#include <TJpg_Decoder.h>

uint32_t targetTime = 0;   
byte omm = 99;
boolean initial = 1;
byte xcolon = 0;
String location="411627";
String key="d4055a0ac0848d29ab8bb6e4ad498d7a";
String extensions="base";
unsigned int colour = 0;

uint16_t bgColor = 0xFFFF;

String cityCode = "";//天气城市代码

//NTP服务器
static const char ntpServerName[] = "ntp6.aliyun.com";
const int timeZone = 8;     //东八区

time_t prevDisplay = 0; // 显示时间
unsigned long weaterTime = 0;

WiFiUDP Udp;
unsigned int localPort = 8000;

time_t getNtpTime();
void digitalClockDisplay();
void printDigits(int digits);
void getcityCode();
void getCityWeater();
void scrollBanner();
void scrollText();
void imgAnim();
String num2str(int digits);
void sendNTPpacket(IPAddress &address);

bool tft_output(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t* bitmap)
{
  if ( y >= tft.height() ) return 0;
  tft.pushImage(x, y, w, h, bitmap);
  // Return 1 to decode next block
  return 1;
}

byte loadNum = 6;

void loading(byte delayTime){
  clk.setColorDepth(8);
  
  clk.createSprite(200, 50);
  clk.fillSprite(0x0000);

  clk.drawRoundRect(0,0,200,16,8,0xFFFF);
  clk.fillRoundRect(3,3,loadNum,10,5,0xFFFF);
  clk.setTextDatum(CC_DATUM);
  clk.setTextColor(TFT_GREEN, 0x0000); 
  clk.drawString("Connecting to WiFi",100,40,2);
  clk.pushSprite(20,110);
  clk.deleteSprite();
  loadNum += 1;
  if(loadNum>=194){
    loadNum = 194;
  }
  delay(delayTime);
}

void setup(){
    Serial.begin(115200);

  tft.init();
  tft.setRotation(0);
  tft.fillScreen(0x0000);
  tft.setTextColor(TFT_BLACK, bgColor);

  targetTime = millis() + 1000; 

  Serial.print("正在连接WIFI ");
  Serial.println(ssid);
  WiFi.begin(ssid, pass);

  while (WiFi.status() != WL_CONNECTED) {
    for(byte n=0;n<10;n++){ //每500毫秒检测一次状态
      loading(50);
    }
  }
  while(loadNum < 194){ //让动画走完
    loading(1);
  }

  Serial.print("本地IP: ");
  Serial.println(WiFi.localIP());
  //Serial.println("启动UDP");
  Udp.begin(localPort);
  //Serial.print("端口号: ");
  //Serial.println(Udp.localPort());
  //Serial.println("等待同步...");
  setSyncProvider(getNtpTime);
  setSyncInterval(300);

  
  TJpgDec.setJpgScale(1);
  TJpgDec.setSwapBytes(true);
  TJpgDec.setCallback(tft_output);

  TJpgDec.drawJpg(0,0,watchtop, sizeof(watchtop));
  TJpgDec.drawJpg(0,220,watchbottom, sizeof(watchbottom));

  
  //绘制一个视口
  tft.setViewport(0, 20, 240, 200);
  tft.fillScreen(0x0000);
  tft.fillRoundRect(0,0,240,200,5,bgColor);//实心圆角矩形
  //tft.resetViewport();

  //绘制线框
  tft.drawFastHLine(0,34,240,TFT_BLACK);
  
  tft.drawFastVLine(150,0,34,TFT_BLACK);
  
  tft.drawFastHLine(0,166,240,TFT_BLACK);
  
  tft.drawFastVLine(60,166,34,TFT_BLACK);
  tft.drawFastVLine(160,166,34,TFT_BLACK);
  getcityCode();  //获取城市代码
  
  TJpgDec.drawJpg(161,171,temperature, sizeof(temperature));  //温度图标
  TJpgDec.drawJpg(159,130,humidity, sizeof(humidity));  //湿度图标
}

void loop(){
  
  if (now() != prevDisplay) {
    prevDisplay = now();
    digitalClockDisplay();
  }

  
  if(millis() - weaterTime > 300000){ //5分钟更新一次天气
    weaterTime = millis();
    getCityWeater();
  }
  scrollBanner();
  imgAnim();
}

// 发送HTTP请求并且将服务器响应通过串口输出
void getcityCode(){
 String URL = "http://wgeo.weather.com.cn/ip/?_="+String(now());
  //创建 HTTPClient 对象
  HTTPClient httpClient;
 
  //配置请求地址。此处也可以不使用端口号和PATH而单纯的
  httpClient.begin(URL); 
  
  //设置请求头中的User-Agent
  httpClient.setUserAgent("Mozilla/5.0 (iPhone; CPU iPhone OS 11_0 like Mac OS X) AppleWebKit/604.1.38 (KHTML, like Gecko) Version/11.0 Mobile/15A372 Safari/604.1");
  httpClient.addHeader("Referer", "http://www.weather.com.cn/");
 
  //启动连接并发送HTTP请求
  int httpCode = httpClient.GET();
  Serial.print("Send GET request to URL: ");
  Serial.println(URL);
  
  //如果服务器响应OK则从服务器获取响应体信息并通过串口输出
  if (httpCode == HTTP_CODE_OK) {
    String str = httpClient.getString();
    
    int aa = str.indexOf("id=");
    if(aa>-1){
       //cityCode = str.substring(aa+4,aa+4+9).toInt();
       cityCode = str.substring(aa+4,aa+4+9);
       Serial.println(cityCode); 
       getCityWeater();
    }else{
      Serial.println("获取城市代码失败");  
    }
    
    
  } else {
    Serial.println("请求城市代码错误：");
    Serial.println(httpCode);
  }
 
  //关闭ESP8266与服务器连接
  httpClient.end();
}


// 获取城市天气
void getCityWeater(){
  // client.setInsecure();
 Serial.println("[HTTPS] begin...");
  String url = "https://restapi.amap.com/v3/weather/weatherInfo?&city="
                + location + "&key=" + key + "&extensions=" + extensions;
  if(https.begin(client,url)){
    Serial.println("[HTTPS] GET...");
    int httpCode = https.GET();
    if(httpCode >= 0){
      Serial.printf("[HTTPS] GET... code: %d\n", httpCode);
      if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) { // 服务器响应
        String str = https.getString();
        
        putjson(str.c_str());
      }
    } else { // 错误返回负值
      Serial.printf("[HTTPS] GET... failed, error: %s\n", https.errorToString(httpCode).c_str());
    }
    https.end();
    }
  else { // HTTPS连接失败
    Serial.printf("[HTTPS] Unable to connect\n");
  }
}

String scrollText[6];
//int scrollTextWidth = 0;
//天气信息写到屏幕上
// String input;

StaticJsonDocument<512> doc;

DeserializationError error = deserializeJson(doc, str);

if (error) {
  Serial.print(F("deserializeJson() failed: "));
  Serial.println(error.f_str());
  return;
}

const char* status = doc["status"]; // "1"
const char* count = doc["count"]; // "1"
const char* info = doc["info"]; // "OK"
const char* infocode = doc["infocode"]; // "10000"

JsonObject lives_0 = doc["lives"][0];
const char* lives_0_province = lives_0["province"]; // "河南"
const char* lives_0_city = lives_0["city"]; // "太康县"
const char* lives_0_adcode = lives_0["adcode"]; // "411627"
const char* lives_0_weather = lives_0["weather"]; // "阴"
const char* lives_0_temperature = lives_0["temperature"]; // "33"
const char* lives_0_winddirection = lives_0["winddirection"]; // "东南"
const char* lives_0_windpower = lives_0["windpower"]; // "≤3"
const char* lives_0_humidity = lives_0["humidity"]; // "59"
const char* lives_0_reporttime = lives_0["reporttime"]; // "2022-07-27 15:30:11"

  //TFT_eSprite clkb = TFT_eSprite(&tft);
  
  /***绘制相关文字***/
  clk.setColorDepth(8);
  clk.loadFont(ZdyLwFont_20);
  
  //温度
  clk.createSprite(54, 32); 
  clk.fillSprite(bgColor);
  clk.setTextDatum(CC_DATUM);
  clk.setTextColor(TFT_BLACK, bgColor); 
  clk.drawString(lives_0_humidity.c_str()+"℃",27,16);
  clk.pushSprite(185,168);
  clk.deleteSprite();

  //城市名称
  clk.createSprite(88, 32); 
  clk.fillSprite(bgColor);
  clk.setTextDatum(CC_DATUM);
  clk.setTextColor(TFT_BLACK, bgColor); 
  clk.drawString(lives_0_city.c_str(),44,16);
  clk.pushSprite(151,1);
  clk.deleteSprite();
  

  clk.createSprite(50, 24); 
  clk.fillSprite(bgColor);
  clk.fillRoundRect(0,0,50,24,4,pm25BgColor);
  clk.setTextDatum(CC_DATUM);
  clk.setTextColor(0xFFFF); 
  clk.drawString(lives_0_temperature.c_str(),25,13);
  clk.pushSprite(5,130);
  clk.deleteSprite();

  scrollText[0] = "实时天气 "+lives_0_weather.c_str();
  scrollText[1] = "空气质量 "+lives_0_temperature.c_str();
  scrollText[2] = "风向 "+lives_0_winddirection.c_str();
  //左上角滚动字幕
  //解析第二段JSON

  //Serial.println(sk["ws"].as<String>());
  //横向滚动方式
  //String aa = "今日天气:" + dz["weather"].as<String>() + "，温度:最低" + dz["tempn"].as<String>() + "，最高" + dz["temp"].as<String>() + " 空气质量:" + aqiTxt + "，风向:" + dz["wd"].as<String>() + dz["ws"].as<String>();
  //scrollTextWidth = clk.textWidth(scrollText);
  //Serial.println(aa);
  scrollText[3] = "今日"+lives_0_weather.c_str();
  
  scrollText[4] = "最低温度"+forecasts_0_casts_0_nighttemp.c_str()+"℃";
  scrollText[5] = "最高温度"+forecasts_0_casts_0_daytemp.c_str()+"℃";
  
  //Serial.println(scrollText[0]);
  
  clk.unloadFont();

}

int currentIndex = 0;
int prevTime = 0;
TFT_eSprite clkb = TFT_eSprite(&tft);

void scrollBanner(){
  if(millis() - prevTime > 2500){ //2.5秒切换一次

    if(scrollText[currentIndex]){
  
      clkb.setColorDepth(8);
      clkb.loadFont(ZdyLwFont_20);
      
      for(int pos = 24; pos>0 ; pos--){
        scrollTxt(pos);
      }
      
      clkb.deleteSprite();
      clkb.unloadFont();
  
      if(currentIndex>=5){
        currentIndex = 0;  //回第一个
      }else{
        currentIndex += 1;  //准备切换到下一个  
      }
      //Serial.println(currentIndex);
      
    }
    prevTime = millis();
  }
}

void scrollTxt(int pos){
    clkb.createSprite(148, 24); 
    clkb.fillSprite(bgColor);
    clkb.setTextWrap(false);
    clkb.setTextDatum(CC_DATUM);
    clkb.setTextColor(TFT_BLACK, bgColor); 
    clkb.drawString(scrollText[currentIndex],74,pos+12);
    clkb.pushSprite(2,4);
}

void imgAnim(){
  int x=80,y=94,dt=30;//瘦子版dt=10毫秒 胖子30较为合适

  TJpgDec.drawJpg(x,y,i0, sizeof(i0));
  delay(dt);
  TJpgDec.drawJpg(x,y,i1, sizeof(i1));
  delay(dt);
  TJpgDec.drawJpg(x,y,i2, sizeof(i2));
  delay(dt);
  TJpgDec.drawJpg(x,y,i3, sizeof(i3));
  delay(dt);  
  TJpgDec.drawJpg(x,y,i4, sizeof(i4));
  delay(dt);  
  TJpgDec.drawJpg(x,y,i5, sizeof(i5));
  delay(dt);  
  TJpgDec.drawJpg(x,y,i6, sizeof(i6));
  delay(dt);  
  TJpgDec.drawJpg(x,y,i7, sizeof(i7));
  delay(dt);  
  TJpgDec.drawJpg(x,y,i8, sizeof(i8));
  delay(dt);  
  TJpgDec.drawJpg(x,y,i9, sizeof(i9));
  delay(dt);  

}

void digitalClockDisplay()
{
  
  clk.setColorDepth(8);

  /***中间时间区***/
  //时分
  clk.createSprite(140, 48);
  clk.fillSprite(bgColor);
  //clk.loadFont(FxLED_48);
  clk.setTextDatum(CC_DATUM);
  clk.setTextColor(TFT_BLACK, bgColor);
  clk.drawString(hourMinute(),70,24,7); //绘制时和分
  //clk.unloadFont();
  clk.pushSprite(28,40);
  clk.deleteSprite();
  
  //秒
  clk.createSprite(40, 32);
  clk.fillSprite(bgColor);
  
  clk.loadFont(FxLED_32);
  clk.setTextDatum(CC_DATUM);
  clk.setTextColor(TFT_BLACK, bgColor); 
  clk.drawString(num2str(second()),20,16);
  
  clk.unloadFont();
  clk.pushSprite(170,60);
  clk.deleteSprite();
  /***中间时间区***/

  /***底部***/
  clk.loadFont(ZdyLwFont_20);
  clk.createSprite(58, 32);
  clk.fillSprite(bgColor);

  //星期
  clk.setTextDatum(CC_DATUM);
  clk.setTextColor(TFT_BLACK, bgColor);
  clk.drawString(week(),29,16);
  clk.pushSprite(1,168);
  clk.deleteSprite();
  
  //月日
  clk.createSprite(98, 32);
  clk.fillSprite(bgColor);
  clk.setTextDatum(CC_DATUM);
  clk.setTextColor(TFT_BLACK, bgColor);  
  clk.drawString(monthDay(),49,16);
  clk.pushSprite(61,168);
  clk.deleteSprite();
  
  
  clk.unloadFont();
  /***底部***/
}

//星期
String week(){
  String wk[7] = {"日","一","二","三","四","五","六"};
  String s = "周" + wk[weekday()-1];
  return s;
}

//月日
String monthDay(){
  String s = String(month());
  s = s + "月" + day() + "日";
  return s;
}
//时分
String hourMinute(){
  String s = num2str(hour());
  s = s + ":" + num2str(minute());
  return s;
}

String num2str(int digits)
{
  String s = "";
  if (digits < 10)
    s = s + "0";
  s = s + digits;
  return s;
}

void printDigits(int digits)
{
  Serial.print(":");
  if (digits < 10)
    Serial.print('0');
  Serial.print(digits);
}

/*-------- NTP code ----------*/

const int NTP_PACKET_SIZE = 48; // NTP时间在消息的前48字节中
byte packetBuffer[NTP_PACKET_SIZE]; //buffer to hold incoming & outgoing packets

time_t getNtpTime()
{
  IPAddress ntpServerIP; // NTP server's ip address

  while (Udp.parsePacket() > 0) ; // discard any previously received packets
  //Serial.println("Transmit NTP Request");
  // get a random server from the pool
  WiFi.hostByName(ntpServerName, ntpServerIP);
  //Serial.print(ntpServerName);
  //Serial.print(": ");
  //Serial.println(ntpServerIP);
  sendNTPpacket(ntpServerIP);
  uint32_t beginWait = millis();
  while (millis() - beginWait < 1500) {
    int size = Udp.parsePacket();
    if (size >= NTP_PACKET_SIZE) {
      Serial.println("Receive NTP Response");
      Udp.read(packetBuffer, NTP_PACKET_SIZE);  // read packet into the buffer
      unsigned long secsSince1900;
      // convert four bytes starting at location 40 to a long integer
      secsSince1900 =  (unsigned long)packetBuffer[40] << 24;
      secsSince1900 |= (unsigned long)packetBuffer[41] << 16;
      secsSince1900 |= (unsigned long)packetBuffer[42] << 8;
      secsSince1900 |= (unsigned long)packetBuffer[43];
      //Serial.println(secsSince1900 - 2208988800UL + timeZone * SECS_PER_HOUR);
      return secsSince1900 - 2208988800UL + timeZone * SECS_PER_HOUR;
    }
  }
  Serial.println("No NTP Response :-(");
  return 0; // 无法获取时间时返回0
}

// 向NTP服务器发送请求
void sendNTPpacket(IPAddress &address)
{
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12] = 49;
  packetBuffer[13] = 0x4E;
  packetBuffer[14] = 49;
  packetBuffer[15] = 52;
  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  Udp.beginPacket(address, 123); //NTP requests are to port 123
  Udp.write(packetBuffer, NTP_PACKET_SIZE);
  Udp.endPacket();
}