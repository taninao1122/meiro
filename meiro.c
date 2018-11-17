/*
制作者情報:1614266 3EP1-07 大谷直也
作成日:2018/11/12
動かし方：スイッチ操作でスタートからゴールまで行く
*/

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <stdbool.h>

#define user_CTOP 50000UL
#define CTOP 10000UL



volatile unsigned char map[8] =
  {
 //     0b10111111,
      0b10010000,
	  0b10011111,
      0b11000001,
      0b11111101,
      0b10000001,
      0b10111111,
      0b10000000,
      0b11111110
    };
volatile unsigned char stat;
volatile unsigned char sw;		
volatile unsigned char sw_flag;

volatile unsigned char mv_flag;
//volatile unsigned char period;
volatile unsigned int period;

static unsigned char scan = 0;
unsigned char my_state = 0;
unsigned char x = 0x40;
unsigned char x_sub = 0;
unsigned char smog_b = 0xE0;
bool sw_flag2 = false;
void update_led();

ISR(PCINT1_vect)
{
	stat = 1;
}

ISR(TIMER0_COMPA_vect)
{
	update_led();	
}

ISR(TIMER2_COMPA_vect)
{
	PORTD  ^= _BV(PORTD3);
	if(sw_flag2 == true){
		if(period != 0){
			period --;
			if(period == 0){
				sw_flag2 = false;
				DDRD = 0xF6;
			//	OCR2A = 0;
			}
		}
	}
	else{
		OCR2A = 0;
	}

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

void update_led()
{
	static unsigned char sc = 0xFE;
	
	PORTB = 0;
	sc = (sc << 1) | (sc >> 7);
	PORTD = (PORTD & 0x0F) | (sc & 0xF0);	          
	PORTC = (PORTC & 0xF0) | (sc & 0x0F);	          
	scan = (scan + 1) & 7;
	//PORTB = map[scan]; //ﾃﾞﾊﾞｯｸﾞ用
   /* 
	if(scan == my_state){
        PORTB |= x; //プレイヤーの表示
    }
*/
//霧の発生	
	if(my_state != 0){

		if(scan == my_state ){
			PORTB = map[scan] & smog_b;
			PORTB |= x;
		}else if(scan == (my_state + 1)){
				PORTB = map[scan] & smog_b;
		}else if(scan == (my_state - 1)){
			PORTB = map[scan] & smog_b;            
		}

	}
	else{
		if(scan == my_state ){
			PORTB = map[scan] & smog_b;
			PORTB |= x;
		}else if(scan == (my_state + 1)){
			PORTB = map[scan] & smog_b;
		}
	}
//霧の発生終了
}

int main()
{
	unsigned long user_cnt = 0;
//	unsigned long user_cnt2 = 0;
	bool user_b = false;

	DDRB = 0xFF;
	DDRC = 0x0F;
	DDRD = 0xF6;

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

	TCCR2A = 02;
	TCCR2B = 0x04;
	TIMSK2 = _BV(OCIE2A);
	OCR2A = 0;
	OCR2B = 0;


	sei();
	for (;;) {
		wdt_reset();
        update_sw();
		user_cnt ++;
		
		
		if(user_cnt >= user_CTOP){
			user_cnt = 0;
			if(user_b == false){
				x_sub = x;
				x = x & 0;
				user_b = true;
			}else{
				x = x_sub;
				user_b = false;
			}
			
			 
		}	
        if (sw_flag) {
			sw_flag = 0;
			DDRD = 0xFE;
			switch (sw) {	
				case 0:
					break;
				case 1:
					sw_flag2 = true;
					if((map[my_state] & x) == 0){
						x = (x >> 7) | (x << 1);
						smog_b = (smog_b >> 7) | (smog_b << 1);
						period = 1000;
						OCR2A = 62;
					}
					else{
						x = (x << 7) | (x >> 1);
						smog_b = (smog_b << 7) | (smog_b >> 1);
						period = 1000;
						OCR2A = 238;
					}
					
					break;
				case 2:
					sw_flag2 = true;
					if((map[my_state] & x) == 0)
					{
						//printf("%d:\n",PORTB);
						x = (x << 7) | (x >> 1);
						smog_b = (smog_b << 7) | (smog_b >> 1);
						period = 1000;
						OCR2A = 62;
							
					}else{
						x = (x >> 7) | (x << 1);
						smog_b = (smog_b >> 7) | (smog_b << 1);						
						period = 1000;
						OCR2A = 238;
						
					}
					break;
				case 3:
					sw_flag2 = true;
					my_state = (my_state + 1) & 7;			
					if((map[my_state] & x) == 0){
						period = 1000;
						OCR2A = 62;
					}
					else{
						my_state = (my_state - 1) & 7;
						period = 1000;
						OCR2A = 238;	
					}
					break;				
			}
		} 
	}
	return 0;
}
