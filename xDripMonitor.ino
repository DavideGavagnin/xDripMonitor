#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <Arduino_JSON.h>
#include <Arduino.h>
#include <U8g2lib.h>
#include <Wire.h>

#include <stdint.h>
#include <time.h>

#include "settings.h"

// WIFI settings
const char* ssid = WIFI_SSID;
const char* password = WIFI_PASS;

// NTP server to request epoch time
const char* ntpServer = "pool.ntp.org";
// Variable to save current epoch time
unsigned long epochTime = 0; 

//Your Domain name with URL path or IP address with path
String serverName = "http://motog625g:17580";
const char* apiSecret = XDRIP_API_SECRET;

// the following variables are unsigned longs because the time, measured in
// milliseconds, will quickly become a bigger number than can be stored in an int.
unsigned long lastTime = 0;

// main loop timer - set to 30s
unsigned long timerDelay = 30000;

// DISPLAY
U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0,/* clock or SCL=*/4,/* data or SDA=*/5,/* reset=*/ U8X8_PIN_NONE);

unsigned long currentMillis = 0;   // stores the value of millis() in each iteration of loop()
unsigned long delayedMillis = 0;   // stores the value of millis() in each iteration of loop() delayed from wificonnection
const int wifiBlinkInterval = 500; // number of millisecs between blinks 
const int wifiRetryInterval = 500; // number of millisecs between retry to connect to wifi 
unsigned long previousWifiBlinkMillis = 0;
bool wifiBlinkStatus = false;

// POSITIONS {x,y}
const int arrowUpPosition[] = {95,0};
const int arrowFlatPosition[] = {95,27};
const int arrowDownPosition[] = {95,42};

const int deltaPosition[] = {0,64};
const int datePosition[] = {50,64};

void setup() {
  Serial.begin(115200);
  
  // clear display
  u8g2.begin();
  u8g2.enableUTF8Print();

  // lower the brightness to minimum value
  u8g2.setContrast(1);
  u8g2.clearBuffer();
  u8g2.sendBuffer();

  WiFi.begin(ssid, password);
  configTime(0, 0, ntpServer);
  
  // OLED test
  printMissingPhone();
  printWifi();
  printJsonError();
  printMainText("888");
  printArrow("Flat");
  printArrow("FortyFiveDown");
  printArrow("FortyFiveUp");
  printArrow("DoubleUp");
  printArrow("SingleDown");
  printArrow("DoubleDown");
  printDelta(8);
  printDate(8);
  delay(2000);
  removeDate();
  removeDelta();
  removeArrow();
  removeMainText();
  removeJsonError();
  removeWifi();
  removeMissingPhone();
}

void loop() {
  currentMillis = millis();

  // Wifi Status
  if (WiFi.status() != WL_CONNECTED) {
    wifiBlink();
    delay(wifiRetryInterval);
    return;
  }
  if (!wifiBlinkStatus) printWifi();
  

  // main loop
  /*Serial.println("Main loop timing:");
  Serial.print(currentMillis - lastTime);
  Serial.print(" > ");
  Serial.print(timerDelay);
  Serial.print(" || ");
  Serial.println(lastTime);*/
  if (((currentMillis - lastTime) > timerDelay || lastTime == 0) && WiFi.status() == WL_CONNECTED) {
    WiFiClient client;
    HTTPClient http;

    String serverPath = serverName + "/sgv.json?count=1";

    http.begin(client, serverPath.c_str());
    http.addHeader("api-secret", apiSecret);
    
    Serial.println("GET: " + serverPath);
    int httpResponseCode = http.GET();
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);

    if (httpResponseCode == 200) {
      delayedMillis = millis();
      if (epochTime == 0) { 
        epochTime = getTime();
        Serial.print("Epoch Time: ");
        Serial.println(epochTime);
      }
      removeMissingPhone();
      String response = http.getString();
      JSONVar myObject = JSON.parse(response);
      Serial.println(response);
      // JSON.typeof(jsonVar) can be used to get the type of the var
      if (JSON.typeof(myObject) == "undefined") {
        printMainText("ERR");
        printJsonError();
        http.end();
      } else {
        removeJsonError();
        // Get infos from response
        /*
        String ts = myObject[0]["dateString"];
        int TIndex = ts.indexOf('T');
  //       Serial.println("dateString: " + ts);
    
        String dateEn = ts.substring(0, TIndex);
        int YIndex = dateEn.indexOf('-');
        String year = dateEn.substring(0, YIndex);
  //       Serial.println("year: " + year);

        dateEn.replace(year + '-', "");
  //       Serial.println("dateString: " + dateEn);

        int MIndex = dateEn.indexOf('-');
        String month = dateEn.substring(0, MIndex);
  //       Serial.println("month: " + month);

        dateEn.replace(month + '-', "");
  //       Serial.println("dateString: " + dateEn);
        String day = dateEn;
  //       Serial.println("day: " + day);

        String timeMs = ts.substring(TIndex + 1, ts.length());
        int MsIndex = timeMs.indexOf('.');
        String time = timeMs.substring(0, MsIndex);
*/
        int minutesElapsed = 0;
        if (epochTime > 0) {
          String dateStr = JSON.stringify(myObject[0]["date"]);
          dateStr = dateStr.substring(0, 10);
          Serial.println("dateStr: " + dateStr);
          Serial.print("epochTime: ");
          Serial.println(epochTime);
          Serial.print("delayedMillis: ");
          Serial.println(delayedMillis);
          int now = (delayedMillis/1000) + epochTime;
          Serial.print("now: ");
          Serial.println(now);
          minutesElapsed = (now - (dateStr.toInt()))/60;
          Serial.print("elapsed: ");
          Serial.println(minutesElapsed);
        }

        String sgv = JSON.stringify(myObject[0]["sgv"]);
        int delta = int(myObject[0]["delta"]); // + " mg/dl";
        String direction = JSON.stringify(myObject[0]["direction"]);
        direction.replace("\"", "");
        // String date = day + '/' + month + '/' + year + ' ' + time;

        Serial.println(sgv);
        Serial.println(delta);
        Serial.println(direction);
        Serial.println(minutesElapsed);

        printMainText(sgv);
        printArrow(direction);
        printDelta(delta);
        if (epochTime > 0) {
          printDate(minutesElapsed);
        }
      }
    } else {
      removeMainText();
      removeDate();
      removeDelta();
      removeArrow();
      printMissingPhone();
    }
    http.end();
    lastTime = millis();
  }
}

// Function that gets current epoch time
unsigned long getTime() {
  time_t now;
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    return(0);
  }
  time(&now);
  return now;
}

void printDate(int value) {
  String text = String(value);
  text = text + " min fa";
  removeDate();
  u8g2.setDrawColor(1);
  u8g2.setFont(u8g2_font_courR08_tf); //7x11
  u8g2.drawStr(datePosition[0],datePosition[1],text.c_str());
  u8g2.sendBuffer(); 
}
void removeDate() {
  u8g2.setDrawColor(0);
  u8g2.drawBox(datePosition[0],datePosition[1]-7,120,11);
  u8g2.sendBuffer();
}

void printDelta(int value) {
  String text = String(value);
  if (!text.startsWith("-") && !text.startsWith("+")) {
    text = "+" + text;
  }
  removeDelta();
  u8g2.setDrawColor(1);
  u8g2.setFont(u8g2_font_crox5h_tn); //12x25
  u8g2.drawStr(deltaPosition[0],deltaPosition[1],text.c_str()); 
  u8g2.setFont(u8g2_font_04b_03b_tr); //5x6
  u8g2.drawStr(deltaPosition[0] + (text.length()*15),deltaPosition[1]-12,"mg/dl"); 
  u8g2.sendBuffer(); 
}
void removeDelta() {
  u8g2.setDrawColor(0);
  u8g2.drawBox(deltaPosition[0],deltaPosition[1]-20,26,20);
  u8g2.setDrawColor(0);
  u8g2.drawBox(deltaPosition[0],deltaPosition[1]-20,54,10);
  u8g2.sendBuffer();
}

void printArrow(String direction) {
  removeArrow();
  u8g2.setDrawColor(1);
  u8g2.setFont(u8g2_font_unifont_t_86); // 16x16
  if (direction == "Flat") {
    Serial.print("Print arrow: ");
    Serial.println(direction);
    u8g2.drawUTF8(arrowFlatPosition[0],arrowFlatPosition[1],"\u2b0c");
  }
  if (direction == "FortyFiveDown") {
    Serial.print("Print arrow: ");
    Serial.println(direction);
    u8g2.drawUTF8(arrowDownPosition[0],arrowDownPosition[1],"\u2b0a");
  }
  if (direction == "FortyFiveUp") {
    Serial.print("Print arrow: ");
    Serial.println(direction);
    u8g2.drawUTF8(arrowUpPosition[0],arrowUpPosition[1],"\u2b08");
  }
  if (direction == "SingleDown") {
    Serial.print("Print arrow: ");
    Serial.println(direction);
    u8g2.drawUTF8(arrowDownPosition[0],arrowDownPosition[1],"\u2b07");
  }
  if (direction == "SingleUp") {
    Serial.print("Print arrow: ");
    Serial.println(direction);
    u8g2.drawUTF8(arrowUpPosition[0],arrowUpPosition[1],"\u2b06");
  }
  if (direction == "DoubleDown") {
    Serial.print("Print arrow: ");
    Serial.println(direction);
    u8g2.drawUTF8(arrowDownPosition[0],arrowDownPosition[1],"\u2b07");
    u8g2.drawUTF8(arrowDownPosition[0]+7,arrowDownPosition[1],"\u2b07");
  }
  if (direction == "DoubleUp") {
    Serial.print("Print arrow: ");
    Serial.println(direction);
    u8g2.drawUTF8(arrowUpPosition[0],arrowUpPosition[1],"\u2b06");
    u8g2.drawUTF8(arrowUpPosition[0]+7,arrowUpPosition[1],"\u2b06");
  }
  u8g2.sendBuffer();
}
void removeArrow() {
  u8g2.setDrawColor(0);
  u8g2.drawBox(arrowUpPosition[0],arrowUpPosition[1],20,42);
  u8g2.sendBuffer();
}


void printMainText(String text) {
  Serial.print("Print main text: ");
  Serial.println(text.c_str());
  removeMainText();
  u8g2.setDrawColor(1);
  if (text.toFloat() > 0.00) {
    u8g2.setFont(u8g2_font_7Segments_26x42_mn);
  } else {
    u8g2.setFont(u8g2_font_fur35_tf);
  }
  u8g2.drawStr(0,42,text.c_str());
  u8g2.sendBuffer();
}
void removeMainText() {
  u8g2.setDrawColor(0);
  u8g2.drawBox(0,0,(30*3),42);
  u8g2.sendBuffer();
}

void wifiBlink() {
  if (currentMillis - previousWifiBlinkMillis >= wifiBlinkInterval) {
    if (!wifiBlinkStatus) {
      printWifi();
      Serial.println("Print WiFi");
    } else {
      removeWifi();
      Serial.println("Hide WiFi");
    }
    previousWifiBlinkMillis += wifiBlinkInterval;
  }
}
void printWifi() {
  removeWifi();
  u8g2.setDrawColor(1);
  u8g2.setFont(u8g2_font_siji_t_6x10); // 12x12
  u8g2.drawUTF8(118,12,"\ue04b");
  u8g2.sendBuffer();
  wifiBlinkStatus = true;
}
void removeWifi() {
  u8g2.setDrawColor(0);
  u8g2.drawBox(118,0,12,12);
  u8g2.sendBuffer();
  wifiBlinkStatus = false;
}

void printMissingPhone() {
  removeMissingPhone();
  u8g2.setDrawColor(1);
  u8g2.setFont(u8g2_font_siji_t_6x10); // 12x12
  u8g2.drawUTF8(118,26,"\ue141");
  u8g2.sendBuffer();
}
void removeMissingPhone() {
  u8g2.setDrawColor(0);
  u8g2.drawBox(118,14,12,12);
  u8g2.sendBuffer();
}

void printJsonError() {
  u8g2.setDrawColor(1);
  u8g2.setFont(u8g2_font_siji_t_6x10); // 12x12
  u8g2.drawUTF8(118,42,"\ue0ae");
  u8g2.sendBuffer();
}
void removeJsonError() {
  u8g2.setDrawColor(0);
  u8g2.drawBox(118,30,12,12);
  u8g2.sendBuffer();
}