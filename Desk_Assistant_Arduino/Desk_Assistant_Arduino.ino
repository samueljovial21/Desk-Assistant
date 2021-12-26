//Import Library Here
#include <LiquidCrystal_I2C.h>
#include "DHT.h"
#ifdef ESP32
#include <WiFi.h>
#endif
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>

//Defining and Variables
int lcdColumns = 16; //LCD stuff
int lcdRows = 2; //LCD Stuff
LiquidCrystal_I2C lcd(0x27, lcdColumns, lcdRows); //LCD Stuff
#define red 23 //Red LED GPIO
#define green 19 //Green LED GPIO
#define buzzer 4 //Buzzer GPIO
#define Light 34 //LDR GPIO
#define trig 12 //HC-SR04
#define echo 13 //HC-SR04
#define dhtPin 15 //DHT11 GPIO 
#define type DHT11 //DHT Type
DHT dht(dhtPin, type); //Calling DHT
bool buzzerState = true; //Buzzer State
unsigned long TimeBefore = 0;
unsigned long TimeWork = 1800000;

//Insert your WiFi SSID and Password below
const char* ssid = "PARDEDE";
const char* password = "elman2020";

//Telegram Bot Details
#define BOTtoken "2146468078:AAE9JChaN4NLQcH7tegPVOx0Awdqq0K6zH8"
#define id_bot "1372210687"
WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);
int botRequestDelay = 1000;
unsigned long lastTimeBotRan;

// Bot Function
void handleNewMessages(int numNewMessages){
  Serial.println("handleNewMessages");
  Serial.println(String(numNewMessages));

  for (int i = 0; i< numNewMessages; i++){
    String chat_id = String(bot.messages[i].chat_id);

    String text = bot.messages[i].text;
    Serial.println(text);

    String from_name = bot.messages[i].from_name;

    if(text == "/help"){
      String welcome = "Hallo, " + from_name + ".\n";
      welcome += "/State for showing your Buzzer state \n";
      welcome += "/buzzer_on for turning the Buzzer Notification On \n";
      welcome += "/buzzer_off for turning the Buzzer Notification Off \n";
      bot.sendMessage(chat_id, welcome, "");
    }
    if(text == "/buzzer_on"){
      bot.sendMessage(chat_id, "Buzzer is now ON", "");
      buzzerState = true;
    }
    if(text == "/buzzer_off"){
      bot.sendMessage(chat_id, "Buzzer is now OFF", "");
      buzzerState = false;
    }
    if(text == "/state"){
      if(buzzerState){
        bot.sendMessage(chat_id, "Buzzer is ON", "");
      }
      else{
        bot.sendMessage(chat_id, "Buzzer is OFF", "");
      }
    }
  }
}

void notification(float temperature,int room,int view){

  //WorkTime Notification
  if(millis() - TimeBefore > TimeWork){
    bot.sendMessage(id_bot,"You've been working for 30 minutes, take some rest", "");
  }

  //Notification for On Buzzer
  if ((room<700)&&(buzzerState)){
    bot.sendMessage(id_bot,"You're now working on an excess of light room", "");
    digitalWrite(buzzer,HIGH);
    digitalWrite(red,HIGH);
    digitalWrite(green,LOW);
    delay(500);
    digitalWrite(buzzer,LOW);
    delay(250);
    digitalWrite(buzzer,HIGH);
    delay(500);
    digitalWrite(buzzer,LOW);
  }
  else if ((room>=700)&&(room<2600)){
    digitalWrite(buzzer,LOW);
    digitalWrite(red,LOW);
    digitalWrite(green,HIGH);
  }
  else{
    bot.sendMessage(id_bot,"You're now working on a lack of light room", "");
    if(buzzerState){
      digitalWrite(buzzer,HIGH);
      digitalWrite(red,HIGH);
      digitalWrite(green,LOW);
      delay(500);
      digitalWrite(buzzer,LOW);
      delay(250);
      digitalWrite(buzzer,HIGH);
      delay(500);
      digitalWrite(buzzer,LOW);
      }
  }

  //Notification for Off Buzzer
  if ((room<700)&&(!buzzerState)){
    bot.sendMessage(id_bot,"You're now working on an excess of light room", "");
    digitalWrite(red,HIGH);
    digitalWrite(green,LOW);
    delay(1000);
  }
  else if ((room>=700)&&(room<2600)){
    digitalWrite(red,LOW);
    digitalWrite(green,HIGH);
  }
  else{
    bot.sendMessage(id_bot,"You're now working on a lack of light room", "");
    if(!buzzerState){
      digitalWrite(red,HIGH);
      digitalWrite(green,LOW);
      delay(1000);
      }
  }

   //Notification for On Buzzer
  if ((view<45)&&(buzzerState)){
    bot.sendMessage(id_bot,"You're to close into monitor", "");
    digitalWrite(buzzer,HIGH);
    digitalWrite(red,HIGH);
    digitalWrite(green,LOW);
    delay(500);
    digitalWrite(buzzer,LOW);
    delay(250);
    digitalWrite(buzzer,HIGH);
    delay(500);
    digitalWrite(buzzer,LOW);
  }
  else{
    if(buzzerState){
      digitalWrite(buzzer,LOW);
      digitalWrite(red,LOW);
      digitalWrite(green,HIGH);
    }
  }

  //Notification for Off Buzzer
  if ((view<45)&&(!buzzerState)){
    bot.sendMessage(id_bot,"You're to close into monitor", "");
    digitalWrite(red,HIGH);
    digitalWrite(green,LOW);
    delay(1000);
  }
  else{
    if(!buzzerState){
      digitalWrite(red,LOW);
      digitalWrite(green,HIGH);
    }
  }

  if(temperature>38){
    bot.sendMessage(id_bot,"Your device has reach high temperature", "");
    bot.sendMessage(id_bot,"I think you should a little worried about it", "");
    digitalWrite(red,HIGH);
    digitalWrite(green,LOW);
    delay(1000);
  }

  else{
    digitalWrite(red,LOW);
    digitalWrite(green,HIGH);
  }
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  
  // HC-SR04
  pinMode(trig,OUTPUT);
  pinMode(echo,INPUT);

  // DHT11
  dht.begin();

  // LCD
  lcd.init();
  // turn on LCD backlight                      
  lcd.backlight();

  // LED and Buzzer
  pinMode(red,OUTPUT);
  pinMode(green,OUTPUT);
  pinMode(buzzer,OUTPUT);

  // Checking Connection into WiFi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid,password);

  #ifdef ESP32
    client.setCACert(TELEGRAM_CERTIFICATE_ROOT);
   #endif
   while(WiFi.status() != WL_CONNECTED){
    delay(1000);
    Serial.println("Connecting to WiFi..");
   }
   Serial.println(WiFi.localIP());
}

void loop() {
  // put your main code here, to run repeatedly:
  Serial.print ("Uptime (s) : ");
  Serial.println (millis()/1000);
  
  // HC-SR04
  digitalWrite(trig,LOW);
  delay(10);
  digitalWrite(trig,HIGH);
  delay(10);
  digitalWrite(trig,LOW);

  long duration;
  long Distance;

  duration = pulseIn(echo,HIGH);
  Distance = duration*0.034/2;
  Serial.print("Distance (cm) : ");
  Serial.println(Distance);

  // LDR 
  int val = analogRead(Light);
  int Sdata = val;
  Serial.print ("Light : ");
  Serial.println(Sdata);

  // DHT11
  float TempC = dht.readTemperature();

  if(isnan(isnan(TempC))){
    Serial.print(F("No DHT11 detected!"));
    return;
  }
  Serial.print(F("Temperature : "));
  Serial.print(TempC);
  Serial.println(F(" C "));

  // Telegram (Checking Messages)
  if(millis()>lastTimeBotRan + botRequestDelay){
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    while(numNewMessages){
      Serial.println("got response");
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    lastTimeBotRan = millis();
  }

  notification(TempC,Sdata,Distance);

  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Lighting");
  lcd.setCursor(0,1);
  if (Sdata<700){
    lcd.print("Excessive");
  }
  else if ((Sdata>=700)&&(Sdata<2600)){
    lcd.print("Good");
  }
  else{
    lcd.print("Bad");
  }
  delay(1000);
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Distance (cm)");
  lcd.setCursor(0,1);
  lcd.print(Distance);
  delay(1000);
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Device Temp (C)");
  lcd.setCursor(0,1);
  lcd.print(TempC);
  delay(900);
}
