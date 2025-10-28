#include "WifiPort2.h"

// A structure, similar to our servo and stepper motors, but this one conatins variables to be transmitted
// Any variable you want to transmit/recieve must be initalized in the DataPacket structure
struct DataPacket {

  bool button = false;
  int joystick1X;
  int joystick1Y;
  int joystick2X;
  int joystick2Y;  


} data;

//gloabl vars are outside datapacket

WifiPort<DataPacket> WifiSerial;
int buttonPin = 3;
int joy1X = A0;
int joy1Y = A1;
int joy2X = A2;
int joy2Y = A3;
bool buttonState;
bool prevState;

void setup() {
  //DONT USE PIN13 FOR ANY SENSOR OR ACTUATORS

  Serial.begin(115200);  //preferred transmission rate for Arduino UNO R4

  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(joy1X, INPUT);
  pinMode(joy1Y, INPUT);
  pinMode(joy2X, INPUT);
  pinMode(joy2Y, INPUT);

  WifiSerial.begin("group88", "superSecurePassword", WifiPortType::Transmitter);
  // WifiSerial.begin("group88", "superSecurePassword", WifiPortType::Receiver);
  // WifiSerial.begin("group88", "superSecurePassword", WifiPortType::Emulator); // one board to rule them all debugging
}

void loop() {

  if (WifiSerial.getPortType() == WifiPortType::Transmitter || WifiSerial.getPortType() == WifiPortType::Emulator) {
    WifiSerial.autoReconnect();//try and connect

    data.joystick1X = analogRead(joy1X);
    data.joystick1Y = analogRead(joy1Y);
    data.joystick2X = analogRead(joy2X);
    data.joystick2Y = analogRead(joy2Y);
    buttonState = digitalRead(buttonPin);

    if(!buttonState && buttonState != prevState) {
      data.button = !data.button;
    }

    prevState = buttonState;

    //Tx stuff below
    Serial.println("Sending: " );
    Serial.print("Button: ");
    Serial.println(data.button);
    Serial.print("Joystick 1: (");
    Serial.print(data.joystick1X);
    Serial.print(", ");
    Serial.print(data.joystick1Y);
    Serial.println(")");
    //Tx stuff above


    if (!WifiSerial.sendData(data))//check and see if connection is established and data is sent
      Serial.println("Wifi Send Problem");//oh no it didn't send --> it iwll try and re-connect at the start of the loop

  }


  
  if ((WifiSerial.getPortType() == WifiPortType::Receiver || WifiSerial.getPortType() == WifiPortType::Emulator) && WifiSerial.checkForData()) {

    data = WifiSerial.getData();//received and unpack data structure

    //all Rx stuff below
    Serial.println("Receiving: " );
    Serial.print("Button: ");
    Serial.println(data.button);
    Serial.print("Joystick 1: (");
    Serial.print(data.joystick1X);
    Serial.print(", ");
    Serial.print(data.joystick1Y);
    Serial.println(")");
    //all RX stuff above

  }

  delay(100); // update delay after you get it working to be a smaller number like 10ms to account for WiFi transmission overhead
}
