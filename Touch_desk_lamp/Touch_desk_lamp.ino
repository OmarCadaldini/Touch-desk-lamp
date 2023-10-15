#include <CapacitiveSensor.h>
#include <EEPROM.h>

#define time_treshold 100
#define V3 255/5*3
#define touch_samples 60
#define step 5;

CapacitiveSensor sensTouch = CapacitiveSensor(49,51);//moneta su 51
int bin_state=0;
long difference=0;
bool EEPWrite=false,alt=false;

int lum=0,lum_on=V3;
typedef enum{Increase,Decrease} Direction;
Direction direction,s2_direction,s3_direction;

typedef enum{Off,Aux_off,On,Aux_on,S2,Aux_s2,S3,Aux_s3,S4,Aux_s4,Fade} State;
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
    case On:
      if (transition==Delta) 
      {
        next_state=Aux_on;
        return;
      }
      next_state=On;
      return;
    
    case Aux_on:
      if (transition==Low)
      {
        next_state=S2;
        s2_direction=Decrease;
        return;
      }
      if (transition==Hold)
      {
        next_state=Fade;
        return;
      }
      next_state=Aux_on;
    
    
    case S2:
      if (transition==Delta)
      {
        next_state=Aux_s2;
        return;
      }
      next_state=S2;
      return;
    
    case Aux_s2:
      if (transition==Low)
      {
        next_state=S3;
        return;
      }
      if (transition==Hold)
      {
        next_state=Fade;
        return;
      }
      next_state=Aux_s2;
      return;
    
    case S3:
      if (transition==Delta)
      {
        next_state=Aux_s3;
        return;
      }
      next_state=S3;
      return;
    
    case Aux_s3:
      if (transition==Low)
      {
        next_state=S4;
        return;
      }
      if (transition==Hold)
      {
        next_state=Fade;
        return;
      }
      next_state=Aux_s3;
      return;
    
    case S4:
      if (transition==Delta)
      {
        next_state=Aux_s4;
        return;
      }
      next_state=S4;
      return;
    
    case Aux_s4:
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
      next_state=Aux_s4;
      return;
    
    case Off:
      if (transition==Delta) 
      {
        next_state=Aux_off;;
        return;
      }
      next_state=Off;;
      return;
    
    case Aux_off:
      if(transition==Low)
      {
        next_state=On;
        return;
      }
      if (transition==Hold)
      {
        next_state=Fade;
        return;
      }
      next_state=Aux_off;
    
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
  if (total>130)//100
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
    analog_led(V3,false);
  }
  else
  {
    analog_led(0,false);
  }
  return;
}
void analog_led(int V,bool alt)
{
  if(alt)
  {   
    analogWrite(2,V);
    analogWrite(4,lum_on-V);
    analogWrite(6,V);
    analogWrite(8,V);
    analogWrite(10,lum_on-V);
    //analogWrite(12,V);

    analogWrite(3,V);
    analogWrite(5,lum_on-V);
    analogWrite(7,V);
    analogWrite(9,lum_on-V);
    //analogWrite(11,V3-V);
    //analogWrite(13,V3-V);
    return;
  }
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
  s2_direction=Decrease;
  delay(1000);//aspetto un po' per simpatia
}

void loop()                    
{
  delay(1);//tenere
  decode_input();
  calc_next_state();

  
  
  previous_state=present_state;
  present_state=next_state;

  switch(present_state)//Azioni associate agli stati
  {
    case On:
      lum=lum_on;
      if(previous_state==Fade)//inverto la direzione di scorrimento quando smetto di regolare la luminosita
      {
        if(direction==Increase) direction=Decrease;
        else direction=Increase;
      }
      break;

    case Aux_on:
      break;
    
    case S2://fade continuo
      if (s2_direction==Increase)
      {
        lum+=1;
      }
      else//direction=decrease
      {
        lum-=1;
      }
      if (lum>lum_on)
      {
        s2_direction=Decrease;
      }
      else 
        if(lum==0) s2_direction=Increase;
      delay(1500/lum_on);
      
      
      break;
    
    case Aux_s2:
      break;
    
    case S3:
      /*if (s3_direction==Increase)
      {
        lum+=1;
      }
      else//direction=decrease
      {
        lum-=1;
      }
      if (lum>lum_on)
      {
        s3_direction=Decrease;
      }
      else 
        if(lum==2) s3_direction=Increase;
      delay(1500/lum_on);*/
      present_state=Off;
      previous_state=Aux_s4;
      break;
    
    case Aux_s3:
      break;

    case S4:
      //analogWrite(10,80);
      break;

    case Aux_s4:
      break;

    case Off:
      lum=0;
      break;
    
    case Aux_off:
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

  
  switch(present_state)//scrive in seriale
  {
    
    case On:
      Serial.print("On   ");
      Serial.println(lum);
      break;

    case Aux_on:
      Serial.print("Aux_on  ");
      Serial.println(lum);
      break;

    case S2:
      Serial.print("S2   ");
      Serial.println(lum);
      break;

    case Aux_s2:
      Serial.print("Aux_s2  ");
      Serial.println(lum);
      break;
    
    case S3:
      Serial.print("S3   ");
      Serial.println(lum);
      break;

    case Aux_s3:
      Serial.print("Aux_s3  ");
      Serial.println(lum);
      break;
    
    case S4:
      Serial.print("S4   ");
      Serial.println(lum);
      break;

    case Aux_s4:
      Serial.print("Aux_s4  ");
      Serial.println(lum);
      break;
    
    case Off:
      Serial.print("Off  ");
      Serial.println(lum);
      break;
    case Aux_off:
      Serial.print("Aux_off  ");
      Serial.println(lum);
      break;

    case Fade:
      Serial.print("Fade ");
      Serial.println(lum);
      break;

    default:
      break;
  }

  if(EEPWrite==true)//gestisce la scrittura in EEPROM
  {
    writeIntEEP(lum_on,0);
    EEPWrite=false;
  }
  if (present_state==S3) alt=true;
  else alt=false;
  analog_led(lum,alt);
}
