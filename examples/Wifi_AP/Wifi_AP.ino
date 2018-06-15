#include <WiFi.h>

#define PIN_STATUS_LED  2

const char *apSsid = "ODROID_GO_AP";
const char *apPasswd = "12345678";

WiFiServer server(80);

void setup() {
    Serial.begin(115200);

    pinMode(PIN_STATUS_LED, OUTPUT);
    digitalWrite(PIN_STATUS_LED, LOW);

    IPAddress gateway(192, 168, 4, 1);
    IPAddress subnet(255, 255, 255, 0);

    if (WiFi.softAP(apSsid, apPasswd)) {
        Serial.println("Wifi AP established.");
        Serial.print("Wifi AP IP: ");
        Serial.println(WiFi.softAPIP());
        Serial.print("AP SSID: ");
        Serial.println(apSsid);
        Serial.print("AP Password: ");
        Serial.println(apPasswd);

        server.begin();
    } else {
        Serial.println("Wifi AP establishing failed.");
    }
}

void loop() {
    WiFiClient client = server.available();

    if (client) {
        Serial.println("New Client.");
        String currentLine = "";
        while (client.connected()) {
            if (client.available()) {
                char c = client.read();
                Serial.write(c);
                if (c == '\n') {
                    if (currentLine.length() == 0) {
                        client.println("HTTP/1.1 200 OK");
                        client.println("Content-type:text/html");
                        client.println();

                        client.print("Click <a href=\"/H\">here</a> to turn the blue status LED on.<br>");
                        client.print("Click <a href=\"/L\">here</a> to turn the blue status LED off.<br>");

                        client.println();
                        break;
                    } else {
                        currentLine = "";
                    }
                } else if (c != '\r') {
                    currentLine += c;
                }

                if (currentLine.endsWith("GET /H")) {
                    digitalWrite(PIN_STATUS_LED, HIGH);
                }
                if (currentLine.endsWith("GET /L")) {
                    digitalWrite(PIN_STATUS_LED, LOW);
                }
            }
        }
        client.stop();
    }
}
