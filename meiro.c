#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>

#define CTOP 10000UL;

volatile unsigned char map[8] =
  {
      0b10111111,
      0b10011111,
      0b11000000,
      0b11111110,
      0b10000000,
      0b10111111,
      0b10000000,
      0b11111110
    };
volatile unsigned char stat;
volatile unsigned char sw;		
volatile unsigned char sw_flag;
volatile unsigned char mv_flag;

void update_led();

static unsigned char scan = 0;
unsigned char x = 0x00;


ISR(PCINT1_vect)
{
	stat = 1;
}

void update_sw()
{
	static unsigned long cnt;
	switch (stat) {
	case 0:
		return;
	case 1:
		cnt = CTOP;
		stat = 2;
		return;
	case 2:
		cnt--;
		if (cnt == 0) {
			sw = ~(PINC >> 4) & 3;	// �ϐ�sw��X�V
			sw_flag = 1;	// �t���O�𗧂Ă�
			stat = 0;
		}
		return;
	}
}

ISR(TIMER0_COMPA_vect)
{
	static int cnt;
	cnt++;
	if (cnt == 100) {
		cnt = 0;
		mv_flag = 1;	
	}
	update_led();	
}
void update_led()
{
	static unsigned char sc = 0xFE;
	
	PORTB = 0;	
	sc = (sc << 1) | (sc >> 7);
	PORTD = (PORTD & 0x0F) | (sc & 0xF0);	          
	PORTC = (PORTC & 0xF0) | (sc & 0x0F);	          
	scan = (scan + 1) & 7;
	PORTB = map[scan];
}

int main()
{
	unsigned char n;
    unsigned char n2;

	DDRB = 0xFF;
	DDRC = 0x0F;
	DDRD = 0xFE;

	PORTB = 0x00;
	PORTC = 0x30;
	PORTD = 0x00;
    PCICR = _BV(PCIE1);
    PCMSK1 = 0x30;

	TCNT0 = 0;
	OCR0A = 249;
	TCCR0A = 2;
	TCCR0B = 3;
	TIMSK0 |= _BV(OCIE0A);	

	sei();
	for (;;) {
		wdt_reset();
        update_sw();
        
        if (sw_flag) {
			sw_flag = 0;	
			switch (sw) {
			case 0:
				break;
			case 1:
				x = (x >> 7) | (x << 1);
				break;
	        case 2:
				x = (x << 7) | (x >> 1);
				break;
			case 3:
				break;
			}
		}
        
		if (mv_flag == 1) {
			mv_flag = 0;
     /*    for (n = 7; n > 0; n--) {
                map[n] = (map[n] << 1) | (map[n] >> 7);
                n2 = (n-1) & 7;
                map[n] = map[n2]; 						
            }
            map[0] = map[7];
*/
		}
	}
	return 0;
}
