#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
const char* AP_SSID     = "HUAWEI-0409E6";         // XXXXXX -- 使用时请修改为当前你的 wifi ssid
const char* AP_PSK = "15660090095";        // XXXXXX -- 使用时请修改为当前你的 wifi 密码
WiFiClient client;                       //创建一个网络对象
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
  DynamicJsonBuffer jsonBuffer(capacity);

  JsonObject& root = jsonBuffer.parseObject(content);
 
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
  Serial.begin(115200 );//设置波特率
  Serial.println("");//换行
  wifi_start_connect();//连接wifi
  client.setTimeout(5000);//设置服务器连接超时
}

void loop() {
  // put your main code here, to run repeatedly:
  if(client.connect("api.qweather.com", 80)==1)              //连接服务器并判断是否连接成功，若成功就发送GET 请求数据下发       
  {                                           //换成你自己在心知天气申请的私钥//改成你所在城市的拼音
   client.print("GET v7/grid-weather/3d.json?&location=116.41,39.92&key=b9fca499ed9f454ba5ccf59059c661f2$language=zh&unit=m HTTP/1.1\r\n"); //心知天气的URL格式          
   client.print("Host:api.qweather.com\r\n");
   client.print("Accept-Language:zh-cn\r\n");
   client.print("Connection:close\r\n\r\n"); //向心知天气的服务器发送请求。
 
                 
   String status_code = client.readStringUntil('\r');        //读取GET数据，服务器返回的状态码，若成功则返回状态码200
   Serial.println(status_code);
      
   if(client.find("\r\n\r\n")==1)                            //跳过返回的数据头，直接读取后面的JSON数据，
     {
      String json_from_server=client.readStringUntil('\n');  //读取返回的JSON数据
      Serial.println(json_from_server);
      parseUserData(json_from_server);                      //将读取的JSON数据，传送到JSON解析函数中进行显示。
     }
  }
  else                                        
  { 
   Serial.println("connection failed this time");
   delay(5000);                                            //请求失败等5秒
  } 
  client.stop();                                            //关闭HTTP客户端，采用HTTP短链接，数据请求完毕后要客户端要主动断开https://blog.csdn.net/huangjin0507/article/details/52396580
  delay(5000);      
}
