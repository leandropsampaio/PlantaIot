//Include da lib de Wifi do ESP8266
#include <ESP8266WiFi.h>
#include <OneWire.h>
#include <DallasTemperature.h>

//Definir o SSID da rede WiFi
const char* ssid = "************"; // Colocar o nome da rede WiFi
//Definir a senha da rede WiFi
const char* password = "***********"; // Colocar a senha da rede WiFi

long previousMillis = 0;        // Variável de controle do tempo
long redLedInterval = 15000;     // Tempo em ms do intervalo a ser executado

//Colocar a API Key para escrita neste campo
//Ela é fornecida no canal que foi criado na aba API Keys
String apiKey = "*************"; // Colocar a apiKey
const char* server = "api.thingspeak.com";

int portaLDR = A0; //Porta analógica utilizada pelo LDR

/********************************************************************/
// Data wire is plugged into pin 2 on the Arduino
#define ONE_WIRE_BUS 2
/********************************************************************/
// Setup a oneWire instance to communicate with any OneWire devices
// (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);
/********************************************************************/
// Pass our oneWire reference to Dallas Temperature.
DallasTemperature sensors(&oneWire);

WiFiClient client;

void setup() {
  //Configuração da UART
  Serial.begin(9600);

  // Inicia o sensor de temperatura
  sensors.begin();

  //Inicia o WiFi
  WiFi.begin(ssid, password);

  //Espera a conexão no router
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  //Logs na porta serial
  Serial.println("");
  Serial.print("Conectado na rede ");
  Serial.println(ssid);
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
}

void loop() {

  unsigned long currentMillis = millis();    //Tempo atual em ms

  if (currentMillis - previousMillis > redLedInterval) {
    previousMillis = currentMillis;    // Salva o tempo atual

    sensors.requestTemperatures();

    //Leitura de umidade
    float umidade = 0;
    //Leitura de temperatura
    float temperatura = sensors.getTempCByIndex(0);
    //Leitura da luminosidade
    float luminosidade = analogRead(portaLDR);

    //Se não for um numero retorna erro de leitura
    if (isnan(umidade) || isnan(temperatura)) {
      Serial.println("Erro ao ler o sensor!");
      return;
    }

    //Inicia um client TCP para o envio dos dados
    if (client.connect(server, 80)) {
      String postStr = apiKey;
      postStr += "&amp;field1=";
      postStr += String(temperatura);
      postStr += "&amp;field2=";
      postStr += String(umidade);
      postStr += "&amp;field3=";
      postStr += String(luminosidade);
      postStr += "\r\n\r\n";

      client.print("POST /update HTTP/1.1\n");
      client.print("Host: api.thingspeak.com\n");
      client.print("Connection: close\n");
      client.print("X-THINGSPEAKAPIKEY: " + apiKey + "\n");
      client.print("Content-Type: application/x-www-form-urlencoded\n");
      client.print("Content-Length: ");
      client.print(postStr.length());
      client.print("\n\n");
      client.print(postStr);

      //Logs na porta serial
      Serial.print(" Temperatura: ");
      Serial.print(temperatura);
      Serial.print(" Umidade: ");
      Serial.println(umidade);
      Serial.print(" Luminosidade: ");
      Serial.println(luminosidade);
    }

    client.stop();
  }
}
