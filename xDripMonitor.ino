#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <Arduino_JSON.h>
#include <Arduino.h>
#include <U8g2lib.h>
#include <Wire.h>

#include <stdint.h>
#include <time.h>

// Parameters
#include "settings.h"
// Prototypes
#include "xDripMonitor.h"

// WIFI settings
const char* ssid = WIFI_SSID;
const char* password = WIFI_PASS;

// NTP server to request epoch time
const char* ntpServer = "pool.ntp.org";
// Variable to save current epoch time
unsigned long epochTime = 0; 

//Your Domain name with URL path or IP address with path
String hostName = XDRIP_HOSTNAME;
String serverName;
const char* apiSecret = XDRIP_API_SECRET;

// the following variables are unsigned longs because the time, measured in
// milliseconds, will quickly become a bigger number than can be stored in an int.
unsigned long lastTime = 0;

// main loop timer
unsigned long timerDelay = XDRIP_POLLING_INTERVAL;

// DISPLAY
U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0,/* clock or SCL=*/OLED_SCL,/* data or SDA=*/OLED_SDA,/* reset=*/ U8X8_PIN_NONE);

unsigned long currentMillis = 0;   // stores the value of millis() in each iteration of loop()
unsigned long delayedMillis = 0;   // stores the value of millis() in each iteration of loop() delayed from wificonnection
const int wifiBlinkInterval = 500; // number of millisecs between blinks 
const int wifiRetryInterval = 500; // number of millisecs between retry to connect to wifi 
unsigned long previousWifiBlinkMillis = 0;
bool wifiBlinkStatus = false;

// POSITIONS                    { x,y }
enum axis {x,y};
const int arrowUpPosition[]   = {95,11};
const int arrowFlatPosition[] = {95,27};
const int arrowDownPosition[] = {95,42};

const int deltaPosition[]     = { 0,57};
const int datePosition[]      = {50,57};

// OLED DISPLAY VALUES
enum values {sgv,delta,direction,minutesElapsed};
String curValues[] = {"","","",""};
String oldValues[] = {"","","",""};
String deltaUnitOfMeasure = "mg/dl";
String minutesElapsedSuffix = "min fa";

void setup() {
  Serial.begin(115200);
  serverName = XDRIP_PROTOCOL;
  serverName+= "://";
  serverName+= XDRIP_HOSTNAME;
  serverName+= ":";
  serverName+= XDRIP_PORT;
  
  // clear display
  u8g2.begin();
  u8g2.enableUTF8Print();

  // lower the brightness to minimum value
  u8g2.setContrast(1);
  u8g2.clearBuffer();
  u8g2.sendBuffer();

  WiFi.begin(ssid, password);
  configTime(0, 0, ntpServer);
}

void loop() {
  if (lastTime == 0) {
    // OLED test
    
    printMissingPhone();
    printWifi();
    printJsonError();
    printArrow("Flat");
    printArrow("FortyFiveDown");
    printArrow("FortyFiveUp");
    printArrow("DoubleUp");
    printArrow("SingleDown");
    printArrow("DoubleDown");
    printDelta(8);
    printDate(8);
    printMainText("888");
    u8g2.sendBuffer();
    delay(5000);

    removeDelta(true);
    printDelta(88, true);
    delay(5000);

    removeDate();
    removeDelta();
    removeArrow();
    removeMainText();
    removeJsonError();
    removeWifi();
    removeMissingPhone();
    
    u8g2.sendBuffer();
  }


  currentMillis = millis();

  // Wifi Status
  if (WiFi.status() != WL_CONNECTED) {
    wifiBlink();
    delay(wifiRetryInterval);
    return;
  }
  if (!wifiBlinkStatus) printWifi(true);
  

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
      removeMissingPhone(true);
      String response = http.getString();
      JSONVar myObject = JSON.parse(response);
      Serial.println(response);
      // JSON.typeof(jsonVar) can be used to get the type of the var
      if (JSON.typeof(myObject) == "undefined") {
        printMainText("ERR");
        printJsonError();
        http.end();
      } else {
        removeJsonError(true);
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

        printArrow(direction);
        printDelta(delta);
        if (epochTime > 0) {
          printDate(minutesElapsed);
        }
        printMainText(sgv);
        u8g2.sendBuffer();
      }
    } else {
      removeMainText();
      removeDate();
      removeDelta();
      removeArrow();
      printMissingPhone();
      u8g2.sendBuffer();
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

void printDate(int value, bool sendBuffer) {
  String text = String(value);
  if (curValues[minutesElapsed] != text) {
    removeDate();
    u8g2.setDrawColor(1);
    u8g2.setFont(u8g2_font_crox3hb_tn); //9x17
    u8g2.drawStr(datePosition[x],datePosition[y],text.c_str());
    // Serial.println(curValues[minutesElapsed]);
    u8g2.setFont(u8g2_font_04b_03b_tr); //5x6
    int startUOfMX = datePosition[x]+(text.length()*9/2)-(minutesElapsedSuffix.length()*5/2)+4;
    if (startUOfMX < 0) startUOfMX = 0;
    u8g2.drawStr(startUOfMX,62,minutesElapsedSuffix.c_str()); 
    if (sendBuffer) u8g2.sendBuffer();
    curValues[minutesElapsed] = text;
  }
}
void removeDate(bool sendBuffer) {
  u8g2.setDrawColor(0);
  int startX = datePosition[x]-(minutesElapsedSuffix.length()*6/2)+4;
  int startY = datePosition[y]-15;
  int length = (curValues[minutesElapsed].length()+1)*9;
  int length2 = minutesElapsedSuffix.length()*6;
  length = length > length2 ? length : length2;
  int height = 60-datePosition[y]+17+6;
  u8g2.drawBox(startX,startY,length,height);
  if (sendBuffer) u8g2.sendBuffer();
}

void printDelta(int value, bool sendBuffer) {
  String text = String(value);
  if (!text.startsWith("-") && !text.startsWith("+")) {
    text = "+" + text;
  }
  if (curValues[delta] != text) {
    removeDelta();
    u8g2.setDrawColor(1);
    u8g2.setFont(u8g2_font_crox3hb_tn); //9x17
    u8g2.drawStr(deltaPosition[x],deltaPosition[y],text.c_str()); 
    u8g2.setFont(u8g2_font_04b_03b_tr); //5x6
    int startUOfMX = deltaPosition[x]+(text.length()*9/2)-(deltaUnitOfMeasure.length()*5/2);
    if (startUOfMX < 0) startUOfMX = 0;
    u8g2.drawStr(startUOfMX,62,deltaUnitOfMeasure.c_str()); 
    if (sendBuffer) u8g2.sendBuffer();
    Serial.println(curValues[delta] == "");
    curValues[delta] = text;
  }
}
void removeDelta(bool sendBuffer) {
  Serial.println("remove: " + curValues[delta]);
  u8g2.setDrawColor(0);
  int startX = deltaPosition[x];
  int startY = deltaPosition[y]-15;
  int length = (curValues[delta].length()+1)*9;
  int height = 60-deltaPosition[y]+17+6;
  u8g2.drawBox(startX,startY,length,height);
  if (sendBuffer) u8g2.sendBuffer();
}

void printArrow(String direction, bool sendBuffer) {
  removeArrow();
  u8g2.setDrawColor(1);
  u8g2.setFont(u8g2_font_unifont_t_86); // 16x16
  if (direction == "Flat") {
//    Serial.print("Print arrow: ");
//    Serial.println(direction);
    u8g2.drawUTF8(arrowFlatPosition[x],arrowFlatPosition[y],"\u2b0c");
  }
  if (direction == "FortyFiveDown") {
//    Serial.print("Print arrow: ");
//    Serial.println(direction);
    u8g2.drawUTF8(arrowDownPosition[x],arrowDownPosition[y],"\u2b0a");
  }
  if (direction == "FortyFiveUp") {
//    Serial.print("Print arrow: ");
//    Serial.println(direction);
    u8g2.drawUTF8(arrowUpPosition[x],arrowUpPosition[y],"\u2b08");
  }
  if (direction == "SingleDown") {
//    Serial.print("Print arrow: ");
//    Serial.println(direction);
    u8g2.drawUTF8(arrowDownPosition[x],arrowDownPosition[y],"\u2b07");
  }
  if (direction == "SingleUp") {
//    Serial.print("Print arrow: ");
//    Serial.println(direction);
    u8g2.drawUTF8(arrowUpPosition[x],arrowUpPosition[y],"\u2b06");
  }
  if (direction == "DoubleDown") {
//    Serial.print("Print arrow: ");
//    Serial.println(direction);
    u8g2.drawUTF8(arrowDownPosition[x],arrowDownPosition[y],"\u2b07");
    u8g2.drawUTF8(arrowDownPosition[x]+7,arrowDownPosition[y],"\u2b07");
  }
  if (direction == "DoubleUp") {
//    Serial.print("Print arrow: ");
//    Serial.println(direction);
    u8g2.drawUTF8(arrowUpPosition[x],arrowUpPosition[y],"\u2b06");
    u8g2.drawUTF8(arrowUpPosition[x]+7,arrowUpPosition[y],"\u2b06");
  }
  if (sendBuffer) u8g2.sendBuffer();
}
void removeArrow(bool sendBuffer) {
  u8g2.setDrawColor(0);
  u8g2.drawBox(arrowUpPosition[x],0,20,42);
  if (sendBuffer) u8g2.sendBuffer();
}

void printMainText(String text, bool sendBuffer) {
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
  if (sendBuffer) u8g2.sendBuffer();
}
void removeMainText(bool sendBuffer) {
  u8g2.setDrawColor(0);
  u8g2.drawBox(0,0,(30*3),42);
  if (sendBuffer) u8g2.sendBuffer();
}

void wifiBlink() {
  if (currentMillis - previousWifiBlinkMillis >= wifiBlinkInterval) {
    if (!wifiBlinkStatus) {
      printWifi(true);
      Serial.println("Print WiFi");
    } else {
      removeWifi(true);
      Serial.println("Hide WiFi");
    }
    previousWifiBlinkMillis += wifiBlinkInterval;
  }
}
void printWifi(bool sendBuffer) {
  removeWifi();
  u8g2.setDrawColor(1);
  u8g2.setFont(u8g2_font_siji_t_6x10); // 12x12
  u8g2.drawUTF8(118,12,"\ue04b");
  if (sendBuffer) u8g2.sendBuffer();
  wifiBlinkStatus = true;
}
void removeWifi(bool sendBuffer) {
  u8g2.setDrawColor(0);
  u8g2.drawBox(118,0,12,12);
  if (sendBuffer) u8g2.sendBuffer();
  wifiBlinkStatus = false;
}

void printMissingPhone(bool sendBuffer) {
  removeMissingPhone();
  u8g2.setDrawColor(1);
  u8g2.setFont(u8g2_font_siji_t_6x10); // 12x12
  u8g2.drawUTF8(118,26,"\ue141");
  if (sendBuffer) u8g2.sendBuffer();
}
void removeMissingPhone(bool sendBuffer) {
  u8g2.setDrawColor(0);
  u8g2.drawBox(118,14,12,12);
  if (sendBuffer) u8g2.sendBuffer();
}

void printJsonError(bool sendBuffer) {
  u8g2.setDrawColor(1);
  u8g2.setFont(u8g2_font_siji_t_6x10); // 12x12
  u8g2.drawUTF8(118,42,"\ue0ae");
  if (sendBuffer) u8g2.sendBuffer();
}
void removeJsonError(bool sendBuffer) {
  u8g2.setDrawColor(0);
  u8g2.drawBox(118,30,12,12);
  if (sendBuffer) u8g2.sendBuffer();
}