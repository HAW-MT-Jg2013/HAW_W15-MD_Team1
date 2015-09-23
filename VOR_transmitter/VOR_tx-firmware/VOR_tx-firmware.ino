// 2 x 7-Segment ansteuern mit Schieberegister 74HC595
int led = 13;
int dataPin = 12;
int clockPin = 11;
int latch1Pin = 10;
int latch2Pin = 9;
int latch3Pin = 8;
int latch4Pin = 7;
int interruptNumber = 0; // Interrupt-Nummer
byte Flanke = 0x00
byte Register = 0x01;
byte count = 0x00;
byte SR_res = 0x00;
void setup () {
  
  pinMode(dataPin,OUTPUT);
  pinMode(clockPin,OUTPUT);
  pinMode(latch1Pin,OUTPUT);
  pinMode(latch2Pin,OUTPUT);  
  pinMode(latch3Pin,OUTPUT);
  pinMode(latch4Pin,OUTPUT);
  pinMode(led,OUTPUT);
// alle Latch-Signale auf low
  digitalWrite(latch1Pin,LOW);
  digitalWrite(latch2Pin,LOW);
  digitalWrite(latch3Pin,LOW);
  digitalWrite(latch4Pin,LOW);
  
  count = 0x01;
  attachInterrupt(interruptNumber, interruptroutine, RISING);
  
}
void interruptroutine(){
Flanke==0x01;
}

void loop() {
  
  
switch(Register){
  case 1:
  
  digitalWrite(led,HIGH);
  // 1 x Lauflicht Schieberegister 1
  for(int i=0;i<8;i++){  // 8 Impulse mit for-Schleife
    shiftOut(dataPin,clockPin,MSBFIRST,count);
    // Latch-Impuls für Schieberegister 1
    digitalWrite(latch1Pin,HIGH); 
    delayMicroseconds(230);
    digitalWrite(latch1Pin,LOW);
    delayMicroseconds(230);
    
    
    count *=2;
    if (count == 0)
    {
      count = 0x01;
    } 
  }
  //alle Ausgänge des Schieberegisters auf low
  shiftOut(dataPin,clockPin,MSBFIRST,SR_res);
  // Latch-Impuls für Schieberegister 1
  digitalWrite(latch1Pin,HIGH); 
    delayMicroseconds(5);
    digitalWrite(latch1Pin,LOW);
    Register = 0x02;
  break;
  
  
   case 2:
  
  digitalWrite(led,HIGH);
  // 1 x Lauflicht Schieberegister 1
  for(int i=0;i<8;i++){  // 8 Impulse mit for-Schleife
    shiftOut(dataPin,clockPin,MSBFIRST,count);
    // Latch-Impuls für Schieberegister 2
    digitalWrite(latch2Pin,HIGH); 
    delayMicroseconds(230);
    digitalWrite(latch2Pin,LOW);
    delayMicroseconds(230);
    
    
    count *=2;
    if (count == 0)
    {
      count = 0x01;
    } 
  }
  //alle Ausgänge des Schieberegisters auf low
  shiftOut(dataPin,clockPin,MSBFIRST,SR_res);
  // Latch-Impuls für Schieberegister 2
  digitalWrite(latch2Pin,HIGH); 
    delayMicroseconds(5);
    digitalWrite(latch2Pin,LOW);
    Register = 0x03;
  break;
  
    case 3:
  
  digitalWrite(led,LOW);
  // 1 x Lauflicht Schieberegister 3
  for(int i=0;i<8;i++){  // 8 Impulse mit for-Schleife
    shiftOut(dataPin,clockPin,MSBFIRST,count);
    // Latch-Impuls für Schieberegister 3
    digitalWrite(latch3Pin,HIGH); 
    delayMicroseconds(230);
    digitalWrite(latch3Pin,LOW);
    delayMicroseconds(230);
    
    
    count *=2;
    if (count == 0)
    {
      count = 0x01;
    } 
  }
  //alle Ausgänge des Schieberegisters auf low
  shiftOut(dataPin,clockPin,MSBFIRST,SR_res);
  // Latch-Impuls für Schieberegister 3
  digitalWrite(latch3Pin,HIGH); 
    delayMicroseconds(5);
    digitalWrite(latch3Pin,LOW);
    Register = 0x04;
  break;
  
    case 4:
  
  digitalWrite(led,LOW);
  // 1 x Lauflicht Schieberegister 4
  for(int i=0;i<7;i++){  // 7 Impulse mit for-Schleife
    shiftOut(dataPin,clockPin,MSBFIRST,count);
    // Latch-Impuls für Schieberegister 4
    digitalWrite(latch4Pin,HIGH); 
    delayMicroseconds(230);
    digitalWrite(latch4Pin,LOW);
    delayMicroseconds(230);
    
    
    count *=2;
    if (count == 0)
    {
      count = 0x01;
    } 
  }
  
  
  
  
  //alle Ausgänge des Schieberegisters auf low
  shiftOut(dataPin,clockPin,MSBFIRST,SR_res);
  // Latch-Impuls für Schieberegister 4
  digitalWrite(latch4Pin,HIGH); 
    delayMicroseconds(5);
    digitalWrite(latch4Pin,LOW);
    Register = 0x01;
  break;
  
  
  
 }
}
  
  

