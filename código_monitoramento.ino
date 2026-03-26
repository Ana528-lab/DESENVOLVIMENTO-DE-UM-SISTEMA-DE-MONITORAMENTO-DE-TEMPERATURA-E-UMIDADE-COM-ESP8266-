#include <Wire.h>
#include <DHT.h>
#include <DHT_U.h>
#include <SD.h>
#include <RTClib.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h> // Biblioteca para HTTPS seguro

// ===== CONFIGURAÇÕES =====
#define SDA_PIN D2
#define SCL_PIN D1
#define DHTPIN D3       
#define DHTTYPE DHT22
#define SD_CS D8

// Wi-Fi - SUBSTITUA PELOS SEUS DADOS AO RODAR NO ARDUINO
const char* ssid = "NOME_DA_SUA_REDE_WIFI";
const char* password = "SENHA_DO_WIFI";

// URL do seu script Google Apps Script
const char* scriptURL = "https://script.google.com/macros/s/SUA_CHAVE_DO_GOOGLE_SCRIPT_AQUI/exec";

// ===== OBJETOS =====
DHT sensor(DHTPIN, DHTTYPE);
RTC_DS3231 rtc;
File myFile;

// ===== VARIÁVEIS =====
unsigned long intervalo = 300000; // 5 minutos
unsigned long ultimoRegistro = 0;
bool primeiraLeitura = true;

// ===== SETUP =====
void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("Iniciando sistema...");

  Wire.begin(SDA_PIN, SCL_PIN);

  // RTC
  if (!rtc.begin()) {
    Serial.println("Erro: RTC não encontrado!");
    while (1);
  }

  if (rtc.lostPower()) {
    Serial.println("RTC perdeu a hora. Ajustando...");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  // Cartão SD
  if (!SD.begin(SD_CS)) {
    Serial.println("Erro ao inicializar o cartão SD!");
    while (1);
  }
  Serial.println("Cartão SD OK!");

  // Sensor de temperatura e umidade
  sensor.begin();

  // Criar cabeçalho do CSV, se não existir
  if (!SD.exists("datalog.csv")) {
    myFile = SD.open("datalog.csv", FILE_WRITE);
    myFile.println("Data;Hora;Temperatura(°C);Umidade(%)");
    myFile.close();
  }

  // Conexão Wi-Fi
  Serial.print("Conectando ao Wi-Fi ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.println("Wi-Fi conectado!");
  Serial.print("Endereço IP: ");
  Serial.println(WiFi.localIP());
}

// ===== LOOP =====
void loop() {
  if (primeiraLeitura || (long)(millis() - ultimoRegistro) >= intervalo) {
    registrarTemperatura();
    primeiraLeitura = false;
  }

  // Reconectar Wi-Fi, se necessário
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Wi-Fi desconectado. Tentando reconectar...");
    WiFi.begin(ssid, password);
    unsigned long start = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - start < 10000) {
      delay(500);
      Serial.print(".");
    }
    Serial.println();
  }
}

// ===== REGISTRA TEMPERATURA =====
void registrarTemperatura() {
  float temperatura = NAN;
  float umidade = NAN;

  // Tenta ler o DHT até 3 vezes (evita falhas)
  for (int i = 0; i < 3 && (isnan(temperatura) || isnan(umidade)); i++) {
    delay(2000);
    temperatura = sensor.readTemperature();
    umidade = sensor.readHumidity();
  }

  if (isnan(temperatura) || isnan(umidade)) {
    Serial.println("Erro ao ler o sensor DHT22.");
    return;
  }

  DateTime now = rtc.now();

  // Formato americano: MÊS/DIA/ANO
  String data = String(now.month()) + "/" + String(now.day()) + "/" + String(now.year());
  String hora = String(now.hour()) + ":" + String(now.minute()) + ":" + String(now.second());

  Serial.printf("Data: %s  Hora: %s  Temp: %.2f °C  Umidade: %.2f %%\n",
                data.c_str(), hora.c_str(), temperatura, umidade);

  // Gravar no cartão SD
  myFile = SD.open("datalog.csv", FILE_WRITE);
  if (myFile) {
    myFile.printf("%s;%s;%.2f;%.2f\n", data.c_str(), hora.c_str(), temperatura, umidade);
    myFile.close();
  } else {
    Serial.println("Erro ao abrir datalog.csv");
  }

  // Enviar ao Google Sheets
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    WiFiClientSecure client;
    client.setInsecure();  // Ignora validação SSL (necessário no ESP8266)

    String url = String(scriptURL) +
                 "?data=" + urlencode(data) +
                 "&hora=" + urlencode(hora) +
                 "&temperatura=" + String(temperatura, 2) +
                 "&umidade=" + String(umidade, 2);

    Serial.print("Enviando para o Google Sheets: ");
    Serial.println(url);

    if (http.begin(client, url)) {
      int httpCode = http.GET();
      if (httpCode > 0) {
        Serial.print("Código HTTP: ");
        Serial.println(httpCode);
        String resposta = http.getString();
        Serial.print("Resposta: ");
        Serial.println(resposta);
      } else {
        Serial.print("Erro HTTP: ");
        Serial.println(http.errorToString(httpCode));
      }
      http.end();
    } else {
      Serial.println("Erro ao iniciar conexão HTTPClient");
    }
  } else {
    Serial.println("Wi-Fi desconectado. Dados salvos apenas no SD.");
  }

  ultimoRegistro = millis();
}

// ===== FUNÇÃO DE ENCODE PARA URL =====
String urlencode(String str) {
  String encodedString = "";
  char c;
  char code0, code1;
  for (int i = 0; i < str.length(); i++) {
    c = str.charAt(i);
    if (isalnum(c)) {
      encodedString += c;
    } else {
      code1 = (c & 0xf) + '0';
      if ((c & 0xf) > 9) code1 = (c & 0xf) - 10 + 'A';
      c = (c >> 4) & 0xf;
      code0 = c + '0';
      if (c > 9) code0 = c - 10 + 'A';
      encodedString += '%';
      encodedString += code0;
      encodedString += code1;
    }
  }
  return encodedString;
}
