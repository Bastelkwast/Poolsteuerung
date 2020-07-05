/*
  Analog input, analog output, serial output

  Reads an analog input pin, maps the result to a range from 0 to 255 and uses
  the result to set the pulse width modulation (PWM) of an output pin.
  Also prints the results to the Serial Monitor.

  The circuit:
  - potentiometer connected to analog pin 0.
    Center pin of the potentiometer goes to the analog pin.
    side pins of the potentiometer go to +5V and ground
  - LED connected from digital pin 9 to ground

  created 29 Dec. 2008
  modified 9 Apr 2012
  by Tom Igoe

  This example code is in the public domain.

  http://www.arduino.cc/en/Tutorial/AnalogInOutSerial
*/

// These constants won't change. They're used to give names to the pins used:
const int analogInPin = A4;  // Analog input pin that the potentiometer is attached to
//const int analogOutPin = 9; // Analog output pin that the LED is attached to

int sensorValue = 0;        // value read from the pot
float mittelwert = 120;
int outputValue = 0;        // value output to the PWM (analog out)
unsigned long next_print = 0;
unsigned long sys_time = 0;
int print_time = 1000;

void setup() {
  // initialize serial communications at 9600 bps:
  Serial.begin(9600);
}

void loop() {
  // read the analog in value:
  sensorValue = analogRead(analogInPin);
  mittelwert = ((mittelwert * 100) + sensorValue) / 101;
  // map it to the range of the analog out:
  outputValue = map(mittelwert, 55, 530, -10, 250);
  // change the analog out value:
  //analogWrite(analogOutPin, outputValue);
//  sys_time = millis();
//  Serial.print(sys_time);
//  Serial.print("    ");
//  Serial.println(next_print);

 if ( next_print <= millis()){
    next_print = millis() + print_time;
    print_temp();
  }

  // wait 2 milliseconds before the next loop for the analog-to-digital
  // converter to settle after the last reading:

}
void print_temp() {
  // print the results to the Serial Monitor:
  Serial.print("sensorValue = ");
  Serial.print(sensorValue);
  Serial.print("  ");
  Serial.print("Mittelwert = ");
  Serial.print(mittelwert);
  Serial.print("  ");
  Serial.print(outputValue);
  Serial.println(" Grad");
}
