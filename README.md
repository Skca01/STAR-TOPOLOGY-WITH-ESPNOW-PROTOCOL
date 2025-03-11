# Star Topology Using ESP-NOW Protocol

This project demonstrates a **star topology communication system** using the **ESP-NOW protocol**, where multiple **transmitter nodes** send data to a **single receiver (central hub)** in a **unidirectional** manner.

## 📋 Project Overview

* **Unidirectional Communication:** The system follows a **star topology** where multiple **ESP32 transmitters** send signals **only to** the central ESP32 receiver, ensuring one-way data transmission.
* **Table Nodes (Transmitters):** Each table has an ESP32 with a button. When pressed, it transmits the **table number** to the receiver.
* **Central Hub (Receiver):** The ESP32 receiver collects data from all transmitters, identifying which table sent the signal, and displays the information on an OLED screen.
* **ESP-NOW Protocol:** Enables **low-latency**, **Wi-Fi-independent** communication, making the system efficient and reliable.


## 🚀 Applications

* **Call System for Restaurants or Conferences** – Staff or organizers get notified when assistance is requested.
* **Wireless Notification System** – Can be used in workplaces or smart home setups for alerts.
* **IoT-Based Service Requests** – Expands into automation and smart solutions.

## 🛠️ Hardware Requirements

### For Each Table Node (Transmitter):
- ESP32 development board
- Push button
- Power source (battery or USB)

### For the Central Hub (Receiver):
- ESP32 development board
- 0.96" OLED Display (SSD1306, I2C)
- Power source (preferably stable USB power)

## 🔧 Setup Instructions

### Step 1: Discover the Receiver's MAC Address

Before configuring the transmitters, you need to find the MAC address of your central hub ESP32. Upload this MAC Address Finder to the receiver ESP32:

```cpp
#include <WiFi.h>

void setup() {
    Serial.begin(115200);
    WiFi.mode(WIFI_MODE_STA); // Ensure station mode is enabled
    delay(1000); // Allow time for WiFi to initialize

    Serial.print("ESP32 MAC Address: ");
    Serial.println(WiFi.macAddress());
}

void loop() {}
```

Open the Serial Monitor at 115200 baud rate and note down the MAC address displayed.

### Step 2: Configure and Upload Receiver Code

1. Open `receiver.ino` in the Arduino IDE
2. Ensure you have installed the required libraries (see Dependencies section)
3. Connect the OLED display to the ESP32 (default SDA, SCL pins)
4. Upload the code to your central hub ESP32

### Step 3: Configure and Upload Transmitter Code

1. Open `sender.ino` in the Arduino IDE
2. Update the `receiverMAC[]` array with the MAC address you noted in Step 1
3. Set a unique `TABLE_ID` for each table node (1, 2, 3, etc.)
4. Upload the code to each table node ESP32
5. Connect a button between GPIO 4 and GND on each transmitter

## 📡 How It Works

1. When a button is pressed at a table, the ESP32 transmitter sends a message containing the table ID to the central hub via ESP-NOW.
2. The central hub receives the message, identifies which table is requesting service, and displays a loading animation on the OLED screen.
3. If multiple requests are received simultaneously, they are queued and processed in order.
4. The system provides visual feedback throughout the process.

## 🧩 System Components

### Table Node (Transmitter)
- Monitors button presses
- Sends table ID via ESP-NOW in a unidirectional communication pattern
- Includes retry mechanism for reliability

### Central Hub (Receiver)
- Listens for incoming requests from any table node
- Maintains a queue for processing multiple requests
- Displays animated status on OLED screen
- Provides debugging interface via Serial

## 🔄 ESP-NOW Protocol Details

The system uses ESP-NOW, a connectionless communication protocol that allows for quick, low-power transmission of small packets of data. Benefits include:

- **Low latency** (typically < 10ms)
- **No need for Wi-Fi router** (peer-to-peer)
- **Low power consumption**
- **Simple implementation**

Message structure:
```cpp
typedef struct {
    int table_id;
} TableMessage;
```

## 🔍 Code Examples

### Transmitter (Table Node)

The transmitter code handles button input and sends messages to the central hub:

```cpp
void sendMessage() {
    TableMessage message;
    message.table_id = TABLE_ID; // Unique ID for each table
    
    esp_err_t result = esp_now_send(receiverMAC, (uint8_t *) &message, sizeof(message));
    
    if (result != ESP_OK) {
        Serial.println("Error sending message");
    }
}
```

### Receiver (Central Hub)

The receiver code processes incoming messages and displays information:

```cpp
void onDataRecv(const esp_now_recv_info_t *recvInfo, const uint8_t *incomingData, int len) {
    if (len == sizeof(TableMessage)) {
        TableMessage message;
        memcpy(&message, incomingData, sizeof(message));
        
        Serial.print("Table ID: ");
        Serial.println(message.table_id);
        
        // Add request to queue
        requestQueue.push(message.table_id);
        
        // If not currently processing a request, start processing
        if (!processingRequest) {
            processNextRequest();
        }
    }
}
```


## 📚 Dependencies

- ESP32 Board support for Arduino IDE
- [ESP-NOW](https://github.com/espressif/arduino-esp32) (included in ESP32 board package)
- [Adafruit GFX Library](https://github.com/adafruit/Adafruit-GFX-Library)
- [Adafruit SSD1306](https://github.com/adafruit/Adafruit_SSD1306)


## 📄 License

This project is licensed under the MIT License - see the LICENSE file for details.

## 🤝 Contributing

Contributions, issues, and feature requests are welcome! Feel free to check the [issues page](link-to-issues).

## 👨‍💻 Author

- Kent Carlo Amante - https://github.com/Skca01
- Contact: carloamante125@gmail.com
