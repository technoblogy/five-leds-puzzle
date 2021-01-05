/* Five LEDs Puzzle v3 - see http://www.technoblogy.com/show?3D8S

   David Johnson-Davies - www.technoblogy.com - 5th January 2021
   ATtiny85 @ 1 MHz (internal oscillator; BOD disabled)
   
   CC BY 4.0
   Licensed under a Creative Commons Attribution 4.0 International license: 
   http://creativecommons.org/licenses/by/4.0/
*/

#include <avr/sleep.h>

// Pin change interrupt is just used to wake us up
ISR (PCINT0_vect) {
}

void setup () {
  PORTB = 0;
  DDRB = 0;
  // Set up pin change interrupts for buttons
  PCMSK = 1<<PINB0 | 1<<PINB1 | 1<<PINB2 | 1<<PINB3 | 1<<PINB4;
  // Disable ADC to save power
  ADCSRA &= ~(1<<ADEN);
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
}

const unsigned long Timeout = 30000; // 30 seconds

void loop () {
  unsigned long Start = millis();
  while (millis() - Start < Timeout) {
    for (int b=0; b<5; b++) {
      int d = DDRB;
      DDRB = d & ~(1<<b);
      delay(1);
      if (!(PINB & 1<<b)) {
        while (!(PINB & 1<<b));
        DDRB = d ^ ((!b || (d & ((1<<b)-1)) == 1<<(b-1))<<b);
        Start = millis();
      } else DDRB = d;
      delay(10);
    }
  }
  // Achieve lowest possible power in sleep
  int timsk = TIMSK;
  TIMSK = 0;                          // Disable timer interrupts
  int d = DDRB;
  DDRB = 0;                           // All lights off
  PORTB = 0x1F;                       // Pullups on
  GIFR = 1<<PCIF;                     // Clear flag
  GIMSK = GIMSK | 1<<PCIE;            // Enable pin change interrupt
  sleep_enable();
  sleep_cpu();                        // Go to sleep
  GIMSK = GIMSK | 1<<PCIE;            // Disable pin change interrupt
  while ((PINB & 0x1F) != 0x1F);      // Wait for all buttons released
  TIMSK = timsk;                      // Re-enable timer interrupts
  PORTB = 0;                          // Pullups off
  DDRB = d;                           // Restore lights
  delay(500);           
}
