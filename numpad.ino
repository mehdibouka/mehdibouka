/* ***********************************************************
 *                         Libraries                         *
 * ********************************************************* */
#include <SPI.h>
#include <MFRC522.h>

/* ***********************************************************
 *                      Global Constants                     *
 *                    Hardware Definitions                   *
 * ********************************************************* */
 /* Typical pin layout used:
 * -----------------------------------------------------------------------------------------
 *             MFRC522      Arduino       Arduino   Arduino    Arduino          Arduino
 *             Reader/PCD   Uno           Mega      Nano v3    Leonardo/Micro   Pro Micro
 * Signal      Pin          Pin           Pin       Pin        Pin              Pin
 * -----------------------------------------------------------------------------------------
 * RST/Reset   RST          9             5         D9         RESET/ICSP-5     RST
 * SPI SS      SDA(SS)      10            53        D10        10               10
 * SPI MOSI    MOSI         11 / ICSP-4   51        D11        ICSP-4           16
 * SPI MISO    MISO         12 / ICSP-1   50        D12        ICSP-1           14
 * SPI SCK     SCK          13 / ICSP-3   52        D13        ICSP-3           15
 */    
/* ***********************************************************
 *                      Global Variables                     *
 * ********************************************************* */
const char rows = 4; 
const char cols = 3; 
const int relay=14;
const int buttonPin=15;
const int buzzer=16;
const int ledPin=17;
int buttonRead;
const int PasswordLength = 4;  
String MasterPassword ="0304"; 
String correctPassword[100];
int key_count=0;
int key_index;


const char keys[rows][cols] = {
              {'1','2','3'},
               {'4','5','6'},
               {'7','8','9'},
               {'*','0','#'}
               };
               
char rowPins[rows] = {8, 7, 6, 5};
char colPins[cols] = {4, 3, 2};

bool stats;
char* ValidTags[12] = {};
String ReadTag = "";
int Tagindex;
int Tagcount=0;
const byte rstPin = 9;
const byte ssPin = 10;
MFRC522 Rfid(ssPin, rstPin);



void setup () {
     Serial.begin(9600);
     SPI.begin();                            
     pinMode(buttonPin, INPUT);
     pinMode(relay,OUTPUT);
     pinMode(ledPin,OUTPUT);
     pinMode(buzzer,OUTPUT);   
     for(char r = 0; r < rows; r++){
           pinMode(rowPins[r], INPUT);    //set the row pins as input
           digitalWrite(rowPins[r], HIGH);    //turn on the pullups
     }     
     
     for(char c = 0; c < cols; c++){
           pinMode(colPins[c], OUTPUT);   //set the column pins as output
     }

  Rfid.PCD_Init();
  Rfid.PCD_DumpVersionToSerial();
  Serial.println("Set the master tag:");
  
  while (!Rfid.PICC_IsNewCardPresent()) {}
  Rfid.PICC_ReadCardSerial();
  get_tag_id();
  ReadTag="D1F1222D";
  ValidTags[Tagcount] = strdup(ReadTag.c_str());
  Tagcount++;
          
}
    
   


void loop() {
      bool state=false;
      String key="A";
      buttonRead=digitalRead(buttonPin);
      if(button_press()=='*'){
      digitalWrite(ledPin,HIGH);
      key = get_key();
      state=prog_state(key);
      digitalWrite(ledPin,LOW);
      }
      if(state==true){
        key="A";
        while(key=="A"){key=get_key();}
        Serial.println("adding key:"+key);
        add_key(key);
        }
      bool door=check_key(key);
      
        if(door==true){
        
        open_door(door);
        }
  if (Rfid.PICC_IsNewCardPresent()) {
    if (Rfid.PICC_ReadCardSerial()) {
      get_tag_id();
      stats = check_tag_id();
      if (!stats) {
        if(ReadTag == ValidTags[0]) {          
          add_rem_tags();
          return;
        }
      }
      open_door(stats);
    } else {
      return;
    }
  }

        if(buttonRead==0){
           Serial.println("button in here");
           open_door(true);
        }
      delay(1000);
}

bool check_key(String key){
 for(int i=0;i<100;i++){ 
      
  if(key==correctPassword[i]){ 
    return true;
      }else if(key=="A"){
        return false;}
        }
  int m=0;
  while(m<4){
  Serial.println(key);
  digitalWrite(buzzer,HIGH);
  digitalWrite(ledPin,HIGH);
  delay(100);
  digitalWrite(buzzer,LOW);
  digitalWrite(ledPin,LOW);
  delay(600);
  Serial.println(m);
  m++;
  }  
  }
void open_door(bool val){
  if(val==true){
    Serial.println("door open");
    digitalWrite(relay,HIGH);
    digitalWrite(buzzer,HIGH);
    digitalWrite(ledPin,HIGH);
    delay(1000);
    digitalWrite(buzzer,LOW);
    delay(1000);
    digitalWrite(ledPin,LOW);
    digitalWrite(relay,LOW);
    Serial.println("door Closed");
    delay(50);
    }
  }
  
String get_key(){
      int t=0;
      String k;
      while(t<PasswordLength){
        for(char c = 0; c < cols; c++){
          digitalWrite(colPins[c], LOW);
           for(char r = 0; r < rows; r++){
              if(digitalRead(rowPins[r]) == LOW){
              delay(50);    //20ms debounce time
              while(digitalRead(rowPins[r])== LOW);
              k = k+keys[r][c];
              t++;
              }
           }
   digitalWrite(colPins[c], HIGH); 
      }
      }
      return k;
}
char button_press(){
      char k = 0;
      
      for(char c = 0; c < cols; c++){
        digitalWrite(colPins[c], LOW);
         for(char r = 0; r < rows; r++){
            if(digitalRead(rowPins[r]) == LOW){
            delay(20);    //20ms debounce time
            while(digitalRead(rowPins[r])== LOW);
            k = keys[r][c];
            }
         }
   digitalWrite(colPins[c], HIGH); 
      }
      return k;
}

bool prog_state(String key){
  bool prog_mode1;
    if (key == MasterPassword){
      prog_mode1=true;
    }
    else{
      prog_mode1=false;
      }
  return prog_mode1;
    }

void add_key(String key){
  if(key!=MasterPassword){
  for(int i=0;i<100;i++){
    if(key==correctPassword[i]){key_index=i;}
    }
  if(key_index==0){
    correctPassword[key_count]=key;
    key_count++;
    Serial.println("added:"+key);
    }

  }else{
    Serial.println("Cannot add master pasword to valid pass");
    }
}

bool check_tag_id() {
  for (int i = 1; i < 12; i++) {
    if (ReadTag == ValidTags[i]) {
      return true;
    }
  }
  return false;
}

void get_tag_id() {
  ReadTag = "";
  for (byte i = 0; i < Rfid.uid.size; i++) {
    ReadTag.concat(String(Rfid.uid.uidByte[i], HEX));
  }
  ReadTag.toUpperCase();
  Rfid.PICC_HaltA();
  Serial.println(ReadTag);
}


void add_rem_tags() {
    ReadTag ="";
    Tagindex = 0;
    while (!Rfid.PICC_IsNewCardPresent()) {digitalWrite(ledPin,HIGH);}
      digitalWrite(ledPin,LOW);
      Rfid.PICC_ReadCardSerial();
      get_tag_id();
    for (int e = 1; e < 12; e++) {
      if (ReadTag == ValidTags[e]) {
        Tagindex = e;
      }
    }
    
    if (Tagindex != 0) {
      ValidTags[Tagindex] =ValidTags[100];
    } else if (ReadTag != ValidTags[0]) {
      ValidTags[Tagcount] = strdup(ReadTag.c_str());
      Tagcount++;
      Serial.println("added this tag: " + ReadTag);
      return;
    } else {
      Serial.println("Cannot add the master tag as a Valid Tag. It's only for adding or removing tags from the valid tags list.");
      return;
    } 
}
