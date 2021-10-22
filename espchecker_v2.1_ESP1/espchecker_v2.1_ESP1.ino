//Caution!! -- This sketch/Project won't work on default ESP32 SPI Lib!!!
//Open SPI.h file for ESP32, replace line on
// default SPIClass - "void begin(int8_t sck=-1, int8_t miso=-1, int8_t mosi=-1, int8_t ss=-1);"
//with - "void begin(int8_t sck=18, int8_t miso=19, int8_t mosi=15, int8_t ss=5);"

#include "BluetoothSerial.h"
#include "FS.h"
#include "SD.h"
#include "SPI.h"

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

String filename = "/File_httpData";
String LastNum = "/File_LastNum.txt";
String LastSent = "/File_LastSent.txt";

//SoftwareSerial esp32_Serial(12,14);
#define RXD2 14
#define TXD2 12
#define LEDPin            13
#define BT_LED            32
String macRec, ESP1;
String BT_String, RxdChar;

BluetoothSerial SerialBT;
// Bt_Status callback function
void Bt_Status (esp_spp_cb_event_t event, esp_spp_cb_param_t *param) {

  if (event == ESP_SPP_SRV_OPEN_EVT) {
    Serial.println ("Client Connected");
    digitalWrite (BT_LED, HIGH);
    // Do stuff if connected
  }

  else if (event == ESP_SPP_CLOSE_EVT ) {
    Serial.println ("Client Disconnected");
    digitalWrite (BT_LED, LOW);
    // Do stuff if not connected
  }
}


void setup()
{
  Serial.begin(115200);
  
  if(!SD.begin(5)){
    Serial.println("Card Mount Failed");
    return;
  }
  uint8_t cardType = SD.cardType();

  if(cardType == CARD_NONE){
    Serial.println("No SD card attached");
    return;
  }
 // reset_SD_Card();
  
  Serial2.begin(9600,SERIAL_8N1, RXD2, TXD2);
  pinMode(LEDPin, OUTPUT);
  pinMode(BT_LED, OUTPUT);
  digitalWrite (BT_LED, LOW);

  SerialBT.begin("SM_Checker 2"); //Bluetooth device name
  SerialBT.register_callback (Bt_Status);

  if(!SD.exists(LastSent)&&!SD.exists(LastNum)){
    writeFile(SD, LastSent, "0");
    writeFile(SD, LastNum, "0");
    Serial.println("LastNum and LastSent written");
  }
  else{
    String lastsent = readFile(SD, LastSent);
    String lastnum = readFile(SD, LastNum);
    Serial.println("LastNum:" + lastnum);
    Serial.println("LastSent:" + lastsent);
  }
 
}


void loop()
{
  while(Serial2.available()>0)
  {
    macRec=Serial2.readStringUntil('\n');
    //Serial.println(macRec);
  }  
  while(SerialBT.available()>0) {
    RxdChar = (char)SerialBT.read();
    BT_String+=RxdChar;
  }
  if(macRec.length()<20&&macRec!=""){
    SerialBT.print(macRec);      
  }
  
  if(BT_String.length()>20)
  {
    Serial.println(macRec);
    ESP1= macRec + "&" + BT_String;
    digitalWrite(LEDPin, HIGH);
    Serial2.print(ESP1);
    Serial.println(ESP1);
    //saveToSD();
  }
  
 
  BT_String="";
  ESP1="";
  vTaskDelay(20/portTICK_PERIOD_MS);
}

void saveToSD()
{
  String lastnum = readFile(SD, LastNum);
  int lastnum_int = lastnum.toInt();
  
  String http_txt= filename+(String)lastnum_int+".txt";
  Serial.println(http_txt);
  bool written1=false;
  bool written2=false;
  while(!written1){ //retry if failed to write
    written1=writeFile(SD, http_txt, ESP1); 
    Serial.println("W1");
  }
  lastnum_int++; 
  Serial.println(lastnum_int); 
  while(!written2){
    written2=writeFile(SD, LastNum, (String)lastnum_int);
    Serial.println("W2");
  }
  Serial.println("savesd");
}

void deleteFile(fs::FS &fs, String path){
    Serial.println("Deleting file: "+ path);
    vTaskDelay(10/portTICK_PERIOD_MS);
    if(fs.remove(path)){
        Serial.println("File deleted");
    } else {
        Serial.println("Delete failed");
    }
}

String readFile(fs::FS &fs, String path){
    //Serial.println("Reading file: "+ path);
    String read_String="";
    File file = fs.open(path);
    vTaskDelay(100/portTICK_PERIOD_MS);
    if(!file){
        Serial.println();
        Serial.println("Failed to open "+ path +" for reading");
        digitalWrite(LEDPin, HIGH);
        vTaskDelay(50/portTICK_PERIOD_MS);
        digitalWrite(LEDPin, LOW);
        return read_String;
    }

    //Serial.print("Read from file: ");
    while(file.available()){
       //Serial.print((char)file.read());
       read_String+=(char)file.read();
    }
    file.close();
    return read_String;
}

bool writeFile(fs::FS &fs, String path, String message){
    
    Serial.println("Writing file: "+ path);
    vTaskDelay(10/portTICK_PERIOD_MS);
    
    File file = fs.open(path, FILE_WRITE);
    vTaskDelay(10/portTICK_PERIOD_MS);
    
    if(!file){
        Serial.println("Failed to open file for writing");
        return false;
    }
    if(file.print(message)){
        Serial.println("File written");
        file.close();
        return true;
    } else {
        Serial.println("Write failed");
        file.close();
        return false;
    }
}
void reset_SD_Card(){
  /*
  String lastsent=readFile(SD, LastSent);
  String lastnum=readFile(SD, LastNum);
  int lastsent_int = lastsent.toInt();
  int lastnum_int = lastnum.toInt();
  if((lastsent_int>lastnum_int)){
          lastnum="0";
          lastsent="0";
          bool written1=false;
          bool written2=false;
          while(!written1) {         
            written1 = writeFile(SD, LastSent, lastsent);
          }
          while(!written2) {
            written2 = writeFile(SD, LastNum, lastnum);
          }
  }*/
  for(int i=0; i<10; i++)
    { 
      String http_txt= filename+String(i)+".txt";
      String data_file = readFile(SD, http_txt);
      if(data_file!="")
      {
        Serial.println(data_file);
        deleteFile(SD, http_txt);
      }
      //else
        //Serial.println("Empty");
    }
    deleteFile(SD, LastNum);
    deleteFile(SD, LastSent);
    
}

String getValue(String data, char separator, int index)
{
    int found = 0;
    int strIndex[] = { 0, -1 };
    int maxIndex = data.length() - 1;

    for (int i = 0; i <= maxIndex && found <= index; i++) {
        if (data.charAt(i) == separator || i == maxIndex) {
            found++;
            strIndex[0] = strIndex[1] + 1;
            strIndex[1] = (i == maxIndex) ? i+1 : i;
        }
    }
    return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}
