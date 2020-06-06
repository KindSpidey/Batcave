#include <ESP8266WiFi.h>
#include <PubSubClient.h>
//#include "pitches.h"
#include <Thread.h>
 
Thread soundThread = Thread();
Thread ledThread = Thread();
Thread pirThread = Thread();
Thread checkSecThread = Thread();
 
 
int PIR = 16;
int redLed = 15;
int bluLed = 14;
int grnLed = 12;
int soundPin = 13;
double timer;
bool email  ;
bool phone  ;
bool led1 ;
bool pir1  ;
bool sound ;
bool sms ;
bool stopalarm = false;
bool blue;
bool alarm;
bool notpublish = true;
const char *ssid = "WiFi-DOM.ru-0494"; // Имя роутера
const char *pass = "xkfJDmMN28"; // Пароль роутера
 
const char *mqtt_server = "tailor.cloudmqtt.com"; // Имя сервера MQTT
const int mqtt_port = 12878; // Порт для подключения к серверу MQTT
const char *mqtt_user = "pyhyvrkj"; // Логи для подключения к серверу MQTT
const char *mqtt_pass = "yc_uB67FT97v"; // Пароль для подключения к серверу MQTT
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
  Serial.begin(115200);
  soundThread.onRun(playmelody);     // назначаем потоку задачу
  soundThread.setInterval(200);
  ledThread.onRun(ledBlink);  // назначаем потоку задачу
  ledThread.setInterval(200);
  pirThread.onRun(working);
  pirThread.setInterval(500);
  checkSecThread.onRun(checkSec);
  checkSecThread.setInterval(500);
}
 
int convertFromPayload(byte* payload, unsigned int length){
  char buf[length];
  for(int i=0;i<length;i++)
    buf[i] = (char) payload[i];
  String value = String(buf);
  return value.toInt();
}
 
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.println("]");
  int value = convertFromPayload(payload, length);
  Serial.println(value);
   if(strcmp(topic, "devices/1/phone/")==0){
   Serial.println(String(topic) + String(value));
   phone = value;}
   if(strcmp(topic, "devices/1/email/")==0){
   email = value;}
   if(strcmp(topic, "devices/1/sms/")==0){
   sms = value;}
   if(strcmp(topic, "devices/1/led1/")==0){
   led1 = value; Serial.print("Led state: "); Serial.println(led1);}
   if(strcmp(topic, "devices/1/sound/")==0){
   sound = value;}
   if(strcmp(topic, "devices/1/pir1/")==0){
    pir1 = value;}
    if(strcmp(topic, "devices/1/stopalarm/")==0){
    stopalarm = value;}
}
 
PubSubClient client(mqtt_server, mqtt_port, callback, wclient);
 
void subscribeToAllTopics(){
  for(int i=0;i<(sizeof(mqtt_topics)/sizeof(*mqtt_topics));i++)
    client.subscribe(mqtt_topics[i]);
    client.subscribe("1234");
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
  if(sound && !stopalarm)
    digitalWrite(soundPin, 1);
//    Serial.println("SOUND WORKS!");
}
 
void ledBlink()
{
  static unsigned long myLedTimer = millis();
  if(millis()-myLedTimer>300){
    myLedTimer = millis();
    digitalWrite(bluLed, 0);
    digitalWrite(redLed, 1-digitalRead(redLed));
  }  
}
void checkSec()
{
  Serial.print("Time:");
  Serial.println(millis()-timer);
  if (millis()-timer<3000 && stopalarm){
    Serial.println("Turned off");
    alarm=false;
    stopalarm = false;
    timer = millis();
    digitalWrite(redLed, 0);
    digitalWrite(soundPin, 0);
  }
  else if (millis()-timer>3000 && !stopalarm)
  {
      if (email){client.publish("devices/1/timetokill/email/", "Проникновение в Бэт-Пещеру из главного входа");
                Serial.println("devices/1/timetokill/email/, Проникновение в Бэт-Пещеру из главного входа");}
      if (sms){client.publish("devices/1/timetokill/sms/", "Проникновение в Бэт-Пещеру из главного входа");
                Serial.println("devices/1/timetokill/sms/, Проникновение в Бэт-Пещеру из главного входа");}
      if (phone) {client.publish("devices/1/timetokill/phone/","Проникновение в Бэт-Пещеру из главного входа");
                Serial.println("devices/1/timetokill/phone/, Проникновение в Бэт-Пещеру из главного входа");}
      Serial.println("Timed off");
      alarm=false;
      stopalarm = true;
      timer = millis();
      digitalWrite(redLed, 0);
      digitalWrite(soundPin, 0);
  }
}
void working()
{
  if (led1 && blue)
  {
    digitalWrite(grnLed, 0);
    digitalWrite(bluLed, 1);
  }
  if(!digitalRead(PIR)){
    timer = millis();
  }
  if (!digitalRead(PIR) || alarm)
  {  
    alarm = true;
      if (led1)
      {
        digitalWrite(bluLed, 0);
        ledThread.run();
      }
      if (sound && !stopalarm)
        soundThread.run();
      if (notpublish)
        client.publish("devices/1/situation/", "Main Enter detected!");
      checkSecThread.run();
    notpublish = false;
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
      client.loop();
      if (led1 && !pir1){
        digitalWrite(grnLed,1);
        digitalWrite(bluLed,0);
      }
      if (pir1){
        pirThread.run();
        blue = true;
      }
  }
}
}
