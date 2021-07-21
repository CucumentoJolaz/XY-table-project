#define Pulse 42

#define Dir 22

long delay_Micros =500; // Set value

long currentMicros = 0; long previousMicros = 0;

void setup()

{
Serial.begin(9600);
pinMode(Pulse,OUTPUT);
pinMode(Dir,OUTPUT);
digitalWrite(Dir,HIGH);

}

void loop()

{


 if (Serial.available() > 0) 
 {
  
    // read incoming serial data:
    char inChar = Serial.read();
  int xDir, yDir; // 0 - платформа не двигается, 1 - платформа двигается вниз (влево), 2 - вверх (вправо)

          if((inChar & 0x43) == 0x43)
          {
            if(bool((1 << 5)& inChar) ^  bool((1 << 3) & inChar))
            {
              if( bool((1 << 5)& inChar))
              {
                xDir = 2;
              }
              else
              {xDir = 1;}
            
            
            Serial.print(inChar & 0x63);
            
            for(int i = 0; i < 90000; i++)
            {
                currentMicros = micros();
                if(currentMicros - previousMicros >= delay_Micros)
                {
                  previousMicros = currentMicros;
                  if(xDir == 2)
                  {
                    digitalWrite(Dir,HIGH);
                    Serial.print("Up");
                  }
                  else if(xDir == 1)
                  {
                    digitalWrite(Dir,LOW);
                    Serial.print("Down");
                  }
                  xDir = 0;
                  digitalWrite(Pulse,HIGH);
                  delayMicroseconds(50); //Set Value
                  digitalWrite(Pulse,LOW);
                }
            }
            Serial.print('\n');
            }
          }
    
 }
}

/*int ENA = 9;
int DIR = 22;
int PUL= 42;
int val;
void setup() {
 //Serial.begin (9600); // Задаем скорость обмена com-порта 9600
  pinMode (PUL, OUTPUT); // Задаем ledpin = 13 как интерфейс вывода информации
  pinMode (DIR, OUTPUT);
  pinMode (ENA, OUTPUT);
}

void loop() {
    digitalWrite(DIR, HIGH);
    delay(20);
    digitalWrite(PUL, HIGH);
    delay(10);
   
}
/*   val = Serial.read ();
   if (val == 'R')
   {*/
