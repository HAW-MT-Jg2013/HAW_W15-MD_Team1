#include "TimerOne.h"
#include "avr/interrupt.h"

  // Variablen für Hardware Pins
  // Pin 0 und 1 frei für Kommunikation
  // Pin 2 für Funksignal (Interrupt pin)
  int IR2 = 3;
  int IR3 = 4;
  int IR4 = 5;
  int IR5 = 6;
  int IR6 = 7;
  int IR7 = 8;
  // Pin 9 und 10 frei wegen timer
  int IR8 = 11;
  int IR1 = 12;
  int funk = 2;
  int LED = 13 ; // Status led
  
  // Funksignal Variablen
  int funkalt = 0;
  int funkneu = 0;
  
  
  //Kreisfrequenz definieren
  float f = 50;  // Frequenz des Nordimpulses
  float T = 1/f;
  float pi= 3.141592;
  float w = f*2*pi;  // angepasst für rechnung in Grad
  float T25 = T/4;  // entspricht 90°
  float T50 = T/2;  // entspricht 180°
  float T75 = T*3/4;// ensprichtt 270°
  int merker1 = 0;  // Information ob bereits ein Signal von Turm 1 empfangen wurde. 0 nein, 1 ja.
  float mT1 = 0;    // Steigung Turm 1
  float bT1 = 3;    // Y-Achsen Abschnitt Turm 1
  int merker2 = 0;
  float mT2 = 0;
  float bT2 = 0;
  int merker3a = 0;  // 3a steht für Turm 3, wenn rechte Hälfte
  float mT3a = 0;
  float bT3a = 0;
  int merker3b = 0;  // 3b steht für Turm 3, wenn linke Hälfte
  float mT3b = 0;
  float bT3b = 0;
  float helptime =0;
  int checksum = 0; //Wird hochgezählt, wenn IR Signal empfangen wird
  
  float X = 0; // X-Koordiate absolutes Koordinatensystem
  float Y = 0;// Y-Koordinate absolutes Koordinatenystem
  float *Xp = &X; // Zeiger auf X
  float *Yp = &Y; // Zeiger auf Y
  
  // Funktionen zur Positionsbestimmung
  void schnittpunkt (float var_m1, float var_m2, float var_b1, float var_b2, float* var_x, float* var_y){
  *var_x = (var_b1-var_b2)/(var_m2-var_m1);
  *var_y = ((var_b1/var_m1)-(var_b2/var_m2))/((1/var_m1)-(1/var_m2));
  // Serial.println("Funktion wurde aufgerufen");
  }
  
void setup() {
  
  pinMode(LED, OUTPUT);
  pinMode(IR1, INPUT);
  pinMode(IR2, INPUT);
  pinMode(IR3, INPUT);
  pinMode(IR4, INPUT);
  pinMode(IR5, INPUT);
  pinMode(IR6, INPUT);
  pinMode(IR7, INPUT);
  pinMode(IR8, INPUT);
  
  // Timer Konfigurieren
  Timer1.initialize(200010);
  Timer1.setPeriod(200010); // Sets the period in microseconds
  
  Serial.begin(9600); // Seriellen Monitor aktiviert -> Pin 1 & 2 belegt 
 
}

void loop() {

// Nordimuls als Flankenerkennung
funkneu = digitalRead(funk);
if(funkneu > funkalt )
  {
    Timer1.restart();
    funkalt = funkneu;
//    Serial.print("Funk");
//    Serial.println(funkneu);    
    }
// XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX

  
if((digitalRead(IR1) || digitalRead(IR2) || digitalRead(IR3) || digitalRead(IR4) ||
digitalRead(IR5) || digitalRead(IR6) || digitalRead(IR7) || digitalRead(IR8)) == LOW)    // Die if anweisung als interrupt umsetzen

   {helptime = Timer1.read();
    helptime = helptime/1000000;
   Serial.print("helptime=");
   Serial.println(helptime,8);
    
     if ((helptime >=0 && helptime <= T25) && merker3a == 0 && merker3b == 0)  // Auswahl des Quadranten Turm 3 rechts
    { mT3a=tan((pi/2)-(w*helptime));                              // Steigung berechnet
      bT3a=-mT3a;      // Offset berechnet
      merker3a = 1;
      checksum = checksum+1;
      Serial.println("Turm3a");
       
      }
     else if (helptime > T25 && helptime <= T50 && merker1 == 0)  // Turm 1
     { mT1=-tan((helptime*w)-(pi/2));
       merker1=1;
       checksum = checksum+1;
       Serial.println("Turm1");
       }
       
      else if (helptime > T50 && helptime <= T75 && merker2 == 0)  // Turm 2
      { mT2=tan((3*pi/2)-(helptime*w));
        bT2=3-2*mT2;
        merker2=1;
         checksum = checksum+1;
        Serial.println("Turm2");
        }
       else if (helptime > T75 && helptime < T && merker3b == 0 && merker3a== 0)                   // Turm 3 links
       { mT3b=-tan((helptime*w)-(3*pi/2));
         bT3b=-mT3b;
         merker3b=1;
         checksum = checksum+1;
         Serial.println("Turm3b");
      }
      else {}
       
       }
       
  if (checksum >= 2)    // Wenn sich zwei oder mehr Signale im Speicher befinden
  { if (merker1==1&&merker2==1)  // Auswerten Welche Türme für die Berechnung verwendet werden
    {  schnittpunkt (mT1, mT2, bT1, bT2, Xp, Yp);
      }
      else {if (merker1==1&&merker3a==1)
      {schnittpunkt (mT1, mT3a, bT1, bT3a, Xp, Yp);
        }
        else {if (merker1==1&&merker3b==1)
        {schnittpunkt (mT1, mT3b, bT1, bT3b, Xp, Yp);
          }
          else {if (merker2==1&&merker3a==1)
          {schnittpunkt (mT2, mT3a, bT2, bT3a, Xp, Yp);
            }
            else {if (merker2==1&&merker3b==1)
            {schnittpunkt (mT2, mT3b, bT2, bT3b, Xp, Yp);
              }
              else {if (merker3a==1 && merker3b==1)
                        {merker3a=merker3b=0;}
              else{ X=10;  // Um zu testen, ob merker nicht funktionieren
                   Y=10;
                   }}}}}}
                   
                   
  Serial.print('H');//Headar zur Identifikation einer neuen NAchricht
  Serial.print(",");//Komma zum Trennen von Elementen
  Serial.print(X);
  Serial.print(",");
  Serial.print(Y);
  Serial.print(",");//Komma zum beenden des letzten Elements
  
//              Serial.print("X=");     
//              Serial.println(X);
//              Serial.print("Y=");
//              Serial.println(Y);
//              Serial.print("mT1=");
//              Serial.println(mT1);
//              Serial.print("mT2=");
//              Serial.println(mT2);
//              Serial.print("mT3a=");
//              Serial.println(mT3a);
//              Serial.print("mT3b=");
//              Serial.println(mT3b);
//              Serial.print("bT1=");
//              Serial.println(bT1);
//              Serial.print("bt2=");
//              Serial.println(bT2);
//              Serial.print("bT3a=");
//              Serial.println(bT3a);
//              Serial.print("bT3b=");
//              Serial.println(bT3b);
//              Serial.print("Merker1=");
//              Serial.println(merker1);
//              Serial.print("Merker2=");
//              Serial.println(merker2);
//              Serial.print("Merker3a=");
//              Serial.println(merker3a);
//              Serial.print("Merker3b=");
//              Serial.println(merker3b);
              checksum=mT1=mT2=mT3a=mT3b=bT2=bT3a=bT3b=merker1=merker2=merker3a=merker3b=0;
              }
  
   }
       

