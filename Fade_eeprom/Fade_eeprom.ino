#include <CapacitiveSensor.h>
#include <EEPROM.h>

#define time_treshold 100
#define V3 255/5*3
#define touch_samples 60
#define step 5;

CapacitiveSensor sensTouch = CapacitiveSensor(49,51);//moneta su 51
int bin_state=0;
long difference=0;
bool EEPWrite=false;

int lum=0,lum_on=V3;
typedef enum{Increase,Decrease} Direction;
Direction direction;

typedef enum{On,Off,Aux,Fade} State;
State previous_state, present_state, next_state;
typedef enum{Delta,Low,Hold} Transition;
Transition transition;

unsigned long offset,now;//verrà azzerato ogni volta che l'ingresso è a 1 | durante setup()

int readIntEEP(int adress)
{
  byte byte1=EEPROM.read(adress);
  byte byte2=EEPROM.read(adress+1);
  return (byte1 << 8)+byte2;
}
void writeIntEEP(int number, int adress)
{
  EEPROM.write(adress, number >> 8);
  EEPROM.write(adress + 1, number & 0xFF);
}

//_______________________________________________________________________________________________________________________
void decode_input()//legge il touch e calcola il tipo di transizione
{
  difference=millis()-offset;

  if (touch()==false)//Low
  {
    transition=Low;
    offset=millis();
    return;
  }

  if (difference>time_treshold)//Hold
  {
    transition=Hold;
    return;
  
  }
  transition=Delta;//Delta
  return;
}



void calc_next_state()
{
  switch(present_state)
  {
    case Off:
      if (transition==Delta) 
      {
        next_state=On;
        return;
      }
      next_state=Off;
      return;
    
    case On:
      if (transition==Delta) 
      {
        next_state=Aux;
        return;
      }
      next_state=On;
      return;
    
    case Aux:
      if (transition==Low) 
      {
        next_state=Off;
        return;
      }
      if (transition==Hold)
      {
        next_state=Fade;
        return;
      }
      next_state=Aux;
      return;
    case Fade:
      if (transition==Low)
      {
        next_state=On;
        //abilito il salvataggio in eeprom
        EEPWrite=true;
        return;
      }
      next_state=Fade;
      return;
    default:
      return;
  }

}


bool touch()
{
  long total=sensTouch.capacitiveSensor(touch_samples);//aumentare se non funziona bene
  if (total>100)
  {
    delay(200);
    return true;
  }
  return false;
}

void digital_led(int on_off)
{
  if (on_off==1)
  {
    analog_led(V3);
  }
  else
  {
    analog_led(0);
  }
  return;
}
void analog_led(int V)
{
  analogWrite(2,V);
  analogWrite(4,V);
  analogWrite(6,V);
  analogWrite(8,V);
  analogWrite(10,V);
  analogWrite(12,V);

  analogWrite(3,V);
  analogWrite(5,V);
  analogWrite(7,V);
  analogWrite(9,V);
  analogWrite(11,V);
  analogWrite(13,V);
  return;
}
void setup()                    
{
  pinMode(LED_BUILTIN, OUTPUT);
  sensTouch.set_CS_AutocaL_Millis(0xFFFFFFFF);     // turn off autocalibrate on channel 1 - just as an example

  pinMode(2,OUTPUT);
  pinMode(3,OUTPUT);
  pinMode(4,OUTPUT);
  pinMode(5,OUTPUT);
  pinMode(6,OUTPUT);
  pinMode(7,OUTPUT);
  pinMode(8,OUTPUT);
  pinMode(9,OUTPUT);
  pinMode(10,OUTPUT);

  Serial.begin(9600);
  while (!Serial);

  transition=Low;
  offset=millis();
  present_state=Off;

  lum=readIntEEP(0);
  lum_on=lum;

  direction=Decrease;

  delay(1000);//aspetto un po' per simpatia
}

void loop()                    
{
  delay(1);//tenere
  decode_input();
  calc_next_state();

  
  
  previous_state=present_state;
  present_state=next_state;
  switch(present_state)
  {
    case Off:
      lum=0;
      break;
    case On:
      lum=lum_on;
      if(previous_state==Fade)//inverto la direzione di scorrimento quando smetto di regolare la luminosita
      {
        if(direction==Increase) direction=Decrease;
        else direction=Increase;
      }
      break;
    case Aux:
      break;
    case Fade:
      if (direction==Increase)
      {
        lum+=step;
        lum_on=lum;
      }
      else//direction=decrease
      {
        lum-=step;
        lum_on=lum;
      }
      if (lum>V3)
      {
        direction=Decrease;
        //lum=V3;
      }
      else 
        if(lum<1) direction=Increase;
      break;
  }
  
  if(lum<0) lum=0;
  if(lum>V3) lum=V3;

  //analog_led(lum);
/*
  switch(transition)
  {
    case Delta:
      {
        Serial.println("Delta");
        Serial.println(lum);
      }
      break;
    case Low:
      //Serial.println("Low");
      break;
    case Hold:
      if(direction==Increase) 
      {
        Serial.print("Increase");
        Serial.println(lum);
      }
      else
      {
        Serial.print("Decrease");
        Serial.println(lum);
      }
      break;
  }
  */
  
  switch(present_state)
  {
    case Off:
      Serial.print("Off  ");
      Serial.println(lum);
      break;
    case On:
      Serial.print("On   ");
      Serial.println(lum);
      break;
    case Aux:
      Serial.print("Aux  ");
      Serial.println(lum);
      break;
    case Fade:
      Serial.print("Fade ");
      Serial.println(lum);
      break;
    default:
      break;
  }

  if(EEPWrite==true)
  {
    writeIntEEP(lum_on,0);
    EEPWrite=false;
  }
  analog_led(lum);  
  if((lum==V3)&&(present_state==Fade)) delay(1000);
}