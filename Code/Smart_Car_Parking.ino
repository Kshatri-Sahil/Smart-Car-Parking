#include <Servo.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Ultrasonic.h> // Include the Ultrasonic library
#include <SPI.h>
#include <MFRC522.h>

#define SS_PIN 10
#define RST_PIN 9

MFRC522 mfrc522(SS_PIN, RST_PIN);

#define I2C_ADDR 0x27
#define BACKLIGHT_PIN 2 
#define LCD_COLUMNS 16
#define LCD_ROWS 2

LiquidCrystal_I2C lcd(I2C_ADDR, LCD_COLUMNS, LCD_ROWS);
const int servoPin = 3;
Servo myservo;

#define ir_enter 2
#define ir_back  4

#define trigPin1 5 // Ultrasonic sensor 1 Trig pin
#define echoPin1 6 // Ultrasonic sensor 1 Echo pin

#define trigPin2 7 // Ultrasonic sensor 2 Trig pin
#define echoPin2 8 // Ultrasonic sensor 2 Echo pin

Ultrasonic ultrasonic1(trigPin1, echoPin1); // Create Ultrasonic objects for each sensor
Ultrasonic ultrasonic2(trigPin2, echoPin2);

int S1 = 0, S2 = 0;
int flag1 = 0, flag2 = 0;
int slot = 2;

void setup() {
  Serial.begin(9600);
  SPI.begin();        // Init SPI bus
  mfrc522.PCD_Init(); // Init MFRC522

  Serial.println("RFID Reader Initialized");
  Serial.println("Scan a RFID tag...");

  pinMode(ir_enter, INPUT);
  pinMode(ir_back, INPUT);

  myservo.attach(servoPin);
  myservo.write(0);
  lcd.init();
  // Turn on the backlight
  lcd.backlight();
  //lcd.begin(16, 2);
  lcd.setCursor(2, 0);
  lcd.print("Car parking");
  lcd.setCursor(3, 1);
  lcd.print("System");
  delay(2000);
  lcd.clear();
  
  Serial.print("Car parking");
  Read_Sensor();

  int total = S1 + S2;
  slot = slot - total;
  }

void loop() {
  Read_Sensor();

  lcd.setCursor(0, 0);
  lcd.print("Have Slot:");
  lcd.print(slot);
  lcd.print("    ");

  lcd.setCursor(0, 1);
  lcd.print("S1:");
  lcd.print(S1 > 0 ? "Fill " : "Empty");
  lcd.setCursor(9, 1);
  lcd.print("S2:");
  lcd.print(S2 > 0 ? "Fill " : "Empty");

  Serial.print("   Have Slot: ");
  Serial.print(slot);
  Serial.print("S1:");
  Serial.print(S1 > 0 ? "Fill" : "Empty");
  Serial.print("S2:");
  Serial.print(S2 > 0 ? "Fill" : "Empty");
 
  if (digitalRead(ir_enter) == 0 && flag1 == 0) {
    if (slot > 0) {
      flag1 = 1;
      if (flag2 == 0) {
        myservo.write(90);
        slot = slot - 1;
      }
    }
    else {
      lcd.setCursor(0, 0);
      lcd.print(" Parking Full ");
      Serial.print(" Sorry Parking Full ");
      delay(1500);
    }
  }

  if (digitalRead(ir_back) == 0 && flag2 == 0) {
    flag2 = 1;
    if (flag1 == 0) {
    
      myservo.write(180);
      slot = slot + 1;
    }
  }

  if (flag1 == 1 && flag2 == 1) {
    delay(1000);
    myservo.write(0);
    flag1 = 0, flag2 = 0;
  }

  if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
    // Show UID on serial monitor
    Serial.print("UID tag :");
    String content = "";

    for (byte i = 0; i < mfrc522.uid.size; i++) {
      content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
      content.concat(String(mfrc522.uid.uidByte[i], HEX));
    }
    content.toUpperCase();
    Serial.println(content);

    // Compare UID to allowed IDs
    if (content.substring(1) == "AE 95 23 A3") {
      Serial.println("Allowed");
      if (slot > 0) {
      flag1 = 1;
      if (flag2 == 0) {
        myservo.write(90);
        slot = slot - 1;
      }
    }
    else {
      lcd.setCursor(0, 0);
      lcd.print(" Parking Full ");
      Serial.print(" Sorry Parking Full ");
      delay(1500);
    }

      delay(1000);
      myservo.write(90);
      // Add your code for allowed access here
    } else {
      Serial.println("Not Allowed");
      // Add your code for denied access here
    }
    delay(1000); // Delay to avoid multiple readings
  }
  delay(100); // Adjust delay as needed

}

void Read_Sensor() {
  S1 = 0, S2 = 0;

  // Read distance values from ultrasonic sensors
  long distance1 = ultrasonic1.distanceRead();
  long distance2 = ultrasonic2.distanceRead();

  if (distance1 < 5) { // Adjust threshold as needed
    S1 = 1;
  }
  if (distance2 < 5) { // Adjust threshold as needed
    S2 = 1;
  }
}
