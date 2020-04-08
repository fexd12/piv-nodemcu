#include <HTTPClient.h>
#include <Arduino.h>
#include "parson.h"
#include <WiFi.h>
#include <SPI.h>
#include <MFRC522.h>
#include <string.h>

#define RST_PIN 22 // pin rfid
#define SS_PIN 21 // pin rfid

String BASE_URL = "http://192.168.0.28:3000/";

char *ssid = "Fe";
const char *password = "q1w2e3r4t5";

MFRC522 mfrc522(SS_PIN, RST_PIN); // Create MFRC522 instance.
								  /*
 * Initialize.
 */

byte *data;
String a;
String b;
String c;
String d;

WiFiClient client;
HTTPClient http;

void dump_byte_array(byte *buffer, byte bufferSize);

void httpRequest(String path, String tag);
String PostTag(String path,String tag);
void httpGetAgendamento(String path);
String GetAgendamento(String path);

void setup(){
	Serial.begin(115200); // Initialize serial communications with the PC

	SPI.begin();		// Init SPI bus
	mfrc522.PCD_Init(); // Init MFRC522 card
  
	//for wifi
	Serial.print("Connecting to ");
	Serial.println(ssid);

	WiFi.begin(ssid, password);
	while (WiFi.status() != WL_CONNECTED){
		delay(500);
		Serial.print(".");
	}
	Serial.println();
	Serial.println("Wifi connected");
	Serial.println("IP adress: ");
	Serial.println(WiFi.localIP());
	Serial.println("Getway: ");
	Serial.println(WiFi.gatewayIP());
  
	Serial.println(F("RFID Read: "));
}

void loop(){
	//Connection to server

	// Look for new cards
	if (!mfrc522.PICC_IsNewCardPresent()){
		delay(50);
		return;
	}
	// Select one of the cards
	if (!mfrc522.PICC_ReadCardSerial()){
		delay(50);
		return;
	}
	// Show some details of the PICC (that is: the tag/card)
	Serial.print(F("Card UID:"));
	dump_byte_array(mfrc522.uid.uidByte, mfrc522.uid.size);
	Serial.println();

	data = mfrc522.uid.uidByte, mfrc522.uid.size;
	for (int i = 0; i < 4; i++){
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
	String tag = String(a)+String(b)+String(c)+String(d);
	//Data sending
	httpRequest("tag",tag);
	httpGetAgendamento("agendamento?tag=" +tag);
}

void httpRequest(String path, String payload){
  String dados = PostTag(path,payload);

  if (!dados) {
    return;
  }

  Serial.println("##[RESULT]## ==> " + dados);

}

String PostTag(String path,String payload){
  http.begin(BASE_URL + path);
  http.addHeader("content-type", "application/x-www-form-urlencoded");

  String body = "tag="+payload;

  int httpCode = http.POST(body);

  if (httpCode < 0) {
    Serial.println("request error - " + httpCode);
    return "error";

  }

  if (httpCode != HTTP_CODE_OK) {
    return "";
  }

  String response =  http.getString();
  http.end();

  return response;
}

void httpGetAgendamento(String path){
  String dados = GetAgendamento(path);

  if (!dados) {
    return;
  }

  Serial.println("##[RESULT]## ==> " + dados);

}

String GetAgendamento(String path){
	http.begin(BASE_URL + path);
	int httpCode = http.GET();

	if (httpCode < 0) {
    Serial.println("request error - " + httpCode);
    return "error";

  }

  if (httpCode != HTTP_CODE_OK) {
    return "";
  }

  String response =  http.getString();
  http.end();

  return response;
}

// Helper routine to dump a byte array as hex values to Serial
void dump_byte_array(byte *buffer, byte bufferSize){
	for (byte i = 0; i < bufferSize; i++){
		//Serial.print(buffer[i] < 0x10 ? " 0" : " ");
		Serial.print(buffer[i], HEX);
	}
}
