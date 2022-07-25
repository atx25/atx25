#include <Arduino.h>
#include <ESP8266HTTPClient.h> 
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
const char* AP_SSID     = "HUAWEI-0409E6";         // XXXXXX -- 使用时请修改为当前你的 wifi ssid
const char* AP_PSK = "15660090095";        // XXXXXX -- 使用时请修改为当前你的 wifi 密码
WiFiClientSecure client; 
HTTPClient https;                      //创建一个网络对象
String content;
void wifi_start_connect()              //连接WIFI
{
  WiFi.mode(WIFI_STA);                 //设置esp8266 工作模式 
  Serial.println("Connecting to ");    //写几句提示
  Serial.println(AP_SSID);
  WiFi.begin(AP_SSID, AP_PSK);         //连接wifi
  WiFi.setAutoConnect(true);
  while (WiFi.status()!= WL_CONNECTED) //这个函数是wifi连接状态，返回wifi链接状态
        {  
         delay(500);
         Serial.print(".");
        }
  Serial.println("WiFi connected IP is");
  Serial.println(WiFi.localIP()); 
}
void parseUserData(String content){
  const size_t capacity = 2*JSON_ARRAY_SIZE(1) + JSON_ARRAY_SIZE(3) + JSON_OBJECT_SIZE(2) +
                          JSON_OBJECT_SIZE(5) + 3*JSON_OBJECT_SIZE(18) + 890;
  DynamicJsonBuffer jsonBuffer(capacity);
 
  JsonObject& root = jsonBuffer.parseObject(content);

  const char* code = root["code"]; // "200"
  const char* updateTime = root["updateTime"]; // "2021-12-16T18:30+08:00"
  const char* fxLink = root["fxLink"]; // "http://hfx.link/1"

  JsonArray& daily = root["daily"];

  JsonObject& daily_0 = daily[0];
  const char* daily_0_fxDate = daily_0["fxDate"]; // "2021-12-16"
  Serial.println(code);                       //通过串口打印出需要的信息
  Serial.println(updateTime);
  Serial.println(fxLink);
  Serial.println(daily_0_fxDate);
  Serial.print("\n");
}
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);//设置波特率
  wifi_start_connect();//连接wifi
  client.setTimeout(5000);//设置服务器连接超时
}
void loop() {
  // put your main code here, to run repeatedly:
  client.setInsecure();
  Serial.println("[HTTPS] begin...");
  String url = "https://api.qweather.com/v7/grid-weather/3d?&location=116.41,39.92&key=b9fca499ed9f454ba5ccf59059c661f2&lang=zh&unit=m";
  if(https.begin(client,url)){
    Serial.println("[HTTPS] GET...");
    int httpCode = https.GET();
    if(httpCode >= 0){
      Serial.printf("[HTTPS] GET... code: %d\n", httpCode);
      if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) { // 服务器响应
        String str = https.getString();
        Serial.println(str);
        parseUserData(str);
      }
    } else { // 错误返回负值
      Serial.printf("[HTTPS] GET... failed, error: %s\n", https.errorToString(httpCode).c_str());
    }
    https.end();
    }
  else { // HTTPS连接失败
    Serial.printf("[HTTPS] Unable to connect\n");
  }  
  delay(10000);
}
