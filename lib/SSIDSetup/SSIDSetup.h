#ifndef SSIDSetup_H
#define SSIDSetup_H

#include <Arduino.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <WiFi.h>

#include "SPIFFS.h"

#define RESET 0        // set to 1 to hard reset the SSID
#define FAIL_COUNT 20  // Wifi retry attempts

const String AP_SERVER_DIR = "/ssid/";        // directory for ssid setup assets
const String STATIC_SERVER_DIR = "/server/";  // directory for static assets

class SSIDSetup {
   public:
    SSIDSetup();
    SSIDSetup(bool);

    void setup();
    void initSPIFFS();
    String readFile(fs::FS &fs, const char *path);
    void writeFile(fs::FS &fs, const char *path, const char *message);
    void deleteFile(fs::FS &fs, const char *path);
    bool initWiFi();
    void initSoftAP();
    void initServer();
    void resetSSID();

   private:
    // after SSID setup success, continue to run a web server?
    bool runWebServer;

    // Variables to save values from HTML form
    String ssid;
    String pass;

    // File paths to save input values permanently
    const char *ssidPath = "/ssid.txt";
    const char *passPath = "/pass.txt";
};

#endif