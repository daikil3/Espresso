#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "secrets.h"


AsyncWebServer server(80);

// シンプルなHTMLページ（Chart.jsをCDNから読み込むだけ）
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8">
  <title>Simple Chart Test</title>
  <!-- Chart.js v2.9.4をCDNから読み込み -->
  <script src="https://cdn.jsdelivr.net/npm/chart.js@2.9.4/dist/Chart.min.js"></script>
</head>
<body>
  <canvas id="chart" width="400" height="200"></canvas>
  <script>
    var ctx = document.getElementById('chart').getContext('2d');
    var chart = new Chart(ctx, {
      type: 'line',
      data: {
        labels: ['January', 'February', 'March'],
        datasets: [{
          label: 'Demo',
          data: [10, 20, 30],
          borderColor: 'blue',
          fill: false
        }]
      }
    });
  </script>
</body>
</html>
)rawliteral";

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

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html);
  });

  server.begin();
}

void loop() {
  // 非同期サーバーなのでloopは空でOK
}