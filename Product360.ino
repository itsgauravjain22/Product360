#include <ESP8266WiFi.h>
#include <FS.h>
#include <WiFiClient.h>
#include <TimeLib.h>
#include <NtpClientLib.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ESP8266mDNS.h>
#include <Ticker.h>
#include <ArduinoOTA.h>
#include <ArduinoJson.h>
#include "FSWebServerLib.h"
#include <Hash.h>
#include <AccelStepper.h>

#define fpin 3 //camera focus pin
#define spin 5 //camera shoot pin

// Motor pin definitions
#define motorPin1  15     // IN1 on the ULN2003 driver 1
#define motorPin2  13     // IN2 on the ULN2003 driver 1
#define motorPin3  12     // IN3 on the ULN2003 driver 1
#define motorPin4  14     // IN4 on the ULN2003 driver 1

#define HALFSTEP 8
#define stepsPerRevolution 4096 //steps of stepper motor per revolution
#define anglePerRevolution 360 //degress per revolution
#define timePerRevolution 25 //set time in sec per revolution

//Variable to be used
String temp;
int shootEnable = 0;
int steps = 0;
int photoCount = 0;
int shootInterval = 0;
int speed = stepsPerRevolution / timePerRevolution;
String directionParam;
bool abortVar = false;

// Initialize with pin sequence IN1-IN3-IN2-IN4 for using the AccelStepper with 28BYJ-48
AccelStepper stepper1(HALFSTEP, motorPin1, motorPin3, motorPin2, motorPin4);

// Declare functions to be exposed to the API
void callbackPOST(AsyncWebServerRequest *request);

void setup() {
  Serial.begin(115200); // Start Serial
  // WiFi is started inside library
  SPIFFS.begin(); // Not really needed, checked inside library and started if needed
  ESPHTTPServer.begin(&SPIFFS);
  ESPHTTPServer.setPOSTCallback(callbackPOST); //set optional callback

  //Set trigger pinmode as output and make it low
  pinMode(spin, OUTPUT);
  digitalWrite(spin, LOW);
  //Set trigger pinmode as output and make it low
  pinMode(fpin, OUTPUT);
  digitalWrite(fpin, LOW);
}

void loop() {
  if (abortVar) {
    stepper1.setCurrentPosition(0);
    stepper1.moveTo(0);
    stepper1.setMaxSpeed(700);
    stepper1.setSpeed(speed);
  }
  else {
    if (shootEnable == 0) {
      if (stepper1.distanceToGo() != 0) {
        stepper1.runSpeed();
      }
    } else if (shootEnable == 1) {
      if (stepper1.distanceToGo() != 0 ) {
        if (stepper1.distanceToGo() % (shootInterval * stepsPerRevolution / anglePerRevolution) == 0) {
          shoot();
          stepper1.runSpeed();
        } else if (abs(stepper1.distanceToGo()) < 2 && abs(stepper1.distanceToGo()) > 1) {
          shoot();
        } else {
          stepper1.runSpeed();
        }
      }
    }
  }
  ESPHTTPServer.handle(); // DO NOT REMOVE. Attend OTA update from Arduino IDE
}

//Funtion to shoot photos
void shoot() {
  //focus first
  pinMode(fpin, OUTPUT);
  digitalWrite(fpin, HIGH);
  delay(1000);
  digitalWrite(fpin, LOW);
  //  pinMode(fpin, INPUT);
  // then trigger shoot
  pinMode(spin, OUTPUT);
  digitalWrite(spin, HIGH);
  delay(1000);
  //  pinMode(spin, INPUT);
  digitalWrite(spin, LOW);
}

//Function registers callback bindings
void callbackPOST(AsyncWebServerRequest *request) {
  if (request->url() == "/post/rotate") {
    bool flag1 = false;
    bool flag2 = false;
    if (request->hasParam("steps", true)) {
      temp = request->getParam("steps", true)->value().c_str();
      steps = temp.toInt();
      flag1 = true;
    } else {
      flag1 = false;
      request->send(200, "application/json", "{\"status\":\"failed\",\"message\":\"missing steps in post form-data parameter.\"}");
    }
    if (request->hasParam("direction", true)) {
      directionParam = request->getParam("direction", true)->value().c_str();
      if (directionParam == "cw" || directionParam == "ccw") {
        flag2 = true;
      } else {
        flag2 = false;
        request->send(200, "application/json", "{\"status\":\"failed\",\"message\":\"wrong direction value in post form-data parameter.\"}");
      }
    } else {
      flag2 = false;
      request->send(200, "application/json", "{\"status\":\"failed\",\"message\":\"missing direction in post form-data parameter.\"}");
    }
    if (flag1 && flag2) {
      abortVar = false;
      shootEnable = 0;
      stepper1.setCurrentPosition(0);
      if (directionParam == "cw") {
        stepper1.moveTo(-abs(steps * stepsPerRevolution / anglePerRevolution));
        stepper1.setMaxSpeed(-700);
        stepper1.setSpeed(-speed);
      } else if (directionParam == "ccw") {
        stepper1.moveTo(abs(steps * stepsPerRevolution / anglePerRevolution));
        stepper1.setMaxSpeed(700);
        stepper1.setSpeed(speed);
      }
      request->send(200, "application/json", "{\"status\":\"success\",\"steps\":" + String(steps) + "}");
    }
  } else  if (request->url() == "/post/rotateandshoot") {
    bool flag1 = false;
    bool flag2 = false;
    bool flag3 = false;
    if (request->hasParam("photoCount", true)) {
      temp = request->getParam("photoCount", true)->value().c_str();
      photoCount = temp.toInt();
      flag1 = true;
    } else {
      flag1 = false;
      request->send(200, "application/json", "{\"status\":\"failed\",\"message\":\"missing photoCount in post form-data parameter.\"}");
    }
    if (request->hasParam("shootInterval", true)) {
      temp = request->getParam("shootInterval", true)->value().c_str();
      shootInterval = temp.toInt();
      flag2 = true;
    } else {
      flag2 = false;
      request->send(200, "application/json", "{\"status\":\"failed\",\"message\":\"missing shootInterval in post form-data parameter.\"}");
    }
    if (request->hasParam("direction", true)) {
      directionParam = request->getParam("direction", true)->value().c_str();
      if (directionParam == "cw" || directionParam == "ccw") {
        flag3 = true;
      } else {
        flag3 = false;
        request->send(200, "application/json", "{\"status\":\"failed\",\"message\":\"wrong direction value in post form-data parameter.\"}");
      }
    } else {
      flag3 = false;
      request->send(200, "application/json", "{\"status\":\"failed\",\"message\":\"missing direction in post form-data parameter.\"}");
    }
    if (flag1 && flag2 && flag3) {
      abortVar = false;
      shootEnable = 1;
      stepper1.setCurrentPosition(0);
      if (directionParam == "cw") {
        stepper1.moveTo(-abs(shootInterval * photoCount * stepsPerRevolution / anglePerRevolution));
        stepper1.setMaxSpeed(-700);
        stepper1.setSpeed(-speed);
      } else if (directionParam == "ccw") {
        stepper1.moveTo(abs(shootInterval * photoCount * stepsPerRevolution / anglePerRevolution));
        stepper1.setMaxSpeed(700);
        stepper1.setSpeed(speed);
      }
      request->send(200, "application/json", "{\"status\":\"success\",\"message\":\"" + String(photoCount) + " photos will be shooted for each " + String(shootInterval) + " steps.\"}");
    }
  } else  if (request->url() == "/post/abort") {
    abortVar = true;
    request->send(200, "application/json", "{\"status\":\"success\",\"message\":\"operation aborted.\"}");
  } else  if (request->url() == "/post/status") {
    if (stepper1.distanceToGo() != 0 ) {
      request->send(200, "application/json", "{\"status\":\"success\",\"message\":\"operation running.\"}");
    } else {
      request->send(200, "application/json", "{\"status\":\"success\",\"message\":\"operation stopped.\"}");
    }
  }
}

