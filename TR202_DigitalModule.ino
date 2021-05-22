#if defined(__AVR_ATmega328P__)
// Timer2 is the same on the mega328 and mega168
#define __AVR_ATmega168__
#endif

#include "Arduino.h"

static uint16_t cnt;
const uint8_t sinetable[256] ={127,130,133,136,139,142,145,148,151,154,157,160,163,166,169,172,
                          175,178,181,184,186,189,192,194,197,200,202,205,207,209,212,214,
                          216,218,221,223,225,227,229,230,232,234,235,237,239,240,241,243,
                          244,245,246,247,248,249,250,250,251,252,252,253,253,253,253,253,
                          254,253,253,253,253,253,252,252,251,250,250,249,248,247,246,245,
                          244,243,241,240,239,237,235,234,232,230,229,227,225,223,221,218,
                          216,214,212,209,207,205,202,200,197,194,192,189,186,184,181,178,
                          175,172,169,166,163,160,157,154,151,148,145,142,139,136,133,130,
                          127,123,120,117,114,111,108,105,102,99,96,93,90,87,84,81,
                          78,75,72,69,67,64,61,59,56,53,51,48,46,44,41,39,
                          37,35,32,30,28,26,24,23,21,19,18,16,14,13,12,10,
                          9,8,7,6,5,4,3,3,2,1,1,0,0,0,0,0,
                          0,0,0,0,0,0,1,1,2,3,3,4,5,6,7,8,
                          9,10,12,13,14,16,18,19,21,23,24,26,28,30,32,35,
                          37,39,41,44,46,48,51,53,56,59,61,64,67,69,72,75,
                          78,81,84,87,90,93,96,99,102,105,108,111,114,117,120,123};

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
}

ISR(TIMER1_COMPA_vect){//timer 1 interrupt
      OCR0B=sinetable[cnt>>8];
}

/*
********************************
Main loop which runs all the controls, display, sequencer and value stuff
This is called every 5ms to ensure to catch the sync signal and have some proper sequencer timing
The remaining time is used for displaying the oscilloscope display (see loop())
********************************
*/
ISR(TIMER2_COMPA_vect) {
  sei();                    // re-enable interrups directly, so the sound interrupt is ensured to run

}


void loop(){
}
