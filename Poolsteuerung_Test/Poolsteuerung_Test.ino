/*Das ist eine Steuerung für einen Pool die mit normalen Bauteilen wie Arduino Uno, Shield und Relais aufgebaut werden kann.
  Als Sensor wird der DS18B20 verwendet*/

#include <OneWire.h> // http://www.arduino.cc/playground/Learning/OneWire
#include <DallasTemperature.h> // http://milesburton.com/index.php?title=Dallas_Temperature_Control_Library
#include <LiquidCrystal.h>
#include <EEPROM.h>
LiquidCrystal lcd(8, 9, 4, 5, 6, 7); //Angabe der erforderlichen Pins

#define ONE_WIRE_BUS A5
OneWire oneWire(ONE_WIRE_BUS); // Einrichten des OneWire Bus um die Daten der Temperaturfühler abzurufen

char firmwareversion[] = "Rev.1";

//Variablen für die Temperatursensoren
#define TEMPERATURE_PRECISION 9

// Addresses of 3 DS18B20s
uint8_t sensor1[8] = { 0x28, 0xFF, 0x02, 0xB4, 0x80, 0x14, 0x02, 0x5D };
uint8_t sensor2[8] = { 0x28, 0xFF, 0xEE, 0x19, 0x80, 0x14, 0x02, 0x92 };
uint8_t sensor3[8] = { 0x28, 0xFF, 0x39, 0x1B, 0x80, 0x14, 0x02, 0x9A };

// Speicherplatz für die Temeraturwerte
float temp1 = 0;
float temp2 = 0;
float temp3 = 0;

// Status der Sensoren ob ein Fehler am Sensor vorliegt
boolean sensor_fail = false;
int sensor_fail1 = false;
int sensor_fail2 = false;
int sensor_fail3 = false;

// Bezeichnung der einzelnen Sensoren
char sensorName1[] = "Pool:      ";
char sensorName2[] = "Vorlauf:   ";
char sensorName3[] = "Ruecklauf: ";

//Variablen für die Temperaturanzeige auf dem LCD
const int MaxSensors = 3;
int TempShow = 0;

#define Tasterrechts 0
#define Tasteroben 1
#define Tasterunten 2
#define Tasterlinks 3
#define Tasterselect 4
#define KeinTaster 5

// Erstellen einiger Variablen
int Taster = 0;
int Analogwert = 0;
int TempCheckDelay = 2000; //Zeit zwischen den Temperaturmessungen
int Temp1 = 0;
int TempDivEin = 5;
int TempDivAus = 2;

//Speicherplatz im EEPROM der einzelnen Variablen
int address0 = 0; //Temp.Diff.Ein
int address1 = 1; //Temp.Diff.Aus
int address2 = 2; // Betrieb Ein/Aus

// Ausgänge für die Relais
int relais1 = A1;
int relais2 = A2;
int relais3 = A3;
int relais4 = A4;
int relaisstat1 = LOW;
int relaisstat2 = LOW;
int relaisstat3 = LOW;
int relaisstat4 = LOW;

unsigned long NextTempCheck = 0;
boolean Betrieb = 1;

boolean Pumpe = 0; // Entspricht Pumpe "Aus"

int keypad_pin = A0;
int keypad_value = 0;
int keypad_value_old = 0;

char btn_push;

byte mainMenuPage = 1;
byte mainMenuPageOld = 1;
byte mainMenuTotal = 4;

unsigned long Button_Push = 0;
int Goto_Menue_1 = 5000 ; //Zeit in msec. bis Info erscheint

DallasTemperature sensors(&oneWire); // Bindung der Sensoren an den OneWire Bus

DeviceAddress tempDeviceAddress; // Verzeichniss zum Speichern von Sensor Adressen

int numberOfDevices; // Anzahl der gefundenen Sensoren

//************************************************************** setup ***********************************
void setup()
{
  pinMode (relais1, OUTPUT);
  digitalWrite (relais1, HIGH);//Schaltet das Relais aus
  pinMode (relais2, OUTPUT);
  Serial.begin(9600);
  sensors.begin();
  lookUpSensors();
  lcd.begin(16, 2); // Starten der Programmbibliothek.
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(" Poolsteuerung   ");
  lcd.setCursor(0, 1);
  lcd.print(" ");
  lcd.print(firmwareversion);
  TempDivEin = EEPROM.read(address0);
  TempDivAus = EEPROM.read(address1);
  Betrieb = EEPROM.read(address2);
  delay(1000);
}

//***************************************************** loop ***********************************************
void loop()
{
  btn_push = ReadKeypad();

  if ((btn_push == 'U') || (btn_push == 'D') || (btn_push == 'L') || (btn_push == 'R')) {
    Button_Push = millis();

    MainMenuBtn();

  }

  if (btn_push == 'R') //enter selected menu
  {
    WaitBtnRelease();
    switch (mainMenuPage)
    {
      case 1:
        MenuA();
        break;
      case 2:
        MenuB();
        break;
      case 3:
        MenuC();
        break;
      case 4:
        MenuD();
        break;
    }

    MainMenuDisplay();
    WaitBtnRelease();
  }

  if (millis() >= NextTempCheck) {
    Tempcheck();
    if (millis() >= Button_Push + Goto_Menue_1) {
      mainMenuPage = 1;
      Printtemp();
      setRelais();

    }

  }

  delay(10);

}//--------------- End of loop() loop ---------------------

// ******************************************************* Tempcheck *************************************************
void Tempcheck() {
  sensors.requestTemperatures();
  temp1 = sensors.getTempC(sensor1);
  
  if (temp1 <= -100) {
    sensor_fail1 = true;
  }
  else {
    sensor_fail1 = false;
  }
  temp2 = sensors.getTempC(sensor2);
  if (temp2 <= -100) {
    sensor_fail2 = true;
  }
  else {
    sensor_fail2 = false;
  }
  temp3 = sensors.getTempC(sensor3);
  if (temp3 <= -100) {
    sensor_fail3 = true;
  }
  else {
    sensor_fail3 = false;
  }
  NextTempCheck = millis() + TempCheckDelay;

  if (sensor_fail1 || sensor_fail2 || sensor_fail3) {
    sensor_fail = true;
  }
  else
  {
    sensor_fail = false;
  }
}

// ***************************************************** Printtemp **********************************************************
void Printtemp() {
  Serial.print(sensorName1);
  Serial.print(temp1);
  Serial.println(" Celsius");

  Serial.print(sensorName2);
  Serial.print(temp2);
  Serial.println(" Celsius");

  Serial.print(sensorName3);
  Serial.print(temp3);
  Serial.println(" Celsius");
  Serial.println();

  //Ausgabe der Temp Div
  Serial.print("Tem. Diff. Ein: ");
  Serial.print(TempDivEin);
  Serial.println();
  Serial.print("Tem. Diff. Aus: ");
  Serial.print(TempDivAus);
  Serial.println();

  if (sensor_fail1) {
    Serial.print (sensorName1);
    Serial.println ("Sensorfehler");
  }
  if (sensor_fail2) {
    Serial.print (sensorName2);
    Serial.println ("Sensorfehler");
  }
  if (sensor_fail3) {
    Serial.print (sensorName3);
    Serial.println ("Sensorfehler");
  }

  LCD_temp();
}

//***************************************************************** LCD_temp ****************************************
void LCD_temp() {
  lcd.clear ();
  TempShow ++;
  if (TempShow == 1) {
    lcd.setCursor(0, 0);
    lcd.print(sensorName1);

    if (!sensor_fail1) {
      lcd.print(temp1, 1);
      lcd.print("C");
    }
    else  {
      //lcd.setCursor(0, 1);
      lcd.print("fail");
    }
    pumpstat();
  }
  if (TempShow == 2) {
    lcd.setCursor(0, 0);
    lcd.print(sensorName2);

    if (!sensor_fail2) {
      lcd.print(temp2, 1);
      lcd.print("C");
    }
    else  {
      //lcd.setCursor(0, 1);
      lcd.print("fail");
    }
    pumpstat();
  }
  if (TempShow == 3) {
    TempShow = 0;

    lcd.setCursor(0, 0);
    lcd.print(sensorName3);

    if (!sensor_fail3) {
      lcd.print(temp3, 1);
      lcd.print("C");
    }
    else  {
      //lcd.setCursor(0, 1);
      lcd.print("fail");
    }
    pumpstat();
  }

}

//*************************************************************** pumpstat ***********************************************
void pumpstat() {
  // Ausgabe Pumpenstatus
  lcd.setCursor(0, 1);
  lcd.print("Pumpe ");
  if (relaisstat1 == 1) {
    lcd.print ("An");
  }
  else {
    lcd.print ("Aus");
  }
}

//******************************************************************** setRelais ******************************************
void setRelais() {
  Serial.print("Betrieb = ");
  Serial.println(Betrieb);
  if (temp2 - temp1 >= TempDivEin && Betrieb && !sensor_fail) {
    digitalWrite (relais1, LOW);
    relaisstat1 = 1;
    //Serial.println("Relais1 = Ein:");
    //Serial.println();
  }
  else if (temp2 - temp1 <= TempDivAus || Betrieb == 0 || sensor_fail) {
    digitalWrite (relais1, HIGH);
    relaisstat1 = 0;
    //Serial.println("Relais1 = Aus:");
    //Serial.println();
  }
  Serial.print("Relais1 = ");
  Serial.println(relaisstat1);
  Serial.println();
}


// ******************************************************* Menu A-D ************************************************************************
void MenuA()
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Inside Menu A");

  while (ReadKeypad() != 'L')
  {
    //Insert Task for Menu A here

  }
}

void MenuB()
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Temp Diff Ein");
  lcd.setCursor(0, 1);
  lcd.print(TempDivEin);
  lcd.print(" Grad");

  while (ReadKeypad() != 'L')
  {
    btn_push = ReadKeypad();

    if (btn_push == 'U') {
      TempDivEin ++;
      WaitBtnRelease();
      MenuB();
    }
    if (btn_push == 'D') {
      TempDivEin --;
      WaitBtnRelease();
      MenuB();
    }

    Button_Push = millis();

  }
  EEPROM.write(address0, TempDivEin);
  delay (10);
}
void MenuC()
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Temp Diff Aus");
  lcd.setCursor(0, 1);
  lcd.print(TempDivAus);
  lcd.print(" Grad");

  while (ReadKeypad() != 'L')
  {
    btn_push = ReadKeypad();

    if (btn_push == 'U') {
      TempDivAus ++;
      WaitBtnRelease();
      MenuC();
    }
    if (btn_push == 'D') {
      TempDivAus --;
      WaitBtnRelease();
      MenuC();
    }

    Button_Push = millis();

  }
  EEPROM.write(address1, TempDivAus);
  delay (10);
}
void MenuD()
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Betrieb:");
  lcd.setCursor(10, 0);
  //lcd.print (Betrieb);
  if (Betrieb == 1 ) {
    lcd.print("Ein");
  }
  if (Betrieb == 0) {
    lcd.print("Aus");
  }
  //lcd.print(" Grad");*/

  while (ReadKeypad() != 'L')
  {
    btn_push = ReadKeypad();

    if (btn_push == 'U') {
      Betrieb = 1;
      WaitBtnRelease();
      MenuD();
    }
    if (btn_push == 'D') {
      Betrieb = 0;
      WaitBtnRelease();
      MenuD();
    }

    Button_Push = millis();

  }
  EEPROM.write(address2, Betrieb);
  delay (10);
}

//****************************************************** MainMenuDisplay ********************************************
void MainMenuDisplay()
{
  lcd.clear();
  lcd.setCursor(0, 0);
  switch (mainMenuPage)
  {
    case 1:
      Printtemp();
      break;
    case 2:
      lcd.print("Set. Diff. Ein");
      break;
    case 3:
      lcd.print("Set. Diff. Aus");
      break;
    case 4:
      lcd.print("Betrieb");
      lcd.setCursor(0, 1);
      lcd.print("Ein / Aus");
      break;
  }
}

// ************************************************************ MainMenuBtn ********************************************
void MainMenuBtn()
{
  WaitBtnRelease();
  if (btn_push == 'D')
  {
    mainMenuPage++;
    if (mainMenuPage > mainMenuTotal)
      mainMenuPage = 2;
  }
  else if (btn_push == 'U')
  {
    mainMenuPage--;
    if (mainMenuPage == 1)
      mainMenuPage = mainMenuTotal;
  }

  if (mainMenuPage != mainMenuPageOld) //only update display when page change
  {
    MainMenuDisplay();
    mainMenuPageOld = mainMenuPage;
  }
}

// **************************************************************** ReadKeypad ********************************************
char ReadKeypad()
{
  /* Keypad button analog Value
    no button pressed 1023
    select  741
    left    503
    down    326
    up      142
    right   0
  */
  keypad_value = analogRead(keypad_pin);

  if (keypad_value < 50)
    return 'R';
  else if (keypad_value < 195)
    return 'U';
  else if (keypad_value < 380)
    return 'D';
  else if (keypad_value < 555)
    return 'L';
  else if (keypad_value < 780)
    return 'S';
  else
    return 'N';

}

// ****************************************************************** WaitBtnRelease *****************************************
void WaitBtnRelease()
{
  while ( analogRead(keypad_pin) < 800) {}
}

// ****************************************************** Sensorabfrage **************************************************************
//int Sensorabfrage()
//{
//  // Aufruf der Funktion sensors.requestTemperatures()
//  // Dadurch werden alle werte abgefragt.
//  Serial.print("Abfrage der Temperatur... ");
//  sensors.requestTemperatures();
//  Serial.println("DONE");
//
//  // Ausgabe der Daten für jeden Sensor
//  for (int i = 0 ; i < numberOfDevices; i++) {
//    float tempC = sensors.getTempCByIndex(i);
//    Serial.print("Sensor ");
//    Serial.print(i, DEC);
//    Serial.print(" hat grad Celsius: ");
//    Serial.println(tempC);
//  }
//}

// ******************************************************* lookUpSensors **********************************************
void lookUpSensors()
{
  byte address[8];
  int i = 0;
  byte ok = 0, tmp = 0;

  Serial.println("--Suche gestartet--");
  while (oneWire.search(address))
  {
    tmp = 0;
    //0x10 = DS18S20
    if (address[0] == 0x10)
    {
      Serial.print("Device is a DS18S20 : ");
      tmp = 1;
    }
    else
    {
      //0x28 = DS18B20
      if (address[0] == 0x28)
      {
        Serial.print("Device is a DS18B20 : ");
        tmp = 1;
      }
    }
    //display the address, if tmp is ok
    if (tmp == 1)
    {
      if (OneWire::crc8(address, 7) != address[7])
      {
        Serial.println("but it doesn't have a valid CRC!");
      }
      else
      {
        //all is ok, display it
        for (i = 0; i < 8; i++)
        {
          if (address[i] < 9)
          {
            Serial.print("0");
          }
          Serial.print("0x");
          Serial.print(address[i], HEX);
          if (i < 7)
          {
            Serial.print(", ");
          }
        }
        Serial.println("");
        ok = 1;
      }
    }//end if tmp
  }//end while
  if (ok == 0)
  {
    Serial.println("Keine Sensoren gefunden");
  }
  Serial.println("--Suche beendet--");
  Serial.println("Abfrage mehrerer Dallas Temperatur Sensoren");
  Serial.println("-------------------------------------------");

  // Suche der Sensoren
  Serial.println("Suche Temperatur Sensoren...");
  sensors.begin();
  numberOfDevices = sensors.getDeviceCount();

  Serial.print("Habe ");
  Serial.print(numberOfDevices, DEC);
  Serial.println(" Sensoren gefunden.");

  // Setzen der Genauigkeit
  for (int i = 0 ; i < numberOfDevices; i++) {
    if (sensors.getAddress(tempDeviceAddress, i)) {
      sensors.setResolution(tempDeviceAddress, TEMPERATURE_PRECISION);
      Serial.print("Sensor ");
      Serial.print(i);
      Serial.print(" hat eine genauigkeit von ");
      Serial.println(sensors.getResolution(tempDeviceAddress), DEC);
    }

    lcd.setCursor(0, 0); // Angabe des Cursorstartpunktes oben links.

  }
}
