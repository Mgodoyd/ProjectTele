#include <SPI.h>
// #include <MFRC522v2.h>
#include <MFRC522.h>
#include <WiFi.h>
#include <HTTPClient.h>

// Definición de pines
#define RST_PIN 22
#define SS_PIN 21
#define MISO_PIN 19
#define MOSI_PIN 23
#define SCK_PIN 18

// Configuración de Wi-Fi
const char* ssid = "HITRON-2DE0";
const char* password = "RicardoArjona@8090";

// URL del servidor
const char* serverUrl = "http://192.168.0.11/Tele/index.php";

unsigned long previousMillis = 0;  // Almacena el último tiempo que se ejecutó
const long interval = 5000;        // Intervalo de 5 segundos
MFRC522 rfid(SS_PIN, RST_PIN);


void setup() {
  Serial.begin(115200);
  Serial.println("Conexión con el servidor");

  // Iniciar el lector RFID
  SPI.begin();
  rfid.PCD_Init();
  Serial.println("Lector RFID iniciado.");


  // Conectar a la red Wi-Fi
  WiFi.begin(ssid, password);
  Serial.print("Conectando a la red Wi-Fi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConexión exitosa!");
  Serial.print("IP Local: ");
  Serial.println(WiFi.localIP());
}

void loop() {

if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {
    String cardID = getCardID(); // Obtener el ID de la tarjeta
    int ingreso = 1;  // Aquí puedes cambiar la lógica según sea necesario

    Serial.println("Tarjeta detectada!");
    Serial.println("ID de la tarjeta: " + cardID);
    Serial.println("Ingreso: " + String(ingreso));

    sendData(cardID, ingreso); // Enviar datos al servidor

    rfid.PICC_HaltA(); // Detener la lectura de la tarjeta
  }



  unsigned long currentMillis = millis();  // Obtener el tiempo actual

  // Verificar si ha pasado el intervalo de tiempo
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;  // Guardar el tiempo actual

    // Enviar datos al servidor
    //  sendData(cardID, ingreso); // Enviar datos al servid
  }

  // Aquí puedes agregar otras tareas que necesites realizar
}
// Función para obtener el ID de la tarjeta en formato hex
String getCardID() {
  String id = "";
  for (byte i = 0; i < rfid.uid.size; i++) {
    id += String(rfid.uid.uidByte[i], HEX);
    if (i < rfid.uid.size - 1) {
      id += ""; // No agregar separador si no es el último byte
    }
  }
  return id;
}

void sendData(String cardID, int ingreso) {
    if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;

        Serial.println("Enviando datos al servidor...");

        http.begin(serverUrl);
        http.addHeader("Content-Type", "application/x-www-form-urlencoded");

        String dataToSend = "uuid=" + cardID + "&ingreso=" + ingreso;
        Serial.println(dataToSend);

        int httpCode = http.POST(dataToSend);

        if (httpCode > 0) {
            Serial.print("Código HTTP: ");
            Serial.println(httpCode);
            if (httpCode == HTTP_CODE_OK) {
                String response = http.getString();
                Serial.println("Respuesta del servidor:");
                Serial.println(response);
            }
        } else {
            Serial.print("Error al enviar POST, código: ");
            Serial.println(httpCode);
        }

        http.end();
    } else {
        Serial.println("Error en la conexión Wi-Fi");
    }
}
