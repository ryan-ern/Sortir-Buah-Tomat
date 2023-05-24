#include <WiFi.h>
#include <HTTPClient.h>

//-------------------------------------- Webhooks Configuration & Wifi
const char * ssid = "YOUR SSID WIFI";
const char * password = "YOUR PASSWORD WIFI";
const char* resource = "/trigger/yourspreadsheetname/with/key/yourkey_12aisndaonsudauisbhd";

const char* server = "maker.ifttt.com";

// ------------------------------Global Variable 
String ket = "";
float berat = 0;

WiFiClientSecure client;

// -----------------------------PROXIMITY IR OBSTACLE
#define ir 25

// -----------------------------COLOR SENSOR
#define s0 19
#define s1 18
#define out 15  
#define s2 2 
#define s3 4  

int red, blue, green, white;

// -----------------------------LOAD CELL
#include "HX711.h"
#define LOADCELL_DOUT_PIN 12
#define LOADCELL_SCK_PIN 13

HX711 scale;

// -----------------------------SERVO SG90
#include <ESP32Servo.h>
#define LB 33 // LEFT DOWN
#define RB 32 // RIGHT DOWN
#define LA 21 // LEFT UP
#define RA 5 // RIGHT UP

Servo rb90;
Servo lb90;
Servo la90;
Servo ra90;

// -----------------------------LED
#define ledgreen 22
#define ledred 23

void setup() {
  Serial.begin(9600);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

Serial.println("Wifi Connection Start");
Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

Serial.println("Wifi Connected");

  // -----------------------------PROXIMITY IR OBSTACLE
  pinMode(ir, INPUT);

  // -----------------------------COLOR
  pinMode(s0, OUTPUT);  
  pinMode(s1, OUTPUT);  
  pinMode(s2, OUTPUT);  
  pinMode(s3, OUTPUT);  
  pinMode(out, INPUT);  
  digitalWrite(s0, HIGH);  
  digitalWrite(s1, HIGH); 

  // -----------------------------LOAD CELL
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  scale.set_scale(710.21);    // value from calibration
  scale.tare();				         // reset loadcell to zero 

  // -----------------------------SERVO
  rb90.setPeriodHertz(50);
  lb90.setPeriodHertz(50);
  la90.setPeriodHertz(50);
  ra90.setPeriodHertz(50);

  rb90.attach(RB, 500, 2400);
  lb90.attach(LB, 500, 2400);
  la90.attach(LA, 500, 2400);
  ra90.attach(RA, 500, 2400);

  // -----------------------------LED
  pinMode(ledgreen, OUTPUT);
  pinMode(ledred, OUTPUT);

    for (int pos = 0; pos <= 170; pos += 2) {
      ra90.write(pos);
      delay(10);
      Serial.println("servo kanan atas ditutup");
    }
    delay(500);

    for (int pos = 140; pos <= 180; pos += 1) {
      rb90.write(pos);
      delay(10);
      Serial.println("servo kanan bawah ditutup");
    }
    delay(500);
    for (int pos = 0; pos <= 170; pos += 2) {
      la90.write(pos);
      delay(10);
      Serial.println("servo kiri atas ditutup");
    }
    delay(500);
    for (int pos = 180; pos >= 145; pos -= 1) {
      lb90.write(pos);
      delay(10);
      Serial.println("servo kiri bawah ditutup");
    }
}

void loop() {
  ket = "";
  berat = 0;

  // -----------------------------PROXIMITY IR OBSTACLE
  int state = digitalRead(ir);
  float weight = scale.get_units(10); //loadcell
  Serial.print("Weight: ");
  Serial.println(weight,1);
  delay(500);
  
  if (state == LOW){
    Serial.println("Objek Terdeteksi");
    delay(500);
    // -----------------------------LOAD CELL
    if (weight >= 45 && weight <=70) {
      berat = scale.get_units(10);
      Serial.println("Berat Sesuai Kriteria");
      // -----------------------------COLOR
      color();
    }
    else {
      digitalWrite(ledgreen, LOW);
      digitalWrite(ledred, LOW);
      Serial.println("Berat Tidak Sesuai Kriteria");
      for (int pos = 170; pos >= 0; pos -= 5) {
      la90.write(pos);
      delay(10);
      Serial.println("servo kiri atas dibuka");
      }
      delay(200);
      for (int pos = 0; pos <= 170; pos += 1) {
      la90.write(pos);
      delay(10);
      Serial.println("servo kiri atas ditutup");
    }
  }

    scale.power_down();			        // put the ADC in sleep mode
    delay(1000);
    scale.power_up();
  }
  else{
    digitalWrite(ledgreen, LOW);
    digitalWrite(ledred, LOW);
    Serial.println("Tidak Ada Objek");
    delay(500);
  }
}

void color()  
{    
  //red
  digitalWrite(s2, LOW);  
  digitalWrite(s3, LOW);   
  delay(50); 
  red = pulseIn(out, digitalRead(out) == HIGH ? LOW : HIGH);

  //blue
  digitalWrite(s2, LOW);
  digitalWrite(s3, HIGH); 
  delay(50);  
  blue = pulseIn(out, digitalRead(out) == HIGH ? LOW : HIGH);

  //green
  digitalWrite(s2, HIGH);
  digitalWrite(s3, HIGH);  
  delay(50);  
  green = pulseIn(out, digitalRead(out) == HIGH ? LOW : HIGH);

  //white
  digitalWrite(s2, LOW);
  digitalWrite(s3, LOW);  
  delay(50);  
  white = pulseIn(out, digitalRead(out) == HIGH ? LOW : HIGH);
  
  
  //-----------------------------------------Color Recognation
  if (red < blue && red < green){
    ket = "matang";
    Serial.println("color: Red");
    digitalWrite(ledred, HIGH);
    digitalWrite(ledgreen, LOW);

    for (int pos = 180; pos >= 140; pos -= 1) {
      rb90.write(pos);
      delay(10);
      Serial.println("servo kanan bawah dibuka");
    }

     for (int pos = 170; pos >= 0; pos -= 5) {
      ra90.write(pos);
      delay(10);
      Serial.println("servo kanan atas dibuka");
    }

  delay(5000);
    for (int pos = 0; pos <= 170; pos += 2) {
      ra90.write(pos);
      delay(10);
      Serial.println("servo kanan atas ditutup");
    }
    delay(500);

    for (int pos = 140; pos <= 180; pos += 1) {
      rb90.write(pos);
      delay(10);
      Serial.println("servo kanan bawah ditutup");
    }
    makeIFTTTRequest();
  }

  else if (green < red && green < blue){
    ket = "belum matang";
    Serial.println("color: Green");
    digitalWrite(ledgreen, HIGH);
    digitalWrite(ledred, LOW);

    for (int pos = 145; pos <= 180; pos += 1) {
      lb90.write(pos);
      delay(10);
      Serial.println("servo kiri bawah dibuka");
    }
     for (int pos = 170; pos >= 0; pos -= 2) {
      ra90.write(pos);
      delay(10);
      Serial.println("servo kanan atas dibuka");
    }

  delay(5000);
   for (int pos = 0; pos <= 170; pos += 2) {
      ra90.write(pos);
      delay(10);
      Serial.println("servo kanan atas ditutup");
    }
    delay(500);

    for (int pos = 180; pos >= 145; pos -= 1) {
      lb90.write(pos);
      delay(10);
      Serial.println("servo kiri bawah ditutup");
    }
    makeIFTTTRequest();
  }

  else if (red < 50 && green < 50 && blue < 50 && white > 10){
    ket = "busuk";
    digitalWrite(ledgreen, LOW);
    digitalWrite(ledred, LOW);
    Serial.println("color: Black");
   for (int pos = 170; pos >= 0; pos -= 2) {
      ra90.write(pos);
      delay(10);
      Serial.println("servo kanan atas dibuka");
    }
    delay(500);
   for (int pos = 0; pos <= 170; pos += 2) {
      ra90.write(pos);
      delay(10);
      Serial.println("servo kanan atas ditutup");
    }
    makeIFTTTRequest();
  }
  else if(white<10){
    ket = "tidak dikenal";
    Serial.println("color: Not Defined");
    digitalWrite(ledgreen, LOW);
    digitalWrite(ledred, LOW);
   for (int pos = 170; pos >= 0; pos -= 5) {
      la90.write(pos);
      delay(10);
      Serial.println("servo kiri atas dibuka");
    }
    delay(500);
    for (int pos = 0; pos <= 170; pos += 1) {
      la90.write(pos);
      delay(10);
      Serial.println("servo kiri atas ditutup");
    }
     makeIFTTTRequest();
  }
}
// ----------------------------------------------- VOID WEBHOOKS
void makeIFTTTRequest() {
  Serial.print("Connecting to "); 
  Serial.print(server);
  
  WiFiClient client;
  int retries = 5;
  while(!!!client.connect(server, 80) && (retries-- > 0)) {
    Serial.print(".");
  }
  Serial.println();
  if(!!!client.connected()) {
    Serial.println("Failed to connect...");
  }
  
  Serial.print("Request resource: "); 
  Serial.println(resource);

   String jsonObject = String("{\"value1\":\"") + berat + "\",\"value2\":\"" + ket + "\"}";
                      
  client.println(String("POST ") + resource + " HTTP/1.1");
  client.println(String("Host: ") + server); 
  client.println("Connection: close\r\nContent-Type: application/json");
  client.print("Content-Length: ");
  client.println(jsonObject.length());
  client.println();
  client.println(jsonObject);
        
  int timeout = 10 * 10; // 10 seconds             
  while(!!!client.available() && (timeout-- > 0)){
    delay(100);
  }
  if(!!!client.available()) {
    Serial.println("No response... Repeat Server");
    delay(100);
    makeIFTTTRequest();
  }
  while(client.available()){
    Serial.write(client.read());
  }
  
  Serial.println("\nclosing connection");
  client.stop();
}

