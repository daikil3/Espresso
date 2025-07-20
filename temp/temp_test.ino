const int analogPin = 36; // GPIO36 (VP)
const float seriesResistor = 10000.0; // 10kΩ
const float nominalResistance = 10000.0; // 25℃
const float nominalTemperature = 25.0; // 25℃
const float bCoefficient = 3950.0; // B定数
const int adcMax = 4095;
const float vcc = 3.3; // ADC基準電圧

void setup() {
  Serial.begin(115200);
}

void loop() {
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

  Serial.print("Temperature: ");
  Serial.print(steinhart);
  Serial.println(" °C");

  delay(1000);
}