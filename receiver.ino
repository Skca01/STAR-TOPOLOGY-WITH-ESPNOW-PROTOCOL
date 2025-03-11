#include <esp_now.h>
#include <WiFi.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <queue>


#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define OLED_ADDRESS 0x3C


#define ANIMATION_FRAMES 3
#define ANIMATION_SPEED 500 
#define MESSAGE_DISPLAY_TIME 2000 


typedef struct {
    int table_id;
} TableMessage;


std::queue<int> requestQueue;
bool processingRequest = false;
unsigned long lastAnimationUpdate = 0;
int currentAnimationFrame = 0;
int currentTableID = 0;


Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);


void processNextRequest() {
  if (requestQueue.empty()) {
    processingRequest = false;
    displayIdleScreen();
    return;
  }
  
  
  currentTableID = requestQueue.front();
  requestQueue.pop();
  
  
  processingRequest = true;
  currentAnimationFrame = 0;
  lastAnimationUpdate = millis();
  
  
  updateLoadingAnimation();
}


void updateLoadingAnimation() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  

  display.setCursor(0, 0);
  display.println("TABLE REQUEST RECEIVED");
  display.drawLine(0, 10, SCREEN_WIDTH-1, 10, WHITE);
  

  display.setCursor(10, 25);
  display.print("Going to Customer ");
  display.print(currentTableID);
  

  display.setCursor(10, 40);
  for (int i = 0; i < currentAnimationFrame + 1; i++) {
    display.print(".");
  }
  
 
  int progressWidth = map(currentAnimationFrame, 0, ANIMATION_FRAMES-1, 0, SCREEN_WIDTH-20);
  display.drawRect(10, 50, SCREEN_WIDTH-20, 8, WHITE);
  display.fillRect(10, 50, progressWidth, 8, WHITE);
  
  display.display();
  

  currentAnimationFrame = (currentAnimationFrame + 1) % ANIMATION_FRAMES;
  

  if (currentAnimationFrame == 0) {
 
    display.clearDisplay();
    display.setCursor(10, 25);
    display.print("Request for Table ");
    display.print(currentTableID);
    display.setCursor(10, 40);
    display.print("Processing!");
    display.display();
    
    delay(MESSAGE_DISPLAY_TIME);
    

    processNextRequest();
  }
}


void displayIdleScreen() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  
  display.setCursor(0, 0);
  display.println("TABLE REQUEST SYSTEM");
  display.drawLine(0, 10, SCREEN_WIDTH-1, 10, WHITE);
  
  display.setCursor(10, 25);
  display.println("Waiting for requests...");
  

  display.setCursor(10, 45);
  display.print("Queue: ");
  display.print(requestQueue.size());
  
  display.display();
}


void onDataRecv(const esp_now_recv_info_t *recvInfo, const uint8_t *incomingData, int len) {

  char macStr[18];
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           recvInfo->src_addr[0], recvInfo->src_addr[1], recvInfo->src_addr[2],
           recvInfo->src_addr[3], recvInfo->src_addr[4], recvInfo->src_addr[5]);
  
  Serial.print("Received from: ");
  Serial.println(macStr);
  
  if (len == sizeof(TableMessage)) {
    TableMessage message;
    memcpy(&message, incomingData, sizeof(message));
    
    Serial.print("Table ID: ");
    Serial.println(message.table_id);
    
 
    requestQueue.push(message.table_id);
    
 
    if (!processingRequest) {
      processNextRequest();
    }
  } else {
    Serial.println("Received invalid data size");
  }
}

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    ; 
  }
  
 
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS)) {
    Serial.println("OLED initialization failed!");
    while (1);
  }
  display.clearDisplay();
  
  
  WiFi.mode(WIFI_STA);
  
  
  Serial.print("Receiver MAC Address: ");
  Serial.println(WiFi.macAddress());
  
  
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    ESP.restart();
    return;
  }
  
  
  esp_now_register_recv_cb(onDataRecv);
  
  Serial.println("Receiver hub initialized and ready!");
  displayIdleScreen();
}

void loop() {
  
  if (processingRequest) {
    unsigned long currentTime = millis();
    if (currentTime - lastAnimationUpdate > ANIMATION_SPEED) {
      lastAnimationUpdate = currentTime;
      updateLoadingAnimation();
    }
  }
  

  if (Serial.available() > 0) {
    String command = Serial.readStringUntil('\n');
    command.trim();
    
    if (command.startsWith("test")) {
      int testTable = 0;
      if (command.length() > 5) {
        testTable = command.substring(5).toInt();
      } else {
        testTable = 99; 
      }
      
      Serial.println("Adding test request for Table " + String(testTable));
      requestQueue.push(testTable);
      
      if (!processingRequest) {
        processNextRequest();
      }
    } else if (command == "status") {
      Serial.println("Queue size: " + String(requestQueue.size()));
      Serial.println("Processing: " + String(processingRequest ? "Yes" : "No"));
    } else if (command == "clear") {
      
      while (!requestQueue.empty()) {
        requestQueue.pop();
      }
      processingRequest = false;
      displayIdleScreen();
      Serial.println("Queue cleared");
    }
  }
  
  
  delay(10);
}
