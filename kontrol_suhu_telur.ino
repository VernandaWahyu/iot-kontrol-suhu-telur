// Library pada Arduino Framework IDE
#include <ESP8266WiFi.h>         //Menambahkan Library untuk menghubungkan ESP8266 ke jaringan WiFi
#include <DHT.h>                 //Menambahkan Library Untuk Sensor DHT11/22
#include <ThingSpeak.h>          //Menambahkan Library untuk mengirim data ke ThingSpeak 
#include <FirebaseArduino.h>     //Menambahkan Library untuk Firebase
#include <LiquidCrystal_I2C.h>   //Menambahkan Library untuk LCD

// Mendefinisikan PinSensor yang digunakan
#define DHTPIN D4 // Pin pada ESP8266 yang terhubung dengan sensor DHT
#define FIREBASE_HOST "kontrol-suhu-c5be2-default-rtdb.asia-southeast1.firebasedatabase.app"//URL yang terdapat pada Firebase
#define FIREBASE_AUTH "BsThSsukZ4Fr0foNHyzByeyuPiLVnRYmXBOxXbe7"//API KEY yang terdapat pada Firebase
DHT dht(DHTPIN, DHT11);  // Inisialisasi sensor  yang digunakan yaitu sensor DHT11
LiquidCrystal_I2C lcd(0x27, 16, 2); // Inisialisasi LCD dengan typenya
int buzzer = D5;             //Definisikan pin buzzer
const int relay = D3;        //Definisikan pin relay
int PIR_sensor = D8;         //Definisikan pin pir motion
int state = LOW;             //nilai default ketika mendeteksi sensor
int val = 0;                 //variable pada sensor
int relayON = LOW;           //relay nyala
int relayOFF = HIGH;         //relay mati


String apiKey = "6CFWXZ2FEJ63XY8R"; //Api key yang terdapat pada thingspeak
const char* ssid = "KHUSUS GAME"; //Nama jaringan WiFi yang akan di hubungkan pada ESP8266
const char* pass = "donasimas"; //Password jaringan wifi (jika tidak ada password bisa di kosongi)
const char* server = "api.thingspeak.com";  // Alamat server Thingspeak
WiFiClient client;  // Objek WiFi Client untuk koneksi ke Thingspeak

void setup() {
  Serial.begin(115200);//Serial monitor untuk menjalankan program
  dht.begin();  // Memulai sensor DHT11
  lcd.begin(16,2);//Memulai fungsi LCD dengan type 1602
  lcd.init();
  pinMode(relay, OUTPUT); //Inisialisasi relay dengan tipe ouput
  pinMode(buzzer, OUTPUT); //Inisialisasi buzzer dengan tipe ouput
  pinMode(PIR_sensor, INPUT);    //inisialisasi pir motion dengan tipe input
  digitalWrite(relay, relayOFF); //Mendefinisikan relay dengan relay mati untuk awal nilai dimasukan
  WiFi.begin(ssid, pass);  // Memulai terhubung ke jaringan WiFi
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);//Menghubungkan URL dan API key pada firebase
  while (WiFi.status() != WL_CONNECTED) {     // Proses ESP8266 terhubung ke WiFi
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("Terhubung Ke WiFi");   // Terhubung ke jaringan WiFi
  delay(4000);
}

void loop() {
  float h = dht.readHumidity();  // Membaca kelembaban udara
  float t = dht.readTemperature();  // Membaca suhu udara
  digitalWrite(relay, relayON);    //Membaca relay ketika relay menyala
  digitalWrite(relay, relayOFF);  //Membaca relay ketika relay mati
  val = digitalRead(PIR_sensor);   // read sensor value

  //Memulai display pada serial monitor
  Serial.print("Suhu Udara: "); //Menampilkan pada serial monitor suhu udara
  Serial.print(t);
  Serial.print(" C ");
  Serial.println("");
  Serial.print("Kelembaban Udara: "); //Menampilkan pada serial monitor kelembapan udara
  Serial.print(h);
  Serial.print(" % ");
  Serial.println("");
  if(t <37){ // kondisi pada suhu udara
     digitalWrite(relay, relayON);
     Serial.println("Lampu ON");
     Firebase.setFloat("Lampu ON",relayON); //input data pada firebase
  }
  else if(t >40){//kondisi pada suhu udara
     digitalWrite(relay, relayOFF);
     Serial.println("Lampu OFF");
     Firebase.setFloat("Lampu OFF",relayOFF); //input data pada firebase
  }
  val = digitalRead(PIR_sensor);   // membaca nilai sensor
  if (val == HIGH) {           // kondisi jika sensor high
    tone(buzzer, 6000, 500);  // menyalakan buzzer dengan frekuensi 6000 dan delay 500
    if (state == LOW) {       //kondisi jika sensor low
      Serial.println("Motion detected!"); //menampilkan data pada serial monitor
      state = HIGH;       // update data kembali ke high
      delay(100);                // delay 100 milliseconds 
    }
  } 
  else {
    delay(100);                // delay 100 milliseconds 
      if (state == HIGH){     //jika kondisi high kembali
        Serial.println("Motion stopped!");//menampilkan data pada serial monitor
        state = LOW;       // update variable state to LOW
    }
  }
 //Selesao display pada serial monitor
  // Nyalakan backlight
  lcd.backlight();
    // Pindahkan kursor ke kolom 0 dan baris 0
  lcd.setCursor(0, 0);
  // Cetak suhu udara ke layar
  lcd.print("Suhu Udara:");
   // Pindahkan kursor ke kolom 11 dan baris 0
  lcd.setCursor(11, 0);
  // Cetak data dari sensor layar
  lcd.print(t);
   // Pindahkan kursor ke kolom 13 dan baris 0
  lcd.setCursor(13, 0);
  // Cetak hellow ke layar
  lcd.print(".");
   // Pindahkan kursor ke kolom 0 dan baris 1
  lcd.setCursor(0, 1);
  // Cetak hellow ke layar
  lcd.print("Kelembapan: ");
   // Pindahkan kursor ke kolom 11 dan baris 1
  lcd.setCursor(11, 1);
  // Cetak hellow ke layar
  lcd.print(h);
   // Pindahkan kursor ke kolom 13 dan baris 1
  lcd.setCursor(13, 1);
  // Cetak hellow ke layar
  lcd.print(".");
  //Selesai  display pada LCD
   //Memulai display pada firebase
  Firebase.setFloat("Suhu Udara", t);
  Firebase.setFloat("Kelembapan Udara", h);
   // Tunggu beberapa saat agar data terinput ke Firebase
  Serial.println("Input Data Ke Firebase"); //
  // handle error
  if (Firebase.failed()) {
      Serial.print("setting /number failed:"); //Display pada serial monitor jika firebase salah
      Serial.println(Firebase.error());  
      return;
  }
  delay(1000);
  //Selesai  display pada Firebase dengan delay 1000 millisecond
  if (client.connect(server, 80)) { //Mengconnect kan ke server
    String postStr = apiKey; //Mengambil data API Key untuk thingspeak
    //Memulai display pada Thingspeak
    postStr += "&field1=";
    postStr += String(t);
    postStr += "&field2=";
    postStr += String(h);
    postStr += "\r\n\r\n\r\n\r\n";
    //Selesai  display pada Thingspeak

    // Mengirim data ke Thingspeak
    client.print("POST /update HTTP/1.1\n");      // Mengirim permintaan POST ke alamat URL "/update" menggunakan protokol HTTP versi 1.1.
    client.print("Host: api.thingspeak.com\n");   // Menentukan tujuan permintaan adalah "api.thingspeak.com".
    client.print("Connection: close\n");          // Menentukan bahwa koneksi akan ditutup setelah permintaan selesai.
    client.print("X-THINGSPEAKAPIKEY: " + apiKey + "\n");  // Mengirim API key sebagai header permintaan.
    client.print("Content-Type: application/x-www-form-urlencoded\n");   // Menentukan tipe konten permintaan adalah "application/x-www-form-urlencoded".
    client.print("Content-Length: ");
    client.print(postStr.length());
    client.print("\n\n");
    client.print(postStr);
    delay(2000);                   // Tunggu beberapa saat agar data terinput ke thingspeak
    Serial.println("Input Data Ke Thingspeak");
  }
  client.stop();
  delay(2000);
}
