#include <Servo.h>
#include "WifiPort2.h"
#include "motor.h"

// Data packet to be transmitted over WiFi
struct DataPacket {

  bool button = false;
  int joystick1X;
  int joystick1Y;
  int joystick2X;
  int joystick2Y;  

} data;

// build WiFi interface
WifiPort<DataPacket> WifiSerial;

// debug
const bool debug = false;

// pins
const short motor1PWM = 6,
            motor1DIR1 = 5, 
            motor1DIR2 = 7,
            motor2PWM = 3,
            motor2DIR1 = 2,
            motor2DIR2 = 4,
            CLAW_SERVO_PIN = A1,
            ARM_SERVO_PIN = A0;

// max/min constraints (currently configured for 17)
const int ARM_MIN = 60,
          ARM_MAX = 130,
          CLAW_MIN = 0,
          CLAW_MAX = 70;

// movement speeds
int clawArmSpeed[2] = {-3, 3};
int movementSpeed[2] = {-255, 255};

// assemble motor objects
dcMotor LMotor(motor1DIR1, motor1DIR2, motor1PWM);
dcMotor RMotor(motor2DIR1, motor2DIR2, motor2PWM);
Servo clawServo;
Servo armServo;

// initialize other necessary variables
int xAxis, yAxis, 
    leftSpeed, rightSpeed, 
    leftPower, rightPower, 
    armJoystick, clawJoystick, 
    armDelta, clawDelta, 
    armPos, clawPos;

void setup() {
  //DONT USE PIN13 FOR ANY SENSOR OR ACTUATORS

  Serial.begin(115200);  //preferred transmission rate for Arduino UNO R4

  // attach pins to servos
  clawServo.attach(CLAW_SERVO_PIN);
  armServo.attach(ARM_SERVO_PIN);

  // set servos to default positions
  clawServo.write(10);
  clawPos = 10;
  delay(20);
  armServo.write(70);
  armPos = 70;
  delay(20);

  // begin receive on WiFi network
  WifiSerial.begin("group88", "superSecurePassword", WifiPortType::Receiver);
}

void loop() {

  if (WifiSerial.getPortType() == WifiPortType::Transmitter || WifiSerial.getPortType() == WifiPortType::Emulator) {
    // try and connect
    WifiSerial.autoReconnect();

    // incorrect mode error
    Serial.println("Incorrect mode selected. Please select receive.");

    //check and see if connection is established and data is sent
    if (!WifiSerial.sendData(data))
      Serial.println("Wifi Send Problem");

  }
  
  if ((WifiSerial.getPortType() == WifiPortType::Receiver || WifiSerial.getPortType() == WifiPortType::Emulator) && WifiSerial.checkForData()) {

    //received and unpack data structure
    data = WifiSerial.getData();

    // RX debug
    if(debug) {
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
    }

    // map values depending on button state (if button pressed, move slower)
    if(data.button) {
      movementSpeed[0] = -127;
      movementSpeed[1] = 127;
      clawArmSpeed[0] = -3;
      clawArmSpeed[1] = 3;
    } else {
      movementSpeed[0] = -255;
      movementSpeed[1] = 255;
      clawArmSpeed[0] = -6;
      clawArmSpeed[1] = 6;
    }

    // get movement axis data
    xAxis = -1 * (data.joystick1X - 512);
    yAxis = -1 * (data.joystick1Y - 512);

    // configure for tank steering
    int L = xAxis - yAxis;
    int R = xAxis + yAxis;

    // constrain values (so that turning isn't too fast)
    // ** may need to be removed pending performance
    L = constrain(L, -512, 512);
    R = constrain(R, -512, 512);
  
    // map values to a movement speed threshold
    // ** if above is removed, change 512 to 1023
    L = map(L, -512, 512, movementSpeed[0], movementSpeed[1]);
    R = map(R, -512, 512, movementSpeed[0], movementSpeed[1]);

    // check for deadzone, set motor speeds
    if(abs(L) > 10) { LMotor.setSpeed(abs(L)); } 
    else { LMotor.setSpeed(0); }
    if(abs(R) > 10) { RMotor.setSpeed(abs(R)); }
    else { RMotor.setSpeed(0); }
  
    // set motor directions (true = cw)
    if(L >= 0) { LMotor.setDirection(false); }
    else { LMotor.setDirection(true); }
    if(R >= 0) { RMotor.setDirection(true); }
    else { RMotor.setDirection(false); }

    // get claw/arm joystick axis data
    armJoystick = -1 * (data.joystick2X - 512);
    clawJoystick = (data.joystick2Y - 512);

    // check for deadzones and map servo deltas
    if(abs(armJoystick) > 10) { armDelta = map(armJoystick, -513, 513, clawArmSpeed[0], clawArmSpeed[1]); }
    else { armDelta = 0; }
    if(abs(clawJoystick) > 10) { clawDelta = map(clawJoystick, -513, 513, clawArmSpeed[0], clawArmSpeed[1]); }
    else { clawDelta = 0; }

    // debug for servo positions/delta
    if(debug){
      Serial.print(armDelta);
      Serial.print(" ");
      Serial.println(clawDelta);
      Serial.print(armPos);
      Serial.print(" ");
      Serial.println(clawPos);
    }

    // check if servos in bounds, move arm +- delta
    if(((armPos + armDelta) > ARM_MIN) && ((armPos + armDelta) < ARM_MAX)) {
      armPos += armDelta;
      armServo.write(armPos);
      delay(20);
    }
    if(((clawPos + clawDelta) > CLAW_MIN) && ((clawPos + clawDelta) < CLAW_MAX)) {
      clawPos += clawDelta;
      clawServo.write(clawPos);
      delay(20);
    }

  }

  delay(10); // WiFi transmittion delay
}
