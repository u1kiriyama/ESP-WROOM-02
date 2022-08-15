/*
 * BME280で5分毎に温度、湿度、気圧を測定し、Ambientに送信する
 * 測定と測定の間はDeep Sleepで待つ
 */
#include <ESP8266WiFi.h>
#include <SPI.h>
#include "BME280_SPI.h"

extern "C" {
#include "user_interface.h"
}

#define BME_CS 15
#define PERIOD 600

BME280 bme280;

#include "ssid.h"
const char* ssid = MY_SSID;
const char* password = MY_SSID_PASS;
const char* remote_host = IP;
const int remote_port = PORT;

void setup()
{
    int t = millis();
    wifi_set_sleep_type(LIGHT_SLEEP_T);

    Serial.begin(115200);
    delay(10);

    Serial.println("Start");

    WiFi.begin(ssid, password);  //  Wi-Fi APに接続
    while (WiFi.status() != WL_CONNECTED) {  //  Wi-Fi AP接続待ち
        delay(100);
    }

    Serial.print("WiFi connected\r\nIP address: ");
    Serial.println(WiFi.localIP());

    bme280.begin(BME_CS);
    double temp=0.0, humid=0.0, pressure=0.0 ,vbat=0.0;

    delay(100); // 追加
    temp = bme280.readTemperature();
    humid = bme280.readHumidity();
    pressure = bme280.readPressure();

    Serial.print("temp: ");
    Serial.print(temp);
    Serial.print(", humid: ");
    Serial.print(humid);
    Serial.print(", pressure: ");
    Serial.print(pressure);
    Serial.print(", vbat: ");
    Serial.println(vbat);
    WiFiClient client;
    if(client.connect(remote_host, remote_port)){
      Serial.println("client.connect OK.");
    }else{
      Serial.println("client.connect error.");
    }
    String request = "/db/store_data.php?point_id=" + WiFi.macAddress() + "&T=" + String(temp) + "&H=" + String(humid) + "&P=" + String(pressure) + "&V=" + String(vbat);
    String req_line = "GET " + request + " HTTP/1.1\r\nHost: " + String(remote_host) + "\r\nConnection: close\r\n\r\n";
    Serial.println(req_line);
  
    client.print(req_line);
    unsigned long timeout = millis();
    while (client.available() == 0) {
      if (millis() - timeout > 5000) {
        Serial.println(">>> Client Timeout !");
        client.stop();
        break;
      }
    }
    Serial.println("proccess finished.");

    t = millis() - t;
    t = (t < PERIOD * 1000) ? (PERIOD * 1000 - t) : 1;
    ESP.deepSleep(t * 1000, RF_DEFAULT);
    delay(1000);
}

void loop()
{
}
