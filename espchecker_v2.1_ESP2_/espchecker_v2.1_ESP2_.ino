#include <esp_now.h>
#include <WiFi.h>
//#include <SoftwareSerial.h>

//SoftwareSerial esp32_Serial(12,14);
#define RXD2 14
#define TXD2 12

String dataRec;
String success;

String mac, mac2;
#define broadcast_Button  27
int buttonState;
int lastButtonState = LOW;
const int broadcastLED= 13;

uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

// Structure example to receive data
// Must match the sender structure
typedef struct struct_message {
  char dt[250];
}struct_message;

// Create a struct_message called myData
struct_message myData;

// Create a structure to hold the readings from each board


// Create an array with all the structures
//struct_message boardsStruct[3] = {board1, board2, board3};

// callback function that will be executed when data is received
void OnDataRecv(const uint8_t * mac_addr, const uint8_t *incomingData, int len) {
  char macStr[18];
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  //Serial.println(macStr);
  memcpy(&myData, incomingData, sizeof(myData));
  digitalWrite(broadcastLED,HIGH);
  Serial.println(myData.dt);
  Serial2.print(myData.dt);
  delay(500);
  digitalWrite(broadcastLED,LOW);
}
// Callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
  if (status ==0){
    success = "Delivery Success :)";
  }
  else{
    success = "Delivery Fail :(";
  }
}
 
void setup() {
  //Initialize Serial Monitor
  Serial.begin(115200);
  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);
  pinMode(broadcast_Button,INPUT);
  pinMode(broadcastLED,OUTPUT);
  digitalWrite(broadcastLED,HIGH);
  //Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  //Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
   // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Trasnmitted packet
  esp_now_register_send_cb(OnDataSent);
  
  // Register peer
  esp_now_peer_info_t peerInfo;
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  
  // Add peer        
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }
  
  // Once ESPNow is successfully Init, we will register for recv CB to
  // get recv packer info
  esp_now_register_recv_cb(OnDataRecv);
}
 
void loop() 
{ 
 
  while(Serial2.available()>0)
  {
    dataRec=(Serial2.readStringUntil('\n'));
  }
  
  if(dataRec.length()>20&&dataRec!="")
  {
    dataRec.toCharArray(myData.dt, 250);
    esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));
    Serial.println(myData.dt);
    if (result == ESP_OK) {
      Serial.println("Sent with success");
    }
    else {
      Serial.println("Error sending the data");
    }
  }
  dataRec="";
  delay(10);
}
