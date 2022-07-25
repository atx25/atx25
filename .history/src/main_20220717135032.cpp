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
  const size_t capacity =2*JSON_ARRAY_SIZE(1) + JSON_OBJECT_SIZE(2) + JSON_OBJECT_SIZE(5) +
                          JSON_OBJECT_SIZE(15) + 350;
  DynamicJsonBuffer jsconBuffer(capacity);

  JsonObject& root = jsconBuffer.parseObject(content);
 
  JsonObject& results_0 = root["results"][0];
  const char* results_0_code = results_0["code"];
  const char* results_0_updateTime = results_0["updatTime"];
  const char* results_0_fxLink = results_0["fxLink"];
  
  JsonObject& results_0_daily = results_0["daily"];

  const char* results_0_daily_textDay = results_0_daily["textDay"];
  const char* results_0_daily_textNight = results_0_daily["textNight"];
  const char* results_0_daily_fxData = results_0["fxData"];

  Serial.println(results_0_code);                       //通过串口打印出需要的信息
  Serial.println(results_0_fxLink);
  Serial.println(results_0_daily_textDay);
  Serial.println(results_0_daily_textNight);
  Serial.println(results_0_daily_fxData);
  Serial.print("\r\n");
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
  String url = "https://api.qweather.com/v7/grid-weather/3d.json?&location=116.41,39.92&key=b9fca499ed9f454ba5ccf59059c661f2";
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
