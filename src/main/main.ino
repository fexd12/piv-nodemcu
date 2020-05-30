#include <HTTPClient.h>
#include <Arduino.h>
#include "parson.h"
#include <WiFi.h>
#include <SPI.h>
#include <MFRC522.h>
#include <string.h>
#include <ArduinoJson.h>

#define RST_PIN 22 // pin rfid
#define SS_PIN 21  // pin rfid

#define rele1 26
#define rele2 27

String BASE_URL = "http://35.247.255.32:3000/";

char *ssid = "Fe";
const char *password = "q1w2e3r4t5";

MFRC522 mfrc522(SS_PIN, RST_PIN); // Create MFRC522 instance.
/*
  Initialize.
*/

byte *data;
String a;
String b;
String c;
String d;

WiFiClient client;
HTTPClient http;

void dump_byte_array(byte *buffer, byte bufferSize);

void setup()
{
  Serial.begin(115200); // Initialize serial communications with the PC

  SPI.begin();        // Init SPI bus
  mfrc522.PCD_Init(); // Init MFRC522 card
  pinMode(rele1, OUTPUT);
  pinMode(rele2, OUTPUT);

  //for wifi
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.println("Wifi connected");
  Serial.println("IP adress: ");
  Serial.println(WiFi.localIP());
  Serial.println("Getway: ");
  Serial.println(WiFi.gatewayIP());

  Serial.println(F("Inserir RFID: "));
}

void loop()
{

  // Look for new cards
  if (!mfrc522.PICC_IsNewCardPresent())
  {
    delay(50);
    return;
  }
  // Select one of the cards
  if (!mfrc522.PICC_ReadCardSerial())
  {
    delay(50);
    return;
  }

  int opcao = menu();

  if (opcao == 0)
  {
    leituraDados();
  }
  else if (opcao == 1)
  {
    GravaDados();
  }
  else
  {
    Serial.println('Opcao incorreta');
    return;
  }

  // Show some details of the PICC (that is: the tag/card)
  // Serial.print(F("Card UID:"));
  // dump_byte_array(mfrc522.uid.uidByte, mfrc522.uid.size);
  // Serial.println();

  // data = mfrc522.uid.uidByte, mfrc522.uid.size;
  // for (int i = 0; i < 4; i++){
  // 	Serial.print(data[i], HEX);
  // }
  // a = String(data[0], HEX);
  // b = String(data[1], HEX);
  // c = String(data[2], HEX);
  // d = String(data[3], HEX);

  // a.toUpperCase();
  // b.toUpperCase();
  // c.toUpperCase();
  // d.toUpperCase();
  // String tag = String(a)+String(b)+String(c)+String(d);
  // //Data sending
  // httpRequest("tag",tag);
  // httpGetAgendamento("agendamento?tag=" +tag);
}

void leituraDados()
{
  Serial.print("Card UID:");
  dump_byte_array(mfrc522.uid.uidByte, mfrc522.uid.size);
  Serial.println();

  data = mfrc522.uid.uidByte, mfrc522.uid.size;
  for (int i = 0; i < 4; i++)
  {
    Serial.print(data[i], HEX);
  }
  a = String(data[0], HEX);
  b = String(data[1], HEX);
  c = String(data[2], HEX);
  d = String(data[3], HEX);

  a.toUpperCase();
  b.toUpperCase();
  c.toUpperCase();
  d.toUpperCase();

  String tag = String(a) + String(b) + String(c) + String(d);

  httpGetAgendamento("tag=" + tag);
}

void GravaDados()
{
  Serial.print("Card UID:");
  dump_byte_array(mfrc522.uid.uidByte, mfrc522.uid.size);
  Serial.println();

  data = mfrc522.uid.uidByte, mfrc522.uid.size;
  for (int i = 0; i < 4; i++)
  {
    Serial.print(data[i], HEX);
  }
  a = String(data[0], HEX);
  b = String(data[1], HEX);
  c = String(data[2], HEX);
  d = String(data[3], HEX);

  a.toUpperCase();
  b.toUpperCase();
  c.toUpperCase();
  d.toUpperCase();

  String tag = String(a) + String(b) + String(c) + String(d);
  httpRequest("tag", tag);
}

int menu()
{
  Serial.println(F("\nEscolha uma opcao:"));
  Serial.println(F("0 - leitura da Tag"));
  Serial.println(F("1 - Salva Tag no banco"));

  while (!Serial.available())
  {
  }

  int op = (int)Serial.read();
  while (Serial.available())
  {
    if (Serial.read() == '\n')
      break;
    Serial.read();
  }
  return (op - 48);
}

void httpRequest(String path, String payload)
{
  String dados = PostTag(path, payload);

  if (!dados)
  {
    return;
  }

  Serial.println("##[RESULT]## ==> " + dados);
}

String PostTag(String path, String payload)
{
  http.begin(BASE_URL + path);
  http.addHeader("content-type", "application/x-www-form-urlencoded");

  String body = "tag=" + payload;

  int httpCode = http.POST(body);

  if (httpCode < 0)
  {
    Serial.println("request error - " + httpCode);
    return "error";
  }

  // if (httpCode != HTTP_CODE_OK)
  // {
  //   return "error";
  // }

  String response = http.getString();
  http.end();

  return response;
}

void httpGetAgendamento(String tag)
{
  String dados = Get("agendamento?" + tag);
  Serial.println(dados);

  if (!dados)
  {
    return;
  }

  //Serial.println("##[RESULT]## ==> " + dados);

  const size_t capacity = JSON_OBJECT_SIZE(1) + 30;

  DynamicJsonDocument doc(capacity);

  // Parse JSON object
  deserializeJson(doc, dados);
  JsonObject obj = doc.as<JsonObject>();

  // if (error)
  // {
  //   Serial.print(F("deserializeJson() failed: "));
  //   Serial.println(error.c_str());
  //   return;
  // }

  int acende = obj["acende"];

  if (acende == 1){
    delay(500);
    Serial.println('ligando acesso 1');
    int Hora = parse(tag);
    while (Hora == 1){// acesso full

      digitalWrite(rele1, HIGH);
      digitalWrite(rele2, HIGH);
      if (Hora == 0){
        digitalWrite(rele1, LOW);
        digitalWrite(rele2, LOW);
        break;
      }
      delay(60000); //1 minutos para cada requisicao
      Hora = parse(tag);
    }
    delay(500);
  }

  else if (acende == 0)
  {
    delay(500);
    Serial.println('ligando acesso 0');
    int Hora = parse(tag);
    while (Hora == 1)
    {
      digitalWrite(rele1, HIGH);
      digitalWrite(rele2, LOW);
      if (Hora == 0){
        digitalWrite(rele1, LOW);
        digitalWrite(rele2, LOW);
        break;
      }
      delay(60000); //1 minutos
      Hora = parse(tag);
    }
    delay(500);
  }
  else
  {
    Serial.println("Tag fora do periodo de agendamento.");
  }
}

int parse(String tag){ //parse hora
  String obj = Get("hora?" + tag);
  const size_t size = JSON_OBJECT_SIZE(1) + 20;
  DynamicJsonDocument document(size);
  deserializeJson(document, obj);
  Serial.println(obj);
  JsonObject obj3 = document.as<JsonObject>();
  int liga = obj3["liga"];
  return liga;
}

String Get(String path)
{
  http.begin(BASE_URL + path);
  int httpCode = http.GET();

  if (httpCode < 0)
  {
    Serial.println("request error - " + httpCode);
    return "error";
  }

  if (httpCode != HTTP_CODE_OK)
  {
    return "";
  }

  String response = http.getString();
  http.end();

  return response;

  // client.println("GET /" + path + " HTTP/1.1");
  // client.print("Host: ");
  // client.println(BASE_URL);
  // client.println("Connection: close");
  // client.println();
  // while (client.connected()) {
  //   //Serial.println("A");
  //   String data = client.readStringUntil('\n');
  //   //Serial.println(data);
  //   if (data == "\r") {
  //     break;
  //   }
  // }
  // String data = client.readStringUntil('\n');
  // return data;

}

// Helper routine to dump a byte array as hex values to Serial
void dump_byte_array(byte *buffer, byte bufferSize)
{
  for (byte i = 0; i < bufferSize; i++)
  {
    //Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
  }
}
int compare(String a,String b){
  int index = 0;
  // Enquanto os caracteres forem iguais, corre os vectores.
  while (a[index] == b[index])
  {
    // Verifica se alguma das strings terminou.
    // Se sim, sai do loop. Caso contrario, analisa o próximo carácter.
    if (a[index] == '\0' || b[index] == '\0')
      break;
    ++index;
  }

  // Se ambos terminaram, as strings contidas são iguais.
  if (a[index] == '\0' && b[index] == '\0')
    return 0;
  else
    return -1;
}
