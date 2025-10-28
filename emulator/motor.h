class dcMotor { 
  public:
    dcMotor(int pin2, int pin6, int pin1) {
      pinMode(pin2, OUTPUT);
      pinMode(pin6, OUTPUT);
      pinMode(pin1, OUTPUT);

      _enPin = pin1;
      _in1Pin = pin2;
      _in2Pin = pin6;
    }

    /*
     * true is CW, false is CCW
     */
    void setDirection(bool dir) {
      if(dir == false) {
        digitalWrite(_in1Pin, LOW);
        digitalWrite(_in2Pin, HIGH);
      } else {
        digitalWrite(_in1Pin, HIGH);
        digitalWrite(_in2Pin, LOW);
      }
    }

    void setSpeed(int speed) {
      analogWrite(_enPin, speed);
    }

  private:
    int _enPin;
    int _in1Pin;
    int _in2Pin;
};
