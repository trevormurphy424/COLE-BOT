#include <Servo.h>
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

// pins
const short motor1PWM = 6,
            motor1DIR1 = 5, 
            motor1DIR2 = 7,
            motor2PWM = 3,
            motor2DIR1 = 2,
            motor2DIR2 = 4,
            CLAW_SERVO_PIN = A1,
            ARM_SERVO_PIN = A0;

dcMotor LMotor(motor1DIR1, motor1DIR2, motor1PWM);
dcMotor RMotor(motor2DIR1, motor2DIR2, motor2PWM);
Servo clawServo;
Servo armServo;

int xAxis, yAxis;
int leftSpeed, rightSpeed;
int leftPower, rightPower;
int armDelta, clawDelta;
int armPos, clawPos;

void setup() {
  //DONT USE PIN13 FOR ANY SENSOR OR ACTUATORS

  Serial.begin(115200);  //preferred transmission rate for Arduino UNO R4

  clawServo.attach(CLAW_SERVO_PIN);
  armServo.attach(ARM_SERVO_PIN);

  clawServo.write(10);
  clawPos = 10;
  delay(20);
  armServo.write(70);
  armPos = 70;
  delay(20);

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
    Serial.print("Joystick 2: (");
    Serial.print(data.joystick2X);
    Serial.print(", ");
    Serial.print(data.joystick2Y);
    Serial.println(")");

    xAxis = data.joystick1X - 512;
    yAxis = data.joystick1Y - 512;
  
    int L = xAxis + yAxis;
    int R = xAxis - yAxis;
  
    L = map(L, -1023, 1023, -255, 255);
    R = map(R, -1023, 1023, -255, 255);
  
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

    armDelta = map(data.joystick2X, 0, 1023, -6, 6);
    clawDelta = map(data.joystick2Y, 0, 1023, -6, 6);

    Serial.print(armDelta);
    Serial.print(" ");
    Serial.println(clawDelta);

    if(((armPos + armDelta) > 20) && ((armPos + armDelta) < 110)) {
      armPos += armDelta;
      armServo.write(armPos);
      delay(20);
    }
    if(((clawPos + clawDelta) > 0) && ((clawPos + clawDelta) < 45)) {
      clawPos += clawDelta;
      clawServo.write(clawPos);
      delay(20);
    }
    //all RX stuff above

  }

  delay(10); // update delay after you get it working to be a smaller number like 10ms to account for WiFi transmission overhead
}
