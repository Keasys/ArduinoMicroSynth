#include <MIDI.h>

#define SEMITONE 1.059
#define BASEC 0.3311
#define SRATE 5000.0
#define C1 24
#define PERIOD 0.002

#define AMP 9
#define FREQ 10
#define LED 12

#define ATTACK A0
#define DECAY A1
#define SUSTAIN A2
#define RELEASE A3


MIDI_CREATE_DEFAULT_INSTANCE();

typedef struct{
  double t;
  double amp;
}BREAKPOINT;
  
typedef struct{  
  BREAKPOINT *points;
  BREAKPOINT sx,dx;
  double currentTime;
  double increment;
  double width;
  double height;
  boolean morepoints;
  unsigned short isx, idx; 
  unsigned short npoints; 
}ENVELOPE;

BREAKPOINT newPoint(double t, double amp){
  BREAKPOINT p;
  p.amp = amp;
  p.t = t;
  return p;
}
void adsModify(ENVELOPE *env){
  env->points[1].t = analogRead(ATTACK)/1000;
  env->points[2].t = analogRead(DECAY)/1000;
  env->points[2].amp = analogRead(SUSTAIN)/1023;

}

void releaseModify(ENVELOPE *env){
    env->points[0].amp = analogRead(SUSTAIN)/1023;
    env->points[1].t = analogRead(RELEASE)/1000;
}
void envReset(ENVELOPE *env){
  env->currentTime = 0.0;
  env->isx=0;
  env->idx=1;
  env->sx=env->points[env->isx];
  env->dx=env->points[env->idx];
  env->width= env->dx.t - env->sx.t;
  env->height = env->dx.amp - env->sx.amp;
  env->morepoints = true;
  
}

ENVELOPE* newADSEnv(double attackTime,double decayTime,double sustainLev){
  ENVELOPE *env = malloc(sizeof(ENVELOPE));
  env->points=malloc(sizeof(BREAKPOINT)*3);
  env->points[0]=newPoint(0.0,0.0);
  env->points[1]=newPoint(attackTime,1.0);
  env->points[2]=newPoint(decayTime+attackTime,sustainLev);
  env->currentTime = 0.0;
  env->isx=0;
  env->idx=1;
  env->increment=PERIOD;
  env->sx=env->points[env->isx];
  env->dx=env->points[env->idx];
  env->width= env->dx.t - env->sx.t;
  env->height = env->dx.amp - env->sx.amp;
  env->morepoints = true;
  env->npoints=3;
  return env;
}
ENVELOPE* newREnv(double sustainLev, double releaseTime){
  ENVELOPE *env = malloc(sizeof(ENVELOPE));
  env->points=malloc(sizeof(BREAKPOINT)*2);
  env->points[0]=newPoint(0.0,sustainLev);
  env->points[1]=newPoint(releaseTime,0.0);
  env->currentTime = 0.0;
  env->isx=0;
  env->idx=1;
  env->increment=PERIOD;
  env->sx=env->points[env->isx];
  env->dx=env->points[env->idx];
  env->width= env->dx.t - env->sx.t;
  env->height = env->dx.amp - env->sx.amp;
  env->morepoints = true;
  env->npoints=2;
  return env;  
}

double envNextValue(ENVELOPE *e){
  double thisvalue, fraction;
  if(!e->morepoints)
      return e->dx.amp;

  fraction = (e->currentTime - e->sx.t)/e->width;
  thisvalue = e->sx.amp+(e->height*fraction);

  e->currentTime += e->increment;
  if(e->currentTime > e->dx.t){
    e->isx++; e->idx++;
    if(e->idx < e->npoints ){
      e->sx = e->points[e->isx];
      e->dx = e->points[e->idx];
      e->width = e->dx.t-e->sx.t;
      e->height = e->dx.amp - e->sx.amp;
    }
    else
        e->morepoints= false;     
  }
  return thisvalue;
}

double voltageFromMIDI(byte pitch){
  if (pitch < C1)
    pitch = C1;
  return BASEC * pow(pitch-C1+1,SEMITONE);
  
}

double getValueFromVoltage(double voltage){
  return ( ( voltage / 5.0 ) * 255 );
}

//variabili di servizio
enum synthState {notPlaying, on, off};
byte state = notPlaying;
double currentamp;

//inviluppi
ENVELOPE *ADS = newADSEnv(0.3,0.1,0.4);
ENVELOPE *Release = newREnv(0.4,2.4);

void handleNoteOn(byte channel, byte pitch, byte velocity)
{
   analogWrite(FREQ,getValueFromVoltage(voltageFromMIDI(pitch)));
   state = on;   
}

void handleNoteOff(byte channel, byte pitch, byte velocity)
{
   state = off;
}

void setup()
{ 
    pinMode(LED,OUTPUT);
    pinMode(FREQ,OUTPUT);
    pinMode(AMP,OUTPUT);
    
    MIDI.setHandleNoteOn(handleNoteOn);
    MIDI.setHandleNoteOff(handleNoteOff);

    MIDI.begin(MIDI_CHANNEL_OMNI);
    currentamp = 0;
}

void loop()
{    
    adsModify(ADS);
    releaseModify(Release);
    
    MIDI.read();
    
    if(state == on){
      digitalWrite(LED,HIGH);
      currentamp = envNextValue(ADS);                  
    }
    else if(state == off){
      //digitalWrite(LED,LOW);
      currentamp = envNextValue(Release);
      if(!Release->morepoints){
        state = notPlaying;
      } 
    }
    else{
      digitalWrite(LED,LOW);
      envReset(ADS);
      envReset(Release);      
    }
    analogWrite(AMP,currentamp*255);
    delay(2);
}
