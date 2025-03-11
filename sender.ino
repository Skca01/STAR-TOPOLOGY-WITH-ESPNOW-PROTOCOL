#include <esp_now.h>
#include <WiFi.h>

// Replace with your receiver ESP32 MAC address
uint8_t receiverMAC[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};


#define BUTTON_PIN  4      // Connect button to GPIO 4
#define TABLE_ID    1      // Set this unique for each sender device example: 1 for table 1 and 2 for table 2 etc...
#define DEBOUNCE_TIME 200  


typedef struct {
    int table_id;
} TableMessage;


esp_now_peer_info_t peerInfo;
unsigned long lastButtonPress = 0;
bool lastSendSuccess = false;
int retryCount = 0;
const int MAX_RETRIES = 3;


void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  lastSendSuccess = (status == ESP_NOW_SEND_SUCCESS);
  if (lastSendSuccess) {
    Serial.println("Delivery success!");
    retryCount = 0;
  } else {
    Serial.println("Delivery failed!");
    if (retryCount < MAX_RETRIES) {
      retryCount++;
      Serial.print("Retrying... (");
      Serial.print(retryCount);
      Serial.println("/3)");
      sendMessage(); 
    } else {
      Serial.println("Max retries reached. Please try again later.");
      retryCount = 0;
    }
  }
}

void sendMessage() {
  TableMessage message;
  message.table_id = TABLE_ID;
  
  esp_err_t result = esp_now_send(receiverMAC, (uint8_t *) &message, sizeof(message));
  
  if (result != ESP_OK) {
    Serial.println("Error initiating message send");
  }
}

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    ; 
  }
  
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  
  WiFi.mode(WIFI_STA);
  
  
  Serial.print("Sender MAC Address: ");
  Serial.println(WiFi.macAddress());
  
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    ESP.restart(); 
    return;
  }
  
  esp_now_register_send_cb(OnDataSent);
  
  memcpy(peerInfo.peer_addr, receiverMAC, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
    ESP.restart(); 
    return;
  }
  
  Serial.println("Sender for Table " + String(TABLE_ID) + " initialized and ready!");
}

void loop() {

  if (digitalRead(BUTTON_PIN) == LOW) {
    unsigned long currentTime = millis();
    
    if (currentTime - lastButtonPress > DEBOUNCE_TIME) {
      lastButtonPress = currentTime;
      
      Serial.println("Button Pressed! Sending request for Table " + String(TABLE_ID));
      retryCount = 0;
      sendMessage();
    }
  }
  
  delay(10);
}
