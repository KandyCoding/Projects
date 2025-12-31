#include <Arduino.h>

void For_Ward1(char Speed);
void For_Back1(char Speed);
void For_Ward2(char Speed);
void For_Back2(char Speed);
void For_Ward3(char Speed);
void For_Back3(char Speed);
void For_Ward4(char Speed);
void For_Back4(char Speed);

char Incoming_value;

//*************************************//DC Motor Speed//*********************************************
int Speed_1 = 3; 
int Speed_2 = 11;
int Speed_3 = 5;
int Speed_4 = 6;

//*************************************// DC Motor Direction//*********************************************
int Motor_1F = 4; 
int Motor_2F = 12;
int Motor_3B = 8;
int Motor_4B = 7;


void For_Ward1() ;  
void For_Back1() ;
void For_Ward2() ;
void For_Back2() ;
void For_Ward3() ;
void For_Back3() ;    
void For_Ward4() ;
void For_Back4() ;
 
class Smart_Car {
 public:
       int i ;
        
} ;

//*************************************//Top Right DC Motor//*********************************************
void For_Ward1(char Speed) ///<Motor1 Back off
{
 digitalWrite(Motor_1F,HIGH) ;
 analogWrite(Speed_1,Speed) ;
}
void For_Back1(char Speed) ///<Motor1 Back off
{
 digitalWrite(Motor_1F,LOW);
 analogWrite(Speed_1,Speed);
}
//*************************************//Top Left DC Motor//*********************************************
void For_Ward2(char Speed)
{
  digitalWrite(Motor_2F,HIGH);
  analogWrite(Speed_2,Speed);
}
void For_Back2(char Speed)
{
  digitalWrite(Motor_2F,LOW);
  analogWrite(Speed_2,Speed);
}

//*************************************//Bottom Right DC Motor//*********************************************
void For_Ward3(char Speed)
{
  digitalWrite(Motor_3B,HIGH);
  analogWrite(Speed_3,Speed);
}

void For_Back3(char Speed)
{
  digitalWrite(Motor_3B,LOW);
  analogWrite(Speed_3,Speed);
}

//*************************************//Bottom Left DC Motor//*********************************************
void For_Ward4(char Speed)
{
  digitalWrite(Motor_4B,HIGH);
  analogWrite(Speed_4,Speed);
}

void For_Back4(char Speed)
{
  digitalWrite(Motor_4B,LOW);
  analogWrite(Speed_4,Speed);
}


void setup() {
  Serial.begin(9600);
}

void main_Func(){
  if(Serial.available()) {
    Incoming_value = Serial.read();     
    Serial.print(Incoming_value);        
    Serial.print("\n");
  
//*************************************//Direction Move Forward//**********************************************
    if(Incoming_value == 'U'){
      for(int i=3;i<9;i++)
        pinMode(i,OUTPUT);
        for(int i=11;i<13;i++)
          pinMode(i,OUTPUT);
          For_Ward1(100);
          For_Ward2(100);
          For_Ward3(100);
          For_Ward4(100);
    }

  //*************************************//Direction Move Backward//*********************************************
    if(Incoming_value == 'd'){
      for(int i=3;i<9;i++)
        pinMode(i,OUTPUT);
        for(int i=11;i<13;i++)
          pinMode(i,OUTPUT);
          For_Back1(100);
          For_Back2(100);
          For_Back3(100);
          For_Back4(100);
    }

  //*************************************//Direction Turn Left//*************************************************
    if(Incoming_value == 'R'){
      for(int i=3;i<9;i++)
        pinMode(i,OUTPUT);
        for(int i=11;i<13;i++)
          pinMode(i,OUTPUT);
          For_Ward1(100);
          For_Back2(100);
          For_Ward3(100);
          For_Back4(100);
    }

  //*************************************//Direction Turn Right//*************************************************
    if(Incoming_value == 'F'){
      for(int i=3;i<9;i++)
        pinMode(i,OUTPUT);
        for(int i=11;i<13;i++)
          pinMode(i,OUTPUT);
          For_Back1(100);
          For_Ward2(100);
          For_Back3(100);
          For_Ward4(100);
    }

  //*********************************************//Stop moving//**************************************************
    if(Incoming_value == 'O'){
      for(int i=3;i<9;i++)
        pinMode(i,OUTPUT);
        for(int i=11;i<13;i++)
          pinMode(i,OUTPUT);
          For_Ward1(0);
          For_Back1(0);
          For_Ward2(0);
          For_Back2(0);
          For_Ward3(0);
          For_Back3(0);
          For_Ward4(0);
          For_Back4(0);
    }
  }
}

void loop() {
  main_Func() ;
  }
