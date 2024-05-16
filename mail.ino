#include <WiFiClientSecure.h>
#include "env.h"

WiFiClientSecure client;

bool readByClient() {
  auto time = millis();
  while (!client.available()) {
    if (millis() - time >= 10000) {
      return false;
    }
  }
  String line = client.readString();
  Serial.println(line);
  return (line[0] == '2' || line[0] == '3');
}

bool SMTPS(String subject, String body) {
  if (!client.connect(server, port)) {
    Serial.println("Connecting Faild");
    return false;
  }
  if (!readByClient()) return false;

  client.println("HELO localhost");
  if (!readByClient()) return false;

  client.println("AUTH LOGIN");
  if (!readByClient()) return false;

  client.println(username);
  if (!readByClient()) return false;

  client.println(mail_password);
  if (!readByClient()) return false;

  client.print("MAIL FROM:" + from);
  if (!readByClient()) return false;

  client.println("RCPT TO:" + to);
  if (!readByClient()) return false;

  client.println("DATA");
  if (!readByClient()) return false;

  client.println("From: " + from);
  client.println("To: " + to);
  client.println("Subject: " + subject);
  client.println();
  client.print(body);
  client.print("\r\n.\r\n");
  if (!readByClient()) return false;

  client.println("QUIT");
  if (!readByClient()) return false;

  client.stop();
  delay(10);
  return true;
}

void setup() {
  Serial.begin(115200);
  delay(100);

  Serial.print("Attempting to connect to SSID: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  }

  Serial.print("Connected to ");
  Serial.println(ssid);

  client.setInsecure();

  pinMode(13, INPUT_PULLUP);
}

void loop() {
  static bool flg = false;
  if (flg && digitalRead(13) == HIGH) {
    if (!SMTPS("緊急🎉", "ESP32に付けていた線が取り外されました！\r\n大変だ！！！！！")) {
      client.stop();
      Serial.println("mail sending faild");
    }
    flg = false;
  }

  if (digitalRead(13) == LOW) {
    flg = true;
  }

  if (Serial.available() > 0) {
    String body = Serial.readStringUntil('\n');
    if (!SMTPS("Test", body)) {
      client.stop();
      Serial.println("mail sending faild");
    }
  }
}
