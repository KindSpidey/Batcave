#include <ESP8266WiFi.h>
#include <PubSubClient.h>
//#include "pitches.h"
#include <Thread.h>
 
Thread soundThread = Thread();
Thread ledThread = Thread();
 
int PIR = 5;
int redLed = 16;
int bluLed = 14;
int grnLed = 12;
int soundPin = 13;
double timer;
bool email = false;
bool phone = false;
bool led1 = false;
bool pir1 = false;
bool sound = false;
bool sms = false;
bool stopalarm = 1;
byte sa = 1;
const char *ssid = "xxxxxxxxx"; // Имя роутера
const char *pass = "xxxxxxxxx"; // Пароль роутера
 
const char *mqtt_server = "xxxxxxxxx"; // Имя сервера MQTT
const int mqtt_port = 1883; // Порт для подключения к серверу MQTT
const char *mqtt_user = "xxxxxxxxx"; // Логи для подключения к серверу MQTT
const char *mqtt_pass = "xxxxxxxxx"; // Пароль для подключения к серверу MQTT
const char* mqtt_topics[] = {"devices/1/phone/", "devices/1/email/",
                             "devices/1/led1/", "devices/1/sound/",
                           "devices/1/pir1/", "devices/1/timetokill/phone/",
                      "devices/1/timetokill/email/", "devices/1/stopalarm/",
                "devices/1/timetokill/sms/", "devices/1/situation/"};
 
WiFiClient wclient;
 
void setup()
{
  pinMode(soundPin, OUTPUT);
  pinMode(PIR, INPUT);
  pinMode(redLed, OUTPUT);
  pinMode(grnLed, OUTPUT);
  pinMode(bluLed, OUTPUT);
  Serial.begin(9600);
  pir1 = 1;
  led1 = 1;
  sound = 1;
  timer = millis();
  soundThread.onRun(playmelody);     // назначаем потоку задачу
  soundThread.setInterval(20);
  ledThread.onRun(ledBlink);  // назначаем потоку задачу
  ledThread.setInterval(1000);
}
 
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.println("]");
   if(strcmp(topic, "devices/1/phone/")==0){
   phone = int(payload);}
   if(strcmp(topic, "devices/1/email/")==0){
   email = int(payload);}
   if(strcmp(topic, "devices/1/led1/")==0){
     led1 = int(payload);}
   if(strcmp(topic, "devices/1/sound/")==0){
   sound = int(payload);}
   if(strcmp(topic, "devices/1/pir1/")==0){
    pir1 = int(payload);}
    if(strcmp(topic, "devices/1/stopalarm/")==0){
     stopalarm = int(payload);}
  Serial.println();
}
 
PubSubClient client(mqtt_server, mqtt_port, callback, wclient);
 
void subscribeToAllTopics(){
  for(int i=0;i<(sizeof(mqtt_topics)/sizeof(*mqtt_topics));i++)
    client.subscribe(mqtt_topics[i]);
}
 
void connectToWiFi(){
  if (WiFi.status() != WL_CONNECTED) {
    Serial.print("Connecting to ");
    Serial.print(ssid);
    Serial.println("...");
    WiFi.begin(ssid, pass);
    if (WiFi.waitForConnectResult() != WL_CONNECTED) return;
    Serial.println("WiFi connected");
  }
}
 
void connectToMQTT(){
  String clientId = "ESP8266Client-";
  clientId += String(random(0xffff), HEX);
  if (client.connect(clientId.c_str(), mqtt_user, mqtt_pass)) {
    Serial.println("Connected to MQTT server ");
    client.setServer(mqtt_server, mqtt_port);
    client.setCallback(callback);
    subscribeToAllTopics();
   }
  else
    Serial.println("Could not connect to MQTT server");
}
 
void playmelody()
{
    static int ton = 100;  // тональность звука, Гц
    tone(soundPin, ton);  // включаем сирену на "ton" Гц
    if (ton <= 500) {  // до частоты 500 Гц
        ton += 100;  // увеличиваем тональность сирены
    }
    else {  // по достижении 500 Гц
        ton = 100;  // сбрасываем тональность до 100 Гц
    }
    Serial.println("SOUND WORKS!")
}
 
void ledBlink() {
    static bool ledStatus = false;    // состояние светодиода Вкл/Выкл
    ledStatus = !ledStatus;           // инвертируем состояние
    digitalWrite(redLed, ledStatus);  // включаем/выключаем светодиод
    Serial.println("LED WORKS")
}

void working()
{
  digitalWrite(bluLed, 1);
  if (digitalRead(PIR)==1)
  {
    if (led1)
    {digitalWrite(bluLed, 0);
     ledThread.run();
    }
    if (sound){soundThread.run();}
    client.publish("devices/1/situation", "Main Enter detected!");
    double currentTime = millis();
    while(millis()-currentTime<10)
    {
      if (stopalarm)
      {
        stopalarm = false;
        break;
      }      
    }
    if (email){client.publish("devices/1/timetokill/email/", "Проникновение в Бэт-Пещеру из главного входа");
              Serial.println("devices/1/timetokill/email/, Проникновение в Бэт-Пещеру из главного входа");}
    if (sms){client.publish("devices/1/timetokill/sms/", "Проникновение в Бэт-Пещеру из главного входа");
            Serial.println("devices/1/timetokill/sms/, Проникновение в Бэт-Пещеру из главного входа");}
    if (phone) {client.publish("devices/1/timetokill/phone/","Проникновение в Бэт-Пещеру из главного входа");
               Serial.println("devices/1/timetokill/phone/, Проникновение в Бэт-Пещеру из главного входа");}
  }
 }
void loop()
{
  connectToWiFi();
  if (WiFi.status() == WL_CONNECTED) {
      if (!client.connected())
      {
        Serial.print("Connecting to MQTT server ");
        Serial.print(mqtt_server);
        Serial.println("...");
        connectToMQTT();
      }
    if (client.connected()){
  client.loop();}
  if (pir1){working();}  
}
}
