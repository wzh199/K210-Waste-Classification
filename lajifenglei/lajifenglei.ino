
#include <ESPAsyncWebServer.h>    // 包含异步Web服务器库文件

#include <ESP32Servo.h>

AsyncWebServer server(80);        // 创建WebServer对象, 端口号80

#include "HardwareSerial.h"        //调用串口库
#define UART_FULL_THRESH_DEFAULT 2048         //修改缓冲区大小，这个是HardwareSerial.h文件中说的修改方法，我试了，并没有发挥作用
#define CJ_RxPin 26                //设置RX管脚
#define CJ_TxPin 27                //设置TX管脚
HardwareSerial Serial_CJ(1);       //定向串口1


int wasteclass = 0;

const char *ssid = "MyESP32AP";
const char *password = "testpassword";

static const int servoPin = 4;
Servo servo1;

// 使用端口号80可以直接输入IP访问，使用其它端口需要输入IP:端口号访问
// 一个储存网页的数组
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML>
<html>
<head>
  <meta charset="utf-8">
</head>
<body>
  <h2>ESP32 网页</h2>
  <!-- 创建一个ID位dht的盒子用于显示获取到的数据 -->
  <div id="dht">
  </div>
  <button onclick="set()"> 发送数据 </button>
</body>
<script>
  // 按下按钮会运行这个JS函数
  function set() {
    var payload = "ESP32"; // 需要发送的内容
    // 通过get请求给 /set
    var xhr = new XMLHttpRequest();
    xhr.open("GET", "/set?value=" + payload, true);
    xhr.send();
  }
  // 设置一个定时任务, 1000ms执行一次
  setInterval(function () {
    var xhttp = new XMLHttpRequest();
    xhttp.onreadystatechange = function () {
      if (this.readyState == 4 && this.status == 200) {
        // 此代码会搜索ID为dht的组件，然后使用返回内容替换组件内容
        document.getElementById("dht").innerHTML = this.responseText;
      }
    };
    // 使用GET的方式请求 /dht
    xhttp.open("GET", "/dht", true);
    xhttp.send();
  }, 1000)
</script>)rawliteral";


String Merge_Data(void)
{

  // 将温湿度打包为一个HTML显示代码
  String dataBuffer = "<p>";
  dataBuffer += "<h1>垃圾类别 </h1>";
  dataBuffer += "<b> </b>";
  if (wasteclass == 1) {
    dataBuffer += "<br />food</p>";
  }
  if (wasteclass == 2) {
    dataBuffer += "<br />harmful</p>";
  }
  if (wasteclass == 3) {
    dataBuffer += "<br />other</p>";
  }
  if (wasteclass == 4) {
    dataBuffer += "<br />RecyclableWaste</p>";
  }
  dataBuffer += "<br /></p>";
  // 最后要将数组返回出去
  return dataBuffer;
}

// 下发处理回调函数
void Config_Callback(AsyncWebServerRequest *request)
{
  if (request->hasParam("value")) // 如果有值下发
  {
    String HTTP_Payload = request->getParam("value")->value();    // 获取下发的数据
    Serial.printf("[%lu]%s\r\n", millis(), HTTP_Payload.c_str()); // 打印调试信息
  }
  request->send(200, "text/plain", "OK"); // 发送接收成功标志符
}

void Collect_Callback() {
  String Collect_Data = "";                     //定义一个String类型的变量
  
  String food="food";
  String harmful="harmful";
  String other="other";
  String RecyclableWaste="RecyclableWaste";
  
  while (Serial_CJ.available()) {               //用While判断缓冲区是否有内容
    Collect_Data += char(Serial_CJ.read());     //取出缓冲区内容
  }
  Serial.print(Collect_Data);                     //输出取出的内容
  if (0==Collect_Data.compareTo("food")) {
    wasteclass = 1;
  }
  if (0 == Collect_Data.compareTo("harmful")) {
    wasteclass = 2;
  }
  if (0 == Collect_Data.compareTo("other")) {
    wasteclass = 3;

  }
  if (Collect_Data==RecyclableWaste) {
    wasteclass = 4;
  }
Serial.println(wasteclass); 
  Collect_Data = "";                              //清空内容
}


void setup()
{
  // 你需要再此处添加WiFi操作代码，开启热点或者连接到热点
  Serial_CJ.begin(115200, SERIAL_8N1, CJ_RxPin, CJ_TxPin); //初始化串口1，初始化参数可以去HardwareSerial.h文件中查看
  Serial_CJ.onReceive(Collect_Callback);    //定义串口中断函数

  Serial.begin(115200);
  WiFi.softAP(ssid, password);
  Serial.println();
  Serial.print("IPaddress: ");
  Serial.println(WiFi.softAPIP());



  // 添加HTTP主页，当访问的时候会把网页推送给访问者
  server.on("/", HTTP_GET, [](AsyncWebServerRequest * request)
  {
    request->send_P(200, "text/html", index_html);
  });
  // 设置反馈的信息，在HTML请求这个Ip/dht这个链接时，返回打包好的传感器数据
  server.on("/dht", HTTP_GET, [](AsyncWebServerRequest * request)
  {
    request->send_P(200, "text/plain", Merge_Data().c_str());
  });
  server.on("/set", HTTP_GET, Config_Callback);   // 绑定配置下发的处理函数
  server.begin();  // 初始化HTTP服务器
servo1.attach(servoPin);


  servo1.write(0);

  
}


void loop() {

if (wasteclass == 1) {
     servo1.write(0);
  }
  if (wasteclass == 2) {
     servo1.write(65);
  }
  if (wasteclass == 3) {
     servo1.write(115);
  }
  if (wasteclass == 4) {
     servo1.write(175);
  }

 delay(1000);

}
