#include <WiFi.h>
#include <HTTPClient.h>

// ピン設定
const int trigPin = 3; // 3:ConnectorA 4:ConnectorB
const int echoPin = 1; // 1:ConnectorA 5:ConnectorB
const int LEDPin = 4;   // LEDのピン

// Wi-Fi設定
const char* ssid = "";
const char* password = "";

// LINE Notify設定
const char* lineNotifyEndpoint = "https://notify-api.line.me/api/notify";
const char* lineNotifyToken = "icjP9DTO6iNjRY1oHsn1AuBfn9LD72vQ6sx9SMRZG15";

bool objectDetected = false; // 物体検出フラグ

void setup() {
  Serial.begin(115200);
  pinMode(echoPin, INPUT);
  pinMode(trigPin, OUTPUT);
  pinMode(LEDPin, OUTPUT);

  // Wi-Fi接続
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWi-Fi接続完了");
  Serial.println("IPアドレス: " + WiFi.localIP().toString());
}

void loop() {
  float Duration = 0; // 受信間隔時間
  float Distance = 0; // Durationから計算された距離

  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH); // 超音波出力
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  Duration = pulseIn(echoPin, HIGH); // センサーからの入力
  Duration = Duration / 2; // 往復時間の半分
  Distance = Duration * 340 * 100 / 1000000; // 音速を340 m/sとして設定

  Serial.print("距離: ");
  Serial.print(Distance);
  Serial.println(" cm");

  // 5cm以内に物体がある場合
  if (Distance <= 5 && Distance > 0) { // 0より大きい値をチェックして異常値を除外
    if (!objectDetected) {
      objectDetected = true;
      sendLineNotification(); // LINE通知を送信
    }
    // LEDを点滅
    digitalWrite(LEDPin, HIGH);
    delay(250);
    digitalWrite(LEDPin, LOW);
    delay(250);
  } else {
    objectDetected = false;
    digitalWrite(LEDPin, LOW); // LEDを消灯
  }

  delay(500); // 0.5秒待機
}

// LINE Notify経由で通知を送信
void sendLineNotification() {
  HTTPClient http;
  http.begin(lineNotifyEndpoint);
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  http.addHeader("Authorization", "Bearer " + String(lineNotifyToken));
  
  String message = "人感センサーに反応がありました。セ〇ムが出動します。";
  String payload = "message=" + message;
  
  int httpResponseCode = http.POST(payload);
  
  if (httpResponseCode > 0) {
    Serial.print("LINE通知送信成功。レスポンスコード: ");
    Serial.println(httpResponseCode);
  } else {
    Serial.print("LINE通知送信失敗。エラーコード: ");
    Serial.println(httpResponseCode);
  }
  
  http.end();
}
