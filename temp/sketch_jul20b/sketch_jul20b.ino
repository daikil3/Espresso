#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include "secrets.h"



const int analogPin = 36;
const float seriesResistor = 10000.0;
const float nominalResistance = 10000.0;
const float nominalTemperature = 25.0;
const float bCoefficient = 3950.0;
const int adcMax = 4095;
const float vcc = 3.3;

AsyncWebServer server(80);

// HTMLページ（次で説明）
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8">
  <title>温度モニター</title>
</head>
<body>
  <h1>現在の温度</h1>
  <p id="temp">読み込み中...</p>

  <script>
    function fetchTemperature() {
      fetch('/temperature')
        .then(response => response.json())
        .then(json => {
          document.getElementById('temp').textContent = json.temperature.toFixed(2) + ' °C';
        })
        .catch(error => {
          document.getElementById('temp').textContent = '取得失敗';
          console.error(error);
        });
    }

    setInterval(fetchTemperature, 2000); // 2秒ごとに更新
    fetchTemperature(); // 初回呼び出し
  </script>
</body>
</html>
)rawliteral";

float readTemperature() {
  int adcValue = analogRead(analogPin);
  float voltage = adcValue * vcc / adcMax;
  float resistance = seriesResistor / ((vcc / voltage) - 1.0);

  float steinhart;
  steinhart = resistance / nominalResistance;
  steinhart = log(steinhart);
  steinhart /= bCoefficient;
  steinhart += 1.0 / (nominalTemperature + 273.15);
  steinhart = 1.0 / steinhart;
  steinhart -= 273.15;
  return steinhart;
}

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("Connected! IP address: ");
  Serial.println(WiFi.localIP());

  // ルートアクセスにHTMLページを返す
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html);
  });

  // /temperature に温度JSONを返す
  server.on("/temperature", HTTP_GET, [](AsyncWebServerRequest *request){
    float temp = readTemperature();
    StaticJsonDocument<200> jsonDoc;
    jsonDoc["temperature"] = temp;
    String jsonStr;
    serializeJson(jsonDoc, jsonStr);
    request->send(200, "application/json", jsonStr);
  });

  server.begin();
}

void loop() {
  // 特に何もしなくてOK（非同期サーバーなので）
}

