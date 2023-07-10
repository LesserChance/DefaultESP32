#include "SSIDSetup.h"
AsyncWebServer webserver(80);

SSIDSetup::SSIDSetup() {}
SSIDSetup::SSIDSetup(bool runServer = false) { runWebServer = runServer; }

void SSIDSetup::initSPIFFS() {
    if (!SPIFFS.begin(true)) {
        Serial.println("An error has occurred while mounting SPIFFS");
    }
}

String SSIDSetup::readFile(fs::FS &fs, const char *path) {
    Serial.printf("Reading file: %s\r\n", path);

    File file = fs.open(path);
    if (!file || file.isDirectory()) {
        Serial.println("- failed to open file for reading");
        return String();
    }

    String fileContent;
    while (file.available()) {
        fileContent = file.readStringUntil('\n');
        break;
    }
    return fileContent;
}

// Write file to SPIFFS
void SSIDSetup::writeFile(fs::FS &fs, const char *path, const char *message) {
    Serial.printf("Writing file: %s\r\n", path);

    File file = fs.open(path, FILE_WRITE);
    if (!file) {
        Serial.println("- failed to open file for writing");
        return;
    }
    if (file.print(message)) {
        Serial.println("- file written");
    } else {
        Serial.println("- frite failed");
    }
}

void SSIDSetup::deleteFile(fs::FS &fs, const char *path) {
    Serial.printf("Deleting file: %s\r\n", path);

    if (!fs.exists(path)) {
        Serial.println("- failed to locate file for deleting");
        return;
    }

    if (fs.remove(path)) {
        Serial.println("- file deleted");
    } else {
        Serial.println("- delete failed");
    }
}

// Initialize WiFi
bool SSIDSetup::initWiFi() {
    if (ssid == "") {
        Serial.println("Undefined SSID");
        return false;
    }

    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid.c_str(), pass.c_str());

    Serial.println("Connecting to WiFi...");

    int failCount = 0;
    while (failCount < FAIL_COUNT && WiFi.status() != WL_CONNECTED) {
        failCount++;
        delay(1000);
    }

    if (failCount >= FAIL_COUNT) {
        Serial.println("Failed to connect");
        return false;
    }

    Serial.println("Successfully connected");
    Serial.print("ESP IP Address: ");
    Serial.println(WiFi.localIP());
    return true;
}

void SSIDSetup::initSoftAP() {
    Serial.println("Setting AP (Access Point)");
    WiFi.softAP("ESP", NULL);
    IPAddress IP = WiFi.softAPIP();

    Serial.print("Connect to the network \"ESP\" and visit IP address: ");
    Serial.println(IP);

    // Web Server Root URL
    webserver.on("/", HTTP_GET, [this](AsyncWebServerRequest *request) {
        request->send(SPIFFS, AP_SERVER_DIR + "wifimanager.html", "text/html");
    });

    // serve any static assets
    webserver.serveStatic("/", SPIFFS, STATIC_SERVER_DIR.c_str());

    // handle the post request
    webserver.on("/wifi", HTTP_POST, [this](AsyncWebServerRequest *request) {
        if (!request->hasArg("ssid")) {
            request->send(400, "text/plain",
                          "Missing Required Parameter: ssid");
            return;
        }
        if (!request->hasArg("pass")) {
            request->send(400, "text/plain",
                          "Missing Required Parameter: pass");
            return;
        }

        ssid = request->arg("ssid").c_str();
        pass = request->arg("pass").c_str();

        writeFile(SPIFFS, ssidPath, ssid.c_str());
        writeFile(SPIFFS, passPath, pass.c_str());

        Serial.print("SSID set to: ");
        Serial.println(ssid);

        request->send(SPIFFS, AP_SERVER_DIR + "wifisuccess.html", "text/html");
        delay(3000);

        ESP.restart();
    });

    webserver.begin();
}

void SSIDSetup::initServer() {
    Serial.println("Starting Server...");

    webserver.on("/", HTTP_GET, [this](AsyncWebServerRequest *request) {
        request->send(SPIFFS, STATIC_SERVER_DIR + "index.html", "text/html");
    });

    // serve any static assets from the data/server directory
    webserver.serveStatic("/", SPIFFS, STATIC_SERVER_DIR.c_str());

    webserver.begin();
}

void SSIDSetup::resetSSID() {
    deleteFile(SPIFFS, ssidPath);
    deleteFile(SPIFFS, passPath);
}

void SSIDSetup::setup() {
    initSPIFFS();

    // delete the files if resetting
#if RESET
    resetSSID();
#endif

    // Load values saved in SPIFFS
    ssid = readFile(SPIFFS, ssidPath);
    pass = readFile(SPIFFS, passPath);

    if (initWiFi()) {
        if (runWebServer) {
            initServer();
        }
    } else {
        initSoftAP();
    }
}