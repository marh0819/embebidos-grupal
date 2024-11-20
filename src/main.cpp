#include <WiFi.h>

// Configura el SSID (nombre de la red) y la contraseña del punto de acceso
const char* ssid = "ESP32_Miguel"; // Cambia este nombre para cada ESP32 (ejemplo: ESP32_B, ESP32_C, ESP32_D)
const char* password = "12345678"; // Contraseña para conectarse al AP

void setup() {
    Serial.begin(115200);

    // Configura el ESP32 como punto de acceso
    WiFi.softAP(ssid, password);

    // Obtén la IP asignada al punto de acceso
    IPAddress IP = WiFi.softAPIP();
    Serial.print("Punto de acceso configurado. Dirección IP: ");
    Serial.println(IP);
}

void loop() {
    // Mantén el punto de acceso activo
    delay(1000);
}
