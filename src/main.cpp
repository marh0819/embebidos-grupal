#include <WiFi.h>
#include <ThingSpeak.h>

// Configuración de la red Wi-Fi
const char* ssid = "Familia_Rivera";
const char* password = "Almi1907";

// Configuración de ThingSpeak
unsigned long channelID = 2755551;
const char* WriteAPIKey = "MGZNWM75S9HIPXFE";

WiFiClient cliente;

// Redes Wi-Fi distribuidas
const char* ssidCocina = "ESP32_Cocina";
const char* ssid3Ro = "ESP32_3Ro";
const char* ssidSala = "ESP32_Sala";
const char* ssidMiguel = "ESP32_Miguel";

const int PTx = -30; // Potencia de transmisión en dBm
const float n = 3.0; // Factor ambiental típico para interiores

// Prototipos de funciones
void determinarZona();
int obtenerRSSI(const char* ssid);
float calcularDistancia(int rssi);
int asignarZona(float distCocina, float dist3Ro, float distSala, float distMiguel);
bool resolverDNS();
void verificarWiFi();
bool enviarDatosThingSpeak();

void setup() {
    Serial.begin(115200);
    Serial.println("\nTest de ubicación\n");

    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nWi-Fi conectado");

    if (!resolverDNS()) {
        Serial.println("Error: No se pudo resolver DNS para ThingSpeak");
    }

    ThingSpeak.begin(cliente);
}

void loop() {
    verificarWiFi();

    determinarZona();

    if (!enviarDatosThingSpeak()) {
        Serial.println("Error persistente al enviar datos a ThingSpeak. Reintentar en el próximo ciclo.");
    }

    delay(15000); // Espera 15 segundos
}

void verificarWiFi() {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("Wi-Fi desconectado. Reconectando...");
        WiFi.begin(ssid, password);
        while (WiFi.status() != WL_CONNECTED) {
            delay(500);
            Serial.print(".");
        }
        Serial.println("\nWi-Fi reconectado");
    }
}

void determinarZona() {
    WiFi.scanNetworks();

    float distanciaCocina = calcularDistancia(obtenerRSSI(ssidCocina));
    float distancia3Ro = calcularDistancia(obtenerRSSI(ssid3Ro));
    float distanciaSala = calcularDistancia(obtenerRSSI(ssidSala));
    float distanciaMiguel = calcularDistancia(obtenerRSSI(ssidMiguel));

    int zonaActual = asignarZona(distanciaCocina, distancia3Ro, distanciaSala, distanciaMiguel);

    Serial.print("\n\nZona:");
    Serial.println(zonaActual);

    ThingSpeak.setField(1, zonaActual);
}

bool enviarDatosThingSpeak() {
    int reintentos = 3;
    while (reintentos > 0) {
        if (ThingSpeak.writeFields(channelID, WriteAPIKey) == 200) {
            Serial.println("Datos enviados a ThingSpeak con éxito.");
            return true;
        } else {
            Serial.println("Error al enviar datos a ThingSpeak. Reintentando...");
            reintentos--;
            delay(5000);
        }
    }
    return false;
}

int asignarZona(float distCocina, float dist3Ro, float distSala, float distMiguel) {
    float menorDistancia = 9999.0;
    int zona = 6; // Cambiar el valor predeterminado a 6

    if (distCocina < menorDistancia) {
        menorDistancia = distCocina;
        zona = 1;
    }
    if (dist3Ro < menorDistancia) {
        menorDistancia = dist3Ro;
        zona = 4;
    }
    if (distSala < menorDistancia) {
        menorDistancia = distSala;
        zona = 2;
    }
    if (distMiguel < menorDistancia) {
        menorDistancia = distMiguel;
        zona = 3;
    }

    Serial.print("Zona detectada: ");
    Serial.println(zona);
    return zona;
}

float calcularDistancia(int rssi) {
    if (rssi == 0) {
        return 9999.0;
    }
    return pow(10, (PTx - rssi) / (10 * n));
}

int obtenerRSSI(const char* ssid) {
    int numRedes = WiFi.scanNetworks();
    for (int i = 0; i < numRedes; i++) {
        if (WiFi.SSID(i) == ssid) {
            return WiFi.RSSI(i);
        }
    }
    return 0;
}

bool resolverDNS() {
    IPAddress serverIP;
    int retries = 5;

    while (!WiFi.hostByName("api.thingspeak.com", serverIP) && retries > 0) {
        Serial.println("Intentando resolver DNS...");
        retries--;
        delay(1000);
    }

    if (retries == 0) {
        return false;
    } else {
        Serial.print("Dirección IP de ThingSpeak: ");
        Serial.println(serverIP);
        return true;
    }
}
