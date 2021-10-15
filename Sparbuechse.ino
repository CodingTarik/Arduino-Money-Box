#include <SD.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>

// Eventuell redfine HIGH = 1 und LOW = 0

// SD Module (Speichern des Geldstandes)
// Light Barrier Sensor (8x)
// LCD Display
// An / Aus Knopf
// Reset Knopf

// LCD
LiquidCrystal_I2C lcd(0x27, 16, 2); //Hier wird festgelegt um was für einen Display es sich handelt. In diesem Fall eines mit 16 Zeichen in 2 Zeilen und der HEX-Adresse 0x27. Für ein vierzeiliges I2C-LCD verwendet man den Code "LiquidCrystal_I2C lcd(0x27, 20, 4)" 

// Obstacle sensors
int Obstac1eSensors[] = {2,3,4,5,6,7,8,9};

// Reset Button
int ResetButtonPin = 10;

// SD Module
int SDModulePin = 11;

// Will be loaded in setup method if start
double currentBalance = 0.0;

// Setup Methode
void setup() {
  // initialize serial communication at 9600 bits per second:
  Serial.begin(9600);
  
  // Setup Obstacle Sensor Pins
  for(int i = 0; i < 8; i++)
  {
    pinMode(Obstac1eSensors[i], INPUT);
  }

  // Setup Reset Button
  pinMode(ResetButtonPin, INPUT);
  attachInterrupt(digitalPinToInterrupt(ResetButtonPin), ResetMoneyBalance, HIGH);

  
  // Load Balance from Memory
  LoadBalanceFromMemory();

  // Setup LCD
  lcd.init(); //Im Setup wird der LCD gestartet 
  lcd.backlight(); //Hintergrundbeleuchtung einschalten (lcd.noBacklight(); schaltet die Beleuchtung aus). 
  RefreshLCD();
}

// the loop routine runs over and over again forever:
void loop() {
      int coinValue = DetectCoin();  
      currentBalance += coinValue;
      SaveBalance();
}

// Versucht eine Münze zu detektieren
double DetectCoin() {
  // Variablen, ob die coins detektiert wurden
  int sensor1Cent;
  int sensor2Cent;
  int sensor10Cent;
  int sensor5Cent;
  int sensor20Cent;
  int sensor1Euro;
  int sensor50Cent;
  int sensor2Euro;
  
  // Wenn eine Münze reingeworfen wurde, dauert es höchstens 2 Sekunden bis diese durchgerutschts ist (400 * 5ms) = 2 Sekunden
  for(int i = 0; i < 400; i++)
  {
      sensor1Cent = digitalRead(Obstac1eSensors[0]) | sensor1Cent;
      sensor2Cent = digitalRead(Obstac1eSensors[1]) | sensor2Cent;
      sensor10Cent = digitalRead(Obstac1eSensors[2])| sensor10Cent;
      sensor5Cent = digitalRead(Obstac1eSensors[3]) | sensor5Cent;
      sensor20Cent = digitalRead(Obstac1eSensors[4]) |  sensor20Cent;
      sensor1Euro = digitalRead(Obstac1eSensors[5]) | sensor1Euro;
      sensor50Cent = digitalRead(Obstac1eSensors[6]) |  sensor50Cent; 
      sensor2Euro = digitalRead(Obstac1eSensors[7]) | sensor2Euro;
      delay(5); // Warte 5ms
  }

  // Schaue, ob irgendeine Münze entdeckt wurde, da 2 ganz hinten ist, werden die anderen Sensoren durch z.B. Münzeiwurf
  // von 2€ natürlich auch aktiviert, deshalb muss man mit 2 Euro beginnen und Schritt für Schritt runterarbeiten
  if(sensor2Euro == HIGH)
  {
    return 2.0;
  }
  else if(sensor50Cent == HIGH)
  {
    return 0.5;
  }
  else if(sensor1Euro == HIGH)
  {
    return 1.0;
  }
  else if(sensor20Cent == HIGH)
  {
    return 0.2;
  }
  else if(sensor5Cent == HIGH)
  {
    return 0.05;
  }
  else if(sensor10Cent == HIGH)
  {
    return 0.1;
  }
  else if(sensor2Cent == HIGH)
  {
    return 0.02;
  }
  else if(sensor1Cent == HIGH)
  {
    return 0.01;
  }
  else
  {
    return 0;
  }
}

// Refreshes LCD
void RefreshLCD() {
  lcd.setCursor(0, 0);
  lcd.print("Guthaben: ");
  lcd.setCursor(0, 1);
  lcd.print(String(currentBalance) + "€");
}

// Loads Balance from Memory
void LoadBalanceFromMemory()
{
    Serial.println("Reading balance from SD");
    SD.begin(SDModulePin);
    char c;
    String text;
    if(SD.exists("stats.txt"))
    {
         File statsFile = SD.open("stats.txt", FILE_READ);
         while(statsFile.available())
         {
            c = statsFile.read();
            text += c;
         }    
    }
    currentBalance = text.toDouble();  
}

// Saves the balance
void SaveBalance()
{
    Serial.println("Writing new balance to SD");
    SD.begin(SDModulePin);
    File statsFile = SD.open("stats.txt", FILE_WRITE);
    statsFile.write(currentBalance);
    statsFile.close();
    Serial.println("Write successfull");
    RefreshLCD(); 
}

// Resets the balance
void ResetMoneyBalance()
{
   currentBalance = 0;
   SaveBalance();
   RefreshLCD();
}
