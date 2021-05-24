#if defined(__AVR_ATmega328P__)
// Timer2 is the same on the mega328 and mega168
#define __AVR_ATmega168__
#endif

#include "Arduino.h"

static int32_t tonefreq;
static uint8_t snappiness;
static int32_t toneamp;
static int32_t ampdesc;
static int32_t accent;

static int32_t cnt;
static int32_t amp;
static int32_t samp;
static int32_t freqamp;
uint8_t sinetable[1024];

static bool oldtrigger=false;

void setup(){
  cli();//disable interrupts

//set timer1 interrupt at 15,625khz
  TCCR1A = 0;// set entire TCCR1A register to 0
  TCCR1B = 0;// same for TCCR1B
  TCNT1  = 0;//initialize counter value to 0
  // set compare match register for 1hz increments
  OCR1A=20;    // (16.000.000/64/15625)-1 = 16-1
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

  for (int i=0;i<1024;i++) {
    sinetable[i]=(uint8_t)(sin(((double)i)/1024.0*2.0*3.14159)*127.8+128.f);
  }

  pinMode(A0, INPUT);
  pinMode(A1, INPUT);
  pinMode(A2, INPUT);
  pinMode(A3, INPUT);
  pinMode(A4, INPUT);
  pinMode(A5, INPUT);
  pinMode(A7, INPUT);

  ADCSRA &= ~(bit (ADPS0) | bit (ADPS1) | bit (ADPS2)); // set ADC prescaler to 16 => 1MHz ADC, required to get all the analogReads in time with a little loss in precision
  ADCSRA |= bit (ADPS2);                                // depends on how much time is used in the 2nd interrupt. 4 analog parameters should work, 5 is tricky :)

  Serial.begin(115200);
}
static uint32_t reg=13475283UL;
ISR(TIMER1_COMPA_vect){//timer 1 interrupt
//      OCR0B=(amp*sinetable[(cnt>>8)&1023])>>15;
//      OCR0B=(amp*random()&255)>>15;

  uint32_t newr;
  uint8_t lobit;
  uint8_t b31, b29, b25, b24;
  b31 = (reg & (1L << 31)) >> 31;
  b29 = (reg & (1L << 29)) >> 29;
  b25 = (reg & (1L << 25)) >> 25;
  b24 = (reg & (1L << 24)) >> 24;
  lobit = b31 ^ b29 ^ b25 ^ b24;
  newr = (reg << 1) | lobit;
  reg = newr;



//  OCR0B=(amp*(reg&255))>>15;
//  OCR0B=(amp*((reg&255)+(sinetable[cnt]*toneamp>>10)))>>15;
  OCR0B=(amp*((reg&255)+(sinetable[(cnt>>8)&1023]*toneamp>>10)))>>15;
  cnt+=tonefreq;
}

uint8_t parambrake=0;
ISR(TIMER2_COMPA_vect) {
  sei();                    // re-enable interrups directly, so the sound interrupt is ensured to run

  parambrake++;
  if (parambrake==8){
    parambrake=0;
    tonefreq=((int32_t)analogRead(A0))*5+3000;
    snappiness=((int32_t)analogRead(A1))/4;
    toneamp=((int32_t)analogRead(A2));
    ampdesc=(525-((int32_t)analogRead(A3))/2)*(400-snappiness)/400;
    accent=analogRead(A4)<<5;
  }
//accent=10000;
  samp-=ampdesc;
  if (samp<0)samp=0;

  uint16_t trigger=analogRead(A7);
  if (trigger<10)oldtrigger=false;
  if (!oldtrigger){
    if (trigger>10){
      if (trigger>200){
        samp+=32767;
      } else {
        samp+=accent;
      }
      if (samp>32767)samp=32767;
      oldtrigger=true;
    }
  }

  uint32_t famp=((uint32_t)samp)>>7;
  famp=(famp*famp);         // pow of 8
  famp=(famp*famp)>>16;
  famp=(famp*famp);
  amp=((famp>>17)*snappiness+samp*(255-snappiness))>>8;
  if (amp>32767)amp=32767;
}

void loop(){
}
