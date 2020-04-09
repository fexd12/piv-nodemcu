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
// typedef struct
// {
// 	int id,
// 		int sala_id, "users_tags_id" : 1, "usuario" : "testees32", "data" : "2020-04-08T03:00:00.000Z", "horario_inicial" : "2020-04-06T22:02:00.000Z", "horario_final" : "2020-04-06T22:08:00.000Z", "tag" : "F36EF27", "acesso" : 1, "sala" : "teste"
// };

void dump_byte_array(byte *buffer, byte bufferSize);

void setup()
{
	Serial.begin(115200); // Initialize serial communications with the PC

	SPI.begin();		// Init SPI bus
	mfrc522.PCD_Init(); // Init MFRC522 card

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

	Serial.println(F("RFID Read: "));
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

	httpGetAgendamento("agendamento?tag=" + tag);
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

	if (httpCode != HTTP_CODE_OK)
	{
		return "";
	}

	String response = http.getString();
	http.end();

	return response;
}

void httpGetAgendamento(String path)
{
	String dados = GetAgendamento(path);

	if (!dados)
	{
		return;
	}

	Serial.println("##[RESULT]## ==> " + dados);

	const size_t capacity = JSON_OBJECT_SIZE(10) + JSON_ARRAY_SIZE(2) + 60;

	DynamicJsonDocument doc(capacity);

	// Parse JSON object
	DeserializationError error = deserializeJson(doc, dados);

	if (error)
	{
		Serial.print(F("deserializeJson() failed: "));
		Serial.println(error.c_str());
		return;
	}

	// Extract values
	Serial.println(F("Response:"));
	Serial.println(doc["sensor"].as<char *>());
	Serial.println(doc["time"].as<long>());
	Serial.println(doc["data"][0].as<float>(), 6);
	Serial.println(doc["data"][1].as<float>(), 6);
}

String GetAgendamento(String path)
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
