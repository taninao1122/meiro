/*********************************
情報工学専門実験　演習A(マイコンプログラミング)
課題番号:   第１週予習ノート　設問2
提出日:    平成28年10月09日
学籍番号:   1614266
クラス-番号: 3EP1-07
氏名: 大谷直也

***********************************/
#include<avr/io.h>
#include<avr/wdt.h>

unsigned char sw;
unsigned char sw_flag;
#define BZ_CTOP 1000UL
#define BZ_CTOP2  100UL

#define CTOP 10000UL


void update_sw(){
    sw = (~PINC >> 4) & 3;
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

int main(){
    unsigned long cnt = 0;
    unsigned char scan = 0;
    unsigned char scan2 = 0;
    
    DDRB = 0xFF;
    DDRC = 0x0F;
    DDRD = 0xFE;

    PORTB = 0xFF;
    PORTC = 0x30;
    PORTD = 0x08;

    for(;;){
        wdt_reset();
        cnt++;
        update_sw();
        sw_flag = 0;        
        switch(sw){
            case 0:
               PORTB = 0x00;
               break;
            case 1:
               proc_bz1();
               if(cnt >= CTOP){
                   cnt = 0;
                   scan = (scan + 1) & 7;
                   PORTB = 1 << scan;                    
                }                
                break;
            case 2:
                proc_bz2();
                if(cnt >= CTOP){
                    cnt = 0;
                    scan2 = (scan2 + 1) & 7;
                    PORTB = 0x80 >> scan2;
                    if(PORTC & 0x10){
                        proc_bz2();
                   } 
                }                
                break; 
            case 3:
               PORTB = 0xFF;
               break;          
            }
        }
   
    return 0;
}



