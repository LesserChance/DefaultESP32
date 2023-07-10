#include <Arduino.h>
#include <SSIDSetup.h>

SSIDSetup server;

void setup() {
    Serial.begin(115200);
    Serial.println("Starting...");
    server = SSIDSetup(true);
    server.setup();
}

void loop() {}