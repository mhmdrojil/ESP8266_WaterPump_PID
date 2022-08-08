#include <Blynk.h>
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>

//untuk blynk setup
#define BLYNK_PRINT Serial
#define BLYNK_TEMPLATE_ID "TMPLUBFBFG11"
#define BLYNK_DEVICE_NAME "Penyiraman Tanaman"
#define BLYNK_AUTH_TOKEN "T69Xowq8PZ6FvggUI1KRbCB-1pwJxvan"

//metode PID 
#define SET_POIN 60 //sesuai presentase kelembaban tanah ideal
#define P 1
#define I 0.000083
#define D 638.0067

char auth[] = "T69Xowq8PZ6FvggUI1KRbCB-1pwJxvan"; //kode otentikasi
char ssid[] = "W-KNMN"; // nama wifi
char pass[] = "pakaimasker"; // password wifi

//konfigurasi port nodeMCU
const int moisture = A0; //Moisture pada port A0
const int motorEn = 16; // port D0
const int motorIn1 = 5; // pada port D1
const int motorIn2 = 4; // pada port D2

int nilai = 0, presentase, button;
unsigned long waktuSekarang, waktuAkhir, waktuSelisih;
float errorAwal, koreksiP, koreksiI, koreksiD, errorAkhir, kecepatan;

//aktifkan port virtual blynk
BLYNK_WRITE(V0); //presentase kelembaban tanah
BLYNK_WRITE(V1); //LCD
BLYNK_WRITE(V2); //error
BLYNK_WRITE(V3); //kecepatan
BLYNK_WRITE(V4);

//konfig LCD virtual
WidgetLCD vlcd(V1);

BLYNK_CONNECTED(){
  Blynk.syncVirtual(V0); //port virtual blynk V0 presentase kelembaban tanah
  Blynk.syncVirtual(V1); //port virtual blynk V1 set point
  Blynk.syncVirtual(V2);
  Blynk.syncVirtual(V3);
  Blynk.syncVirtual(V4);
}

void setup()
{
  pinMode(moisture, INPUT); //sensor kelembaban
  pinMode(motorEn, OUTPUT); //kecepatan motor
  pinMode(motorIn1, OUTPUT); //kaki positif 
  pinMode(motorIn2, OUTPUT); //kaki negatif
  Serial.begin(9600); //untuk monitor dan diagram
  Blynk.begin(auth, ssid, pass, "blynk.cloud", 8080); //koneksi ke internet, jaringan indihome pakai port 8080
}

void loop()
{
  Blynk.run(); //syntax wajib untuk jalankan blynk setup
  
  nilai = analogRead(moisture); //baca kelembaban
  presentase = map(nilai,1024,0,0,100); //angka 700 dari keadaan sensor ketika kering
  
  /*//untuk plotter 
  Serial.print(SET_POIN); //tampilan hasil baca analog sensor
  Serial.print(" ");
  Serial.println(presentase); //tampilan presentase kelembaban tanah
  */
  Serial.print("\nSensor : "); Serial.println(nilai);
  Serial.print("Presentase : "); Serial.println(presentase);
  
  //menu blynk
  Blynk.virtualWrite(V0, presentase); //tampilan kelembaban tanah pada blynk
  
  ambilDataKondisi();
  
  //perulangan waktu dan error
  errorAkhir = errorAwal;
  waktuAkhir = waktuSekarang;
}

void ambilDataKondisi(){
 if(presentase <= SET_POIN){
   vlcd.clear();
   vlcd.print(1, 0, "Tanah Kering");
   vlcd.print(1, 1, "Pompa Aktif");
   pompaJalan(); //jalankan pompa
   pid(presentase); //jalankan kontrol PID
 }
 else{
   vlcd.clear();
   vlcd.print(1, 0, "Tanah Lembab");
   vlcd.print(1, 1, "Pompa Mati");
   pompaBerhenti();
 }
}

//perhitungan PID 
float pid(float presentase){
  waktuSekarang = millis();
  waktuSelisih = (waktuSekarang - waktuAkhir);
  Serial.print("Waktu Selisih : "); Serial.println(waktuSelisih);
  
  errorAwal = SET_POIN - presentase;
  Blynk.virtualWrite(V2, errorAwal);
  Serial.print("Error Awal : "); Serial.println(errorAwal);
  
  koreksiP = P * errorAwal;
  Serial.print("Koreksi P : "); Serial.println(koreksiP);
  koreksiI += (errorAwal-errorAkhir) * waktuSelisih;
  Serial.print("Koreksi I : "); Serial.println(koreksiI);
  koreksiD = (errorAwal-errorAkhir) / waktuSelisih;
  Serial.print("Koreksi D : "); Serial.println(koreksiD);

  float kecepatanP = koreksiP; //menghitung usaha P
  float kecepatanPI = koreksiP + I*koreksiI; //menghitung usaha PI
  kecepatan = koreksiP + I*koreksiI + D*koreksiD; // kecepatan hasil PID
  Blynk.virtualWrite(V3, kecepatan);
  Serial.print("Kecepatan(P) : "); Serial.println(kecepatanP);
  Serial.print("Kecepatan(PI) : "); Serial.println(kecepatanPI);
  Serial.print("Kecepatan(PID) : "); Serial.println(kecepatan);
  
//membatasi kecepatan pompa (opsional)
  if(kecepatan <= 255){ 
    kecepatan = -255;
    }
  if(kecepatan >= 255){
    kecepatan = 255;
    }
  
    return kecepatan;
}

void pompaJalan(){ //jalankan pompa
    digitalWrite(motorEn, kecepatan);
    digitalWrite(motorIn1, HIGH);
    digitalWrite(motorIn2, LOW);
}

void pompaBerhenti(){ //berhentikan pompa
    digitalWrite(motorEn, kecepatan);
    digitalWrite(motorIn1, LOW);
    digitalWrite(motorIn2, LOW);
}
