#include "WifiPort2.h"

// data packet for WiFi transmission
struct DataPacket {

  bool button = false;
  int joystick1X;
  int joystick1Y;
  int joystick2X;
  int joystick2Y;  

} data;

// initialize WiFi interface
WifiPort<DataPacket> WifiSerial;

// debug
const bool debug = false;

// pins
int buttonPin = 2,
    joy1X = A0,
    joy1Y = A1,
    joy2X = A2,
    joy2Y = A3,
    joy2Power = 3;


// button state variables
bool  buttonState,
      prevState;

void setup() {
  //DONT USE PIN13 FOR ANY SENSOR OR ACTUATORS

  Serial.begin(115200);  //preferred transmission rate for Arduino UNO R4

  // define pinMode for all inputs
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(joy1X, INPUT);
  pinMode(joy1Y, INPUT);
  pinMode(joy2X, INPUT);
  pinMode(joy2Y, INPUT);
  pinMode(joy2Power, OUTPUT);

  digitalWrite(joy2Power, HIGH);

  // begin transmit on WiFi network 
  WifiSerial.begin("group88", "superSecurePassword", WifiPortType::Transmitter);
}

void loop() {

  if (WifiSerial.getPortType() == WifiPortType::Transmitter || WifiSerial.getPortType() == WifiPortType::Emulator) {
    //try and connect
    WifiSerial.autoReconnect();

    // get values from all input devices
    data.joystick1X = analogRead(joy1X);
    data.joystick1Y = analogRead(joy1Y);
    data.joystick2X = analogRead(joy2X);
    data.joystick2Y = analogRead(joy2Y);
    buttonState = digitalRead(buttonPin);

    // true toggle logic for button
    if(!buttonState && buttonState != prevState) {
      data.button = !data.button;
    }

    prevState = buttonState;

    // TX debug
    if(debug){
      Serial.println("Sending: " );
      Serial.print("Button: ");
      Serial.println(data.button);
      Serial.print("Joystick 1: (");
      Serial.print(data.joystick1X);
      Serial.print(", ");
      Serial.print(data.joystick1Y);
      Serial.println(")");
    }

    //check and see if connection is established and data is sent
    if (!WifiSerial.sendData(data))
      Serial.println("Wifi Send Problem");

  }
  
  if ((WifiSerial.getPortType() == WifiPortType::Receiver || WifiSerial.getPortType() == WifiPortType::Emulator) && WifiSerial.checkForData()) {

    //received and unpack data structure
    data = WifiSerial.getData();

    // incorrect mode error
    Serial.println("Incorrect mode selected. Please change to transmit.");
    delay(1000);

  }

  delay(100); // WiFi transmission overhead delay
}
