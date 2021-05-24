#if defined(__AVR_ATmega328P__)
// Timer2 is the same on the mega328 and mega168
#define __AVR_ATmega168__
#endif

#include "Arduino.h"

static int32_t basefreq;
static int32_t kickfreq;
static int32_t kickdesc;
static int32_t ampdesc;

static int32_t cnt;
static int32_t amp;
static int32_t freqamp;
static int32_t freq;
uint8_t sinetable[1024];

static bool oldtrigger=false;

void setup(){
  cli();//disable interrupts

//set timer1 interrupt at 15,625khz
  TCCR1A = 0;// set entire TCCR1A register to 0
  TCCR1B = 0;// same for TCCR1B
  TCNT1  = 0;//initialize counter value to 0
  // set compare match register for 1hz increments
  OCR1A=15;    // (16.000.000/64/15625)-1 = 16-1
  // turn on CTC mode
  TCCR1B |= (1 << WGM12);
  // Set CS10 and CS11 bits for 64 prescaler
  TCCR1B |= (1 << CS11) | (1 << CS10);
  // enable timer compare interrupt
  TIMSK1 |= (1 << OCIE1A) | (1<<OCIE2A);

  /****Set timer0 for 8-bit fast PWM output ****/
  pinMode(5, OUTPUT); // Make timer’s PWM pin an output
  TCCR0A = _BV(COM0A1) | _BV(COM0B1) | _BV(WGM01) | _BV(WGM00);
  TCCR0B = _BV(CS00);

  /****Set timer2 for sequencer loop ****/
  TCCR2A = (1<<WGM21) | (1<<CS20) | (1<<CS21) | (1<<CS22);
  TCNT2=0;
  OCR2A=250;            // about 1000hz
  TCCR2B|=(1<<WGM12);
  TIMSK2|=(1<<OCIE2A);

  sei();//enable interrupts

  for (int i=0;i<1024;i++) sinetable[i]=(uint8_t)(sin(((double)i)/1024.0*2.0*3.14159)*127.8+128.f);

  pinMode(A0, INPUT);
  pinMode(A1, INPUT);
  pinMode(A2, INPUT);
  pinMode(A3, INPUT);
  pinMode(A4, INPUT);
  pinMode(A5, INPUT);
  pinMode(A7, INPUT);

//  ADCSRA |= (1 << ADPS1) | (1 << ADPS0);    // set prescaler AD conversion to 153.8 KHz

  Serial.begin(115200);
}

ISR(TIMER1_COMPA_vect){//timer 1 interrupt
      OCR0B=(amp*sinetable[(cnt>>8)&1023])>>15;
      cnt+=freq;
}

uint8_t parambrake=0;
ISR(TIMER2_COMPA_vect) {
  sei();                    // re-enable interrups directly, so the sound interrupt is ensured to run

  parambrake++;
  if (parambrake==8){
    parambrake=0;
    basefreq=((int32_t)analogRead(A0))*2+200;
    kickfreq=((int32_t)analogRead(A1))*2;
    kickdesc=2068-((int32_t)analogRead(A2))*2;
    ampdesc=1043-((int32_t)analogRead(A3));
  }

  if (ampdesc>amp)amp=0;else amp-=ampdesc;
  if (kickdesc>freqamp)freqamp=0;else freqamp-=kickdesc;
  freq=basefreq+((freqamp*kickfreq)>>14);

  uint16_t trigger=analogRead(A7);
  if (trigger<10)oldtrigger=false;
  if (!oldtrigger){
    if (trigger>10){
      if (trigger>200){
        freqamp+=32767;
        amp+=32767;
        cnt=65536;
      } else {
        freqamp+=10000;
        amp+=12000;
      }
      if (freqamp>32767)freqamp=32767;
      if (amp>32767)amp=32767;
      oldtrigger=true;
    }
  }
}

void loop(){
}
