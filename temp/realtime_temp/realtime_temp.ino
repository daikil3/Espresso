#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <math.h>  // log関数用
#include "secrets.h"

const int analogPin = 36; // ADCピン
const float seriesResistor = 10000.0;
const float nominalResistance = 10000.0;
const float nominalTemperature = 25.0;
const float bCoefficient = 3950.0;
const int adcMax = 4095;
const float vcc = 3.3;

// 温度読み取り関数
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



AsyncWebServer server(80);

// Chart.jsグラフだけを表示する最小コード
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8">
  <title>Simple Chart Test</title>
  <script src="https://cdn.jsdelivr.net/npm/chart.js@2.9.4/dist/Chart.min.js"></script>
</head>
<body>
  <h2>Temp: <span id="currentTemp">--</span> °C</h2>
  <canvas id="chart" width="400" height="200"></canvas>

  <script>
    var ctx = document.getElementById('chart').getContext('2d');
    var chart = new Chart(ctx, {
      type: 'line',
      data: {
        labels: [],
        datasets: [{
          label: '温度 (°C)',
          data: [],
          borderColor: 'red',
          fill: false
        }]
      },
      options: {
        animation: false,
        scales: {
          xAxes: [{ display: true, scaleLabel: { display: true, labelString: '時間' } }],
          yAxes: [{ display: true, scaleLabel: { display: true, labelString: '温度 (°C)' }, ticks: { suggestedMin: 15, suggestedMax: 35 } }]
        }
      }
    });

    let time = 0;
    setInterval(() => {
      fetch('/temperature')
        .then(response => response.json())
        .then(data => {
          // 現在の温度表示
          document.getElementById('currentTemp').textContent = data.temperature.toFixed(2);

          if (chart.data.labels.length > 30) {
            chart.data.labels.shift();
            chart.data.datasets[0].data.shift();
          }
          chart.data.labels.push(time++);
          chart.data.datasets[0].data.push(data.temperature);
          chart.update();
        })
        .catch(error => console.error('Fetch error:', error));
    }, 2000);
  </script>





  
</body>
</html>
)rawliteral";

void setup() {
  Serial.begin(115200);
  randomSeed(analogRead(0));  // ランダム初期化
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("Connected! IP address: ");
  Serial.println(WiFi.localIP());

  // ルートページのハンドラ
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html);
  });

  server.on("/temperature", HTTP_GET, [](AsyncWebServerRequest *request){
    float temp = readTemperature();

    // JSONを生成
    String json = "{\"temperature\":";
    json += String(temp, 2);
    json += "}";

    // 応答
    request->send(200, "application/json", json);
  });


  server.begin();
}

void loop() {
  // 何もしない
}