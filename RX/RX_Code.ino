#include "WifiPort2.h"
#include "motor.h"

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

dcMotor LMotor(2, 4, 5);
dcMotor RMotor(7, 8, 6);

int xAxis, yAxis;
int leftSpeed, rightSpeed;
int leftPower, rightPower;

void setup() {
  //DONT USE PIN13 FOR ANY SENSOR OR ACTUATORS

  Serial.begin(115200);  //preferred transmission rate for Arduino UNO R4

  WifiSerial.begin("group88", "superSecurePassword", WifiPortType::Receiver);
}

void loop() {

  if (WifiSerial.getPortType() == WifiPortType::Transmitter || WifiSerial.getPortType() == WifiPortType::Emulator) {
    WifiSerial.autoReconnect();//try and connect

    //Tx stuff below
    Serial.println("Incorrect mode selected. Please select receive.");
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

    xAxis = data.joystick1X - 512;
    yAxis = data.joystick1Y - 512;
  
    int L = (int)(((float)yAxis + (float)xAxis) * 255.0 / 512.0);
    int R = (int)(((float)xAxis - (float)yAxis) * 255.0 / 512.0);
  
    L = constrain(L, -255, 255);
    R = constrain(R, -255, 255);
  
    LMotor.setSpeed(abs(L));
    RMotor.setSpeed(abs(R));
  
    if(L >= 0) {
      LMotor.setDirection(true);
    } else {
      LMotor.setDirection(false);
    }
  
    if(R >= 0) {
      RMotor.setDirection(false);
    } else {
      RMotor.setDirection(true);
    }
    //all RX stuff above

  }

  delay(100); // update delay after you get it working to be a smaller number like 10ms to account for WiFi transmission overhead
}
