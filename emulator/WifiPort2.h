
#ifdef ARDUINO_UNOR4_WIFI
#include <WiFi.h>     //defaul wifi library for uno R4
#include <WiFiUdp.h>  //https://docs.arduino.cc/language-reference/en/functions/wifi/udp/ --> use to send and received udp messages over wifi
#include <String>
#endif



enum WifiPortType { Transmitter, Receiver, Emulator };

template< typename DataPacket>
class WifiPort {
protected:

  #ifdef ARDUINO_UNOR4_WIFI
    WiFiUDP udp;
  #endif

  WifiPortType dir;

  const unsigned long packetTimeout = 2000;      // 2 seconds
  const unsigned long wifiCheckInterval = 2000;  // check every 2 seconds

  // The receiver’s SoftAP usually defaults to 192.168.4.1
  const char* serverIP = "192.168.4.1";
  const unsigned int rx_local_port = 8888;  // UDP port to listen on --> port for Rx
  const unsigned int tx_local_port = 8889;  // Arbitrary local port for TX


  // Variables for connection check
  unsigned long lastWiFiCheck = 0;
  unsigned long lastPacketTime = 0;

  bool has_new_data = false;

  String port_name;
  String password;


  DataPacket data_packet;

  void connectWiFi() {
  #ifdef ARDUINO_UNOR4_WIFI
      Serial.print("TX: Connecting to AP ");
      Serial.println(port_name);

      WiFi.begin(port_name.c_str(), password.c_str());
      unsigned long startAttemptTime = millis();
      while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 15000) {
        //attempts to connect to wifi for 15s before restarting connection attempt if failed | if succesful terminates immediatley
        delay(500);
        Serial.print(".");
      }
      if (WiFi.status() == WL_CONNECTED) {
        udp.begin(tx_local_port);  //initializes Tx port
        Serial.println("\nTX: Connected!");
        Serial.print("TX: IP Address: ");
        Serial.println(WiFi.localIP());
      } else {
        Serial.println("\nTX: Failed to connect!");
      }
  #endif
  }

public:
  WifiPort() = default;
  ~WifiPort() = default;

  WifiPortType getPortType() {
    return dir;
  }

  void begin(String port_name, String password, WifiPortType dir) {
    pinMode(13, OUTPUT);  // built-in LED --> visual of connection status between Tx and Rx
    if (password.length() < 8) {
      while (true) {
        Serial.println("Password: <" + password + "> is too short, please increase to 8 characters minimum.");
        delay(1000);
      }
    }

    this->port_name = port_name;
    this->password = password;
    this->dir = dir;

    if (dir == WifiPortType::Emulator) {
      has_new_data = false;
      Serial.println("Local TxRx Emulation started sucessfully.");
      return void();
    }

  #ifdef ARDUINO_UNOR4_WIFI
      if (dir == WifiPortType::Receiver) {
        Serial.println("RX: Starting up...");

        // Start the board as a Soft Access Point (SoftAP), i.e., create its own hot spot
        //this allows the device, specifically the Rx, to act as a temporary wirelss network that other devices can connect to
        if (WiFi.beginAP(port_name.c_str(), password.c_str())) {
          //if the ssid and the password exist for our soft access point
          Serial.println("RX: SoftAP started successfully.");
          Serial.print("RX: AP IP: ");
          Serial.println(WiFi.softAPIP());

          //display AP information
        } else {
          while (true) {
            Serial.println("RX: Failed to start AP!");
            delay(1000);
          }
        }
        udp.begin(rx_local_port);  //initalizes Rx port
        //Initializes the WiFi UDP library and network settings. Starts WiFiUDP socket, listening at localPort
        lastPacketTime = millis();
      } else {
        //DONT USE PIN13 FOR ANY SENSOR OR ACTUATORS --> visual connection
        Serial.println("TX: Starting up...");
        connectWiFi();
      }
  #else
      while (true) {
        Serial.println("Not Ardunio R4 Wifi Board type, Either Select the correct board in the IDE or put into Emulation mode.");
        delay(1000);
      }
  #endif
  }

  bool sendData(const DataPacket& data) {
    if (dir == WifiPortType::Receiver) return false;

    data_packet = data;

    // Send UDP packet to the receiver
    digitalWrite(13, HIGH);
    lastPacketTime = millis();

    if (dir == WifiPortType::Transmitter) {
#ifdef ARDUINO_UNOR4_WIFI
      udp.beginPacket(serverIP, rx_local_port);
      udp.write((uint8_t*)&data, sizeof(DataPacket));
      udp.endPacket();
#endif
    } else {
      has_new_data = true;
      delay(20);// simulate time lag between send/receive
    }

    return true;
  }

  bool checkForData() {
    if (dir == Transmitter)
      return false;

    if (dir == Emulator) {
      return has_new_data;
    }
#ifdef ARDUINO_UNOR4_WIFI
    int packetSize = udp.parsePacket();      //parse out the packet size transmitted, i.e., data Packe Struct size
    if (packetSize >= sizeof(DataPacket)) {  //if sizes match --> we have a transmission
      //within this if statement is the ONLY place we want to do stuff!!!
      udp.read((uint8_t*)&data_packet, sizeof(DataPacket));  //unpack everything in the data DataPacket
      lastPacketTime = millis();
      // Turn on built-in LED to indicate a valid packet was received
      digitalWrite(13, HIGH);
    } else if (20 > millis() - lastPacketTime) {
      digitalWrite(13, LOW);
    }
#else
    int packetSize = 0;
#endif

    if (millis() - lastPacketTime > packetTimeout) {
      Serial.println("RX: Connection lost - no packet received.");
      lastPacketTime = millis();  // Reset to avoid repeated messages
      // Optionally, you might flash the LED to indicate disconnection:
      digitalWrite(13, HIGH);
      delay(100);
      digitalWrite(13, LOW);
      delay(100);
    }

    return has_new_data = packetSize >= sizeof(DataPacket);
  }

  void autoReconnect() {
#ifdef ARDUINO_UNOR4_WIFI

    if (dir != WifiPortType::Transmitter) return void();

    if (20 > millis() - lastPacketTime)
      digitalWrite(13, LOW);

    if (dir == Transmitter) {
      if (millis() - lastWiFiCheck > wifiCheckInterval) {
        if (WiFi.status() != WL_CONNECTED) {
          Serial.println("TX: WiFi disconnected! Reconnecting...");
          connectWiFi();  //calls wifi connection function below
        } else {
          Serial.println("TX: WiFi OK.");
        }
        lastWiFiCheck = millis();
      }
    }
#endif
    return void();
  }

  DataPacket getData() {
    has_new_data = false;
    return data_packet;
  }
};
