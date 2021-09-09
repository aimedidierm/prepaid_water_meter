//#include <A6lib.h>
#include<EEPROM.h>
#include <Keypad.h>
#include <LiquidCrystal.h>
LiquidCrystal lcd(A0,A1,A2,A3,A4,A5);
#define relay 12
const byte ROWS = 4;
const byte COLS = 3;
char newNum[12]="";
unsigned int rupees=0;
unsigned int temp=0,i=0,x=0,k=0;
char str[70],flag2=0;
byte sensorInterrupt = 0;  // 0 = digital pin 2
byte sensorPin       = 2;
float calibrationFactor = 4.5;
volatile byte pulseCount;  
float flowRate;
unsigned int flowMilliLitres;
unsigned long totalMilliLitres;
unsigned long oldTime;
String bal="";
int j=0;
char keys[ROWS][COLS] = {

    {'1','2','3'},

    {'4','5','6'},

    {'7','8','9'},

    {'*','0','#'}

};

byte rowPins[ROWS] = {3, 4, 5, 6}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {7, 8, 9}; //connect to the column pinouts of the keypad
int count=0;
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);
char entryStr[5];  
int m=0;
void setup() 
{
  lcd.begin(16,2);
  Serial.begin(9600);
  pinMode(relay, OUTPUT);
  pinMode(sensorPin, INPUT);
  digitalWrite(sensorPin, HIGH);
  pulseCount        = 0;
  flowRate          = 0.0;
  flowMilliLitres   = 0;
  totalMilliLitres  = 0;
  oldTime           = 0;
  attachInterrupt(sensorInterrupt, pulseCounter, FALLING);
  lcd.setCursor(0,0);
  lcd.print("  Smart Water ");
  lcd.setCursor(0,1);
  lcd.print("      Meter    ");
  delay(2000);
  lcd.clear();
  lcd.print("   Aime Didier");
  delay(2000);
  lcd.clear();
  lcd.print("GSM Initilizing...");
  delay(2000);
  gsm_init();
  lcd.clear();
  lcd.print("System Ready");
  delay(1000);
  lcd.clear();
}
void loop()    
{   
    lcd.clear();
    for (int s = 2; s > 0; s++) {
    lcd.setCursor(0,0);
    lcd.print("Enter number");
    int key = keypad.getKey();
    if (key!=NO_KEY && key!='#' && key!='*')

    {
        newNum[j] = key;
        newNum[j+1]='\0';   
        j++;
        lcd.setCursor(0,1);
        lcd.print(newNum);
    }
    if (key=='#'&& j>11)
    {
        j=0;
        m=0;
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("invalid number");
        lcd.setCursor(0,0);
        lcd.print("11 digits only");
    }

    else if (key=='*')
    {
        lcd.rightToLeft();
        lcd.print(" ");
        lcd.leftToRight();
        lcd.print(" ");
        lcd.rightToLeft();
        lcd.print(" ");
        lcd.leftToRight();
        j--;
        newNum[x] = 0;
    }
    else if (key=='#'&& j==10)
    {
        j=0;
        m=0;
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print(newNum);
        delay(500);
}
  if (j == 10) {
    break;
  }
  }
    serialEvent();
    rupees=EEPROM.read(1);
    lcd.clear();
    lcd.setCursor(0,1);
    lcd.print(rupees);
    lcd.print(" ml");
     if((millis() - oldTime) > 1000)    // Only process counters once per second
  { 
    detachInterrupt(sensorInterrupt);
    flowRate = ((1000.0 / (millis() - oldTime)) * pulseCount) / calibrationFactor;
    oldTime = millis();    
    flowMilliLitres = (flowRate / 60) * 1000;
    totalMilliLitres += flowMilliLitres;
    unsigned int frac;            
    frac = (flowRate - int(flowRate)) * 10; 
    pulseCount = 0;
    attachInterrupt(sensorInterrupt, pulseCounter, FALLING);
  }   
    rupees=rupees-flowMilliLitres;
    EEPROM.write(1,rupees);
    check_status();
    if(temp==1)
    {
     decode_message();
     send_confirmation_sms();
    }
    delay(100);
}
void pulseCounter()
{
  pulseCount++;
}
void serialEvent()
{
  while(Serial.available())
  {
    char ch=(char)Serial.read();
    str[i++]=ch;
    if(ch == '*')
    {
      temp=1;
      lcd.clear();
      lcd.print("Message Received");
      delay(500);
      break;
    }
  }
}
void init_sms()
{
   Serial.println("AT+CMGF=1");
   delay(200);
   Serial.println("AT+CMGS=\"+250788750979\"");
   delay(200);
}
void send_data(String message)
{
  Serial.println(message);
  delay(200);
}
void send_sms()
{
  Serial.write(26);
}
void check_status()
{
      if(rupees>0)
      {
        digitalWrite(relay, HIGH);
        flag2=0;
      }
      if(rupees==0 && flag2==0)
     {
      digitalWrite(relay, LOW);
      lcd.clear();
      lcd.print("Water Cut Due to");
      lcd.setCursor(0,1);
      lcd.print("Low Balance");
      delay(2000);
      lcd.clear();
      lcd.print("Please Recharge ");
      lcd.setCursor(0,1);
      lcd.print("UR Water Meter ");
      init_sms();
      send_data("Water Meter Balance Alert:\nWater cut due to low Balance\nPlease recharge your Water meter soon.\n Thank you");
      send_sms();
      message_sent();
      flag2=1;
    }
}
void decode_message()
{
  x=0,k=0,temp=0;
     while(x<i)
     {
      while(str[x]=='#')
      {
        x++;
        bal="";
        while(str[x]!='*')
        {
          bal+=str[x++];
        }
      }
      x++;
    }
    bal+='\0';
}
void send_confirmation_sms()
{
    int recharge_amount=bal.toInt();
    rupees+=recharge_amount;
    EEPROM.write(1, rupees);
    lcd.clear();
    lcd.print("Water Meter ");
    lcd.setCursor(0,1);
    lcd.print("Recharged:");
    lcd.print(recharge_amount);
    init_sms();
    send_data("Water Meter Balance Alert:\nYour Water meter has been recharged Rs:");
    send_data(bal);
    send_data("Total Balance:");
    Serial.println(rupees);
    temp=0;
    i=0;
    x=0;
    k=0;
    delay(1000);
    message_sent();
}
void message_sent()
{
  lcd.clear();
  lcd.print("Message Sent.");
  delay(1000);
}
void gsm_init()
{
  lcd.clear();
  lcd.print("Finding Module..");
  boolean at_flag=1;
  while(at_flag)
  {
    Serial.println("AT");
    while(Serial.available()>0)
    {
      if(Serial.find("OK"))
      at_flag=0;
    }
    delay(1000);
  }
  lcd.clear();
  lcd.print("Module Connected..");
  delay(1000);
  lcd.clear();
  lcd.print("Disabling ECHO");
  boolean echo_flag=1;
  while(echo_flag)
  {
    Serial.println("ATE0");
    while(Serial.available()>0)
    {
      if(Serial.find("OK"))
      echo_flag=0;
    }
    delay(1000);
  }
  lcd.clear();
  lcd.print("Echo OFF");
  delay(1000);
  lcd.clear();
  lcd.print("Finding Network..");
  boolean net_flag=1;
  while(net_flag)
  {
    Serial.println("AT+CPIN?");
    while(Serial.available()>0)
    {
      if(Serial.find("+CPIN: READY"))
      net_flag=0;
    }
    delay(1000);
  }
  lcd.clear();
  lcd.print("Network Found..");
  delay(1000);
  lcd.clear();
}


