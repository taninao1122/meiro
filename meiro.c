/*
制作者情報:1614266 3EP1-07 大谷直也
作成日:2018/11/12
動かし方：スイッチ操作でスタートからゴールまで行く
*/

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>

#define CTOP 10000UL;
#define BZ_CTOP 1000UL
#define BZ_CTOP2 500UL
#define BZ_CTOP3  100UL


volatile unsigned char map[8] =
  {
 //     0b10111111,
      0b00000000,
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

unsigned char my_state = 0;
static unsigned char scan = 0;
unsigned char x = 0x40;
unsigned char smog_b = 0xE0;

void update_led();

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
			sw = ~(PINC >> 4) & 3;	
			sw_flag = 1;
			stat = 0;
		}
		return;
	}
}

void proc_bz1()
{
    static unsigned long cnt = 0;
    cnt ++;
    if(cnt < BZ_CTOP){
        return;
    }
    cnt = 0;
    PORTD ^=0x08;
}

void proc_bz2()
{
    static unsigned long cnt2 = 0;
    cnt2 ++;
    if(cnt2 < BZ_CTOP2){
        return;
    }
    cnt2 = 0;
    PORTD ^=0x08;
}

void proc_bz3()
{
    static unsigned long cnt3 = 0;
    cnt3 ++;
    if(cnt3 < BZ_CTOP3){
        return;
    }
    cnt3 = 0;
    PORTD ^=0x08;
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
    if(scan == my_state){
        PORTB |= x;
    }	
/*	if(my_state != 0)	{

        if(scan == my_state ){
			//PORTB = map[scan];
		    PORTB = map[scan] & smog_b;
		}else if(scan == (my_state + 1)){
              PORTB = map[scan] & smog_b;
        }else if(scan == (my_state - 1)){
            PORTB = map[scan] & smog_b;            
        }
		
	 }else{
        if(scan == my_state ){
			//PORTB = map[scan];
		    PORTB = map[scan] & smog_b;
		}else if(scan == (my_state + 1)){
              PORTB = map[scan] & smog_b;
        }
    }
*/
}

int main()
{
//	unsigned char n;
//    unsigned char n2;

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

/*			if(scan == my_state){
				if(my_state < 4){
					//PORTC =					
				}else{
					//PORTD = 
				}
					 		
			}
*/
			switch (sw) {
				case 0:
					break;
				case 1:
					x = (x >> 7) | (x << 1);
					if((map[my_state] & x) == 0){
						smog_b = (smog_b >> 7) | (smog_b << 1);
						proc_bz1();	
					}
					else{
						x = (x << 7) | (x >> 1);
						//proc_bz();	
					}
					
					break;
				case 2:
					x = (x << 7) | (x >> 1);
					if((map[my_state] & x) == 0){
						smog_b = (smog_b << 7) | (smog_b >> 1);
						proc_bz2();	
					}else{
						x = (x >> 7) | (x << 1);
						//proc_bz();
					}
					break;
				case 3:
					my_state = (my_state + 1) & 7;
					if((map[my_state] & x) == 0){
						proc_bz3;
					}
					else{
						my_state = (my_state - 1) & 7;
						//proc_bz();	
					}
					break;
			}
			

		}      
		if (mv_flag == 1) {
			mv_flag = 0;
		}
	}
	return 0;
}
