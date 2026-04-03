
#include "device_registers.h"           // Device header
#include "clocks_and_modes.h"
#include "lcd1602A.h"
#include "stdlib.h"
#include "stdio.h"
#define NUM_OF_STATES 8
int lpit0_ch0_flag_counter = 0; /*< LPIT0 timeout counter */
int lpit0_ch1_flag_counter = 0; /*< LPIT0 timeout counter */
int lpit0_ch3_flag_counter = 0;
unsigned int External_PIN=0; /* External_PIN:SW External input Assignment */
char value[10];

char data=0;
unsigned int num,num0,num1,num2,num3 =0;
unsigned int k1,k2,k3,k4,k5 =0;
unsigned int a1,a2,a3,a4;
unsigned int i=0;
unsigned int Delaytime = 0; /* Delay Time Setting Variable*/
int key=0;
int sum=0;
int expin=0;
int prekey=50;
unsigned int FND_SEL[4]={0x0100,0x0200,0x0400,0x0800};
unsigned int j=0; /*FND select pin index */
unsigned int Dtime = 0; /* Delay Time Setting Variable*/
char state_array[NUM_OF_STATES] = {0x06, 0x02, 0x0A, 0x08, 0x09, 0x01, 0x05, 0x04};

void WDOG_disable (void)
{
  WDOG->CNT=0xD928C520;     /* Unlock watchdog       */
  WDOG->TOVAL=0x0000FFFF;   /* Maximum timeout value    */
  WDOG->CS = 0x00002100;    /* Disable watchdog       */
}



void LPIT0_init (uint32_t delay)
{
   uint32_t timeout;

	/*!
	    * LPIT Clocking:
	    * ==============================
	    */
	  PCC->PCCn[PCC_LPIT_INDEX] = PCC_PCCn_PCS(6);    /* Clock Src = 6 (SPLL2_DIV2_CLK)*/
	  PCC->PCCn[PCC_LPIT_INDEX] |= PCC_PCCn_CGC_MASK; /* Enable clk to LPIT0 regs       */

	  /*!
	   * LPIT Initialization:
	   */
	  LPIT0->MCR |= LPIT_MCR_M_CEN_MASK;  /* DBG_EN-0: Timer chans stop in Debug mode */
	                                        /* DOZE_EN=0: Timer chans are stopped in DOZE mode */
	                                        /* SW_RST=0: SW reset does not reset timer chans, regs */
	                                        /* M_CEN=1: enable module clk (allows writing other LPIT0 regs) */

  timeout=delay* 40;
  LPIT0->TMR[0].TVAL = timeout;      /* Chan 0 Timeout period: 40M clocks */
  LPIT0->TMR[0].TCTRL |= LPIT_TMR_TCTRL_T_EN_MASK;
                                     /* T_EN=1: Timer channel is enabled */
                              /* CHAIN=0: channel chaining is disabled */
                              /* MODE=0: 32 periodic counter mode */
                              /* TSOT=0: Timer decrements immediately based on restart */
                              /* TSOI=0: Timer does not stop after timeout */
                              /* TROT=0 Timer will not reload on trigger */
                              /* TRG_SRC=0: External trigger soruce */
                              /* TRG_SEL=0: Timer chan 0 trigger source is selected*/
}


void delay_us (volatile int us){
   LPIT0_init(us);           /* Initialize PIT0 for 1 second timeout  */
   while (0 == (LPIT0->MSR & LPIT_MSR_TIF0_MASK)) {} /* Wait for LPIT0 CH0 Flag */
               lpit0_ch0_flag_counter++;         /* Increment LPIT0 timeout counter */
               LPIT0->MSR |= LPIT_MSR_TIF0_MASK; /* Clear LPIT0 timer flag 0 */
}

void NVIC_init_IRQs(void){
	S32_NVIC->ICPR[1] |= 1<<(59%32); // Clear any pending IRQ59
	S32_NVIC->ISER[1] |= 1<<(59%32); // Enable IRQ59
	S32_NVIC->IP[59] =0x0; //Priority 11 of 15
	
	S32_NVIC->ICPR[1] |= 1<<(61%32); // Clear any pending IRQ59
	S32_NVIC->ISER[1] |= 1<<(61%32); // Enable IRQ59
	S32_NVIC->IP[61] =0xB; //Priority 10 of 15
	
	S32_NVIC->ICPR[1] |= 1<<(60%32); // Clear any pending IRQ59
	S32_NVIC->ISER[1] |= 1<<(60%32); // Enable IRQ59
	S32_NVIC->IP[60] =0xB; //Priority 10 of 15
}

void PORTA_IRQHandler(void){

	
	//PORTA_Interrupt State Flag Register Read
	if((PORTA->ISFR & (1<<10)) != 0){
		External_PIN=1;
	}
	else if((PORTA->ISFR & (1<<11)) != 0){
		External_PIN=2;
	}
	else if((PORTA->ISFR & (1<<12)) != 0){
		External_PIN=3;
	}
		else if((PORTA->ISFR & (1<<13)) != 0){
		External_PIN=4;
		}
else if((PORTA->ISFR & (1<<14)) != 0){
		External_PIN=5;
		}
	

	// External input Check Behavior Assignment
	switch (External_PIN){
	
		
		
		case 2:		
			k2 = 1;		
		  External_PIN=0;
			break;
		case 3:
			k3 = 1;
		  External_PIN=0;
			break;
case 4:
			k4 = 1;
		  External_PIN=0;
			break;
case 5:
			k5 = 1;
		  External_PIN=0;
			break;
		default:
			break;
	}
  PORTA->PCR[10] |= 0x01000000; // Port Control Register ISF bit '1' set
	PORTA->PCR[11] |= 0x01000000; // Port Control Register ISF bit '1' set
	PORTA->PCR[12] |= 0x01000000; // Port Control Register ISF bit '1' set
	PORTA->PCR[13] |= 0x01000000; // Port Control Register ISF bit '1' set
	PORTA->PCR[14] |= 0x01000000; // Port Control Register ISF bit '1' set

}

void PORTC_IRQHandler(void){

	

	//PORTC_Interrupt State Flag Register Read
	if((PORTC->ISFR & (1<<16)) != 0){
		External_PIN=1;
	}

	// External input Check Behavior Assignment
	switch (External_PIN){
		case 1:
			k1= 1;
			External_PIN=0;
			break;
		default:
			break;
	}

	PORTC->PCR[16] |= 0x01000000; // Port Control Register ISF bit '1' set
}

	
	



void PORT_init (void)
{

	
	
	
	PTD->PDDR |= 1<<9 | 1<<10 | 1<<11 | 1<<12 | 1<<13 | 1<<14 | 1<<8;
	PTD->PDDR |= 1<<0	| 1<<15 | 1<<16;
     PCC->PCCn[PCC_PORTD_INDEX] &= ~PCC_PCCn_CGC_MASK;
     PCC->PCCn[PCC_PORTD_INDEX] |= PCC_PCCn_PCS(0x001);
     PCC->PCCn[PCC_PORTD_INDEX] |= PCC_PCCn_CGC_MASK;
     PCC->PCCn[PCC_FTM2_INDEX]  &= ~PCC_PCCn_CGC_MASK;
     PCC->PCCn[PCC_FTM2_INDEX]  |= (PCC_PCCn_PCS(1)| PCC_PCCn_CGC_MASK);		//Clock = 80MHz
         //Pin mux
     PORTD->PCR[9]= PORT_PCR_MUX(1);
     PORTD->PCR[10]= PORT_PCR_MUX(1);
     PORTD->PCR[11]= PORT_PCR_MUX(1);
     PORTD->PCR[12]= PORT_PCR_MUX(1);
     PORTD->PCR[13]= PORT_PCR_MUX(1);
     PORTD->PCR[14]= PORT_PCR_MUX(1);
     PORTD->PCR[8]= PORT_PCR_MUX(1);
	PORTD->PCR[0]= PORT_PCR_MUX(1);
     PORTD->PCR[15]= PORT_PCR_MUX(1);
     PORTD->PCR[16]= PORT_PCR_MUX(1);
	
	PCC->PCCn[PCC_PORTA_INDEX]|=PCC_PCCn_CGC_MASK;

	PORTA->PCR[10] |= PORT_PCR_MUX(1); // Port A11 mux = GPIO
	PORTA->PCR[10] |=(10<<16); // Port A11 IRQC : interrupt on Falling-edge
	PORTA->PCR[11] |= PORT_PCR_MUX(1); // Port A11 mux = GPIO
	PORTA->PCR[11] |=(10<<16); // Port A11 IRQC : interrupt on Falling-edge
	PORTA->PCR[12] |= PORT_PCR_MUX(1); // Port A12 mux = GPIO
	PORTA->PCR[12] |=(10<<16); // Port A12 IRQC : interrupt on Falling-edge
	PORTA->PCR[13] |= PORT_PCR_MUX(1); // Port A13 mux = GPIO
	PORTA->PCR[13] |=(10<<16); // Port A12 IRQC : interrupt on Falling-edge
	PORTA->PCR[14] |= PORT_PCR_MUX(1); // Port A13 mux = GPIO
	PORTA->PCR[14] |=(10<<16); // Port A12 IRQC : interrupt on Falling-edge
	
	
	//B
	PTB->PDDR|=(1<<1);
	
		PCC->PCCn[PCC_PORTB_INDEX]|=PCC_PCCn_CGC_MASK;

	PORTB->PCR[1] |= PORT_PCR_MUX(1); 
	PORTB->PCR[1] |=(10<<16); 

	PTB->PDDR|=(1<<1);
	
	
	/* Enable clock for PORT E */
	PCC-> PCCn[PCC_PORTE_INDEX] = PCC_PCCn_CGC_MASK; 

	  PTE->PDDR |= 1<<12|1<<14|1<<15;		/* Port E12,E14-E15:  Data Direction = output */
	  PTE->PDDR &= ~(1<<0);   /* Port E0: Data Direction= input (default) */
	  PTE->PDDR &= ~(1<<1);   /* Port E1: Data Direction= input (default) */
	  PTE->PDDR &= ~(1<<2);   /* Port E2: Data Direction= input (default) */
	  PTE->PDDR &= ~(1<<3);   /* Port E3: Data Direction= input (default) */

	  PORTE->PCR[0] = PORT_PCR_MUX(1)|PORT_PCR_PFE(1)|PORT_PCR_PE(1)|PORT_PCR_PS(0); /* Port E0: MUX = GPIO, input filter enabled, internal pull-down enabled */
	  PORTE->PCR[1] = PORT_PCR_MUX(1)|PORT_PCR_PFE(1)|PORT_PCR_PE(1)|PORT_PCR_PS(0); /* Port E1: MUX = GPIO, input filter enabled, internal pull-down enabled */
	  PORTE->PCR[2] = PORT_PCR_MUX(1)|PORT_PCR_PFE(1)|PORT_PCR_PE(1)|PORT_PCR_PS(0); /* Port E2: MUX = GPIO, input filter enabled, internal pull-down enabled */
	  PORTE->PCR[3] = PORT_PCR_MUX(1)|PORT_PCR_PFE(1)|PORT_PCR_PE(1)|PORT_PCR_PS(0); /* Port E3: MUX = GPIO, input filter enabled, internal pull-down enabled */
PORTE->PCR[0] |=(9<<16);
PORTE->PCR[1] |=(9<<16);
PORTE->PCR[2] |=(9<<16);
PORTE->PCR[3] |=(9<<16);

	  PORTE->PCR[12]  = PORT_PCR_MUX(1);	/* Port E12: MUX = GPIO  */
	  PORTE->PCR[14]  = PORT_PCR_MUX(1);	/* Port E14: MUX = GPIO  */
	  PORTE->PCR[15]  = PORT_PCR_MUX(1);	/* Port E15: MUX = GPIO  */

	PCC-> PCCn[PCC_PORTC_INDEX] = PCC_PCCn_CGC_MASK; /* Enable clock for PORT C */

	  PTC->PDDR |= (1<<1);   /* Port B1: Data Direction= input (default) */
	  PTC->PDDR |= (1<<2);   /* Port B2: Data Direction= input (default) */
	  PTC->PDDR |= (1<<3);   /* Port B3: Data Direction= input (default) */
	  PTC->PDDR |= (1<<15);   /* Port B4: Data Direction= input (default) */
	  PTC->PDDR |= (1<<12);   /* Port B5: Data Direction= input (default) */
	  PTC->PDDR |= (1<<13);   /* Port B6: Data Direction= input (default) */
	  PTC->PDDR |= (1<<7);   /* Port B7: Data Direction= input (default) */
  PTC->PDDR |= 1<<8|1<<9|1<<10|1<<11;
	
	
PTC->PDDR |= 1<<16;
PORTC->PCR[16] = PORT_PCR_MUX(1)|PORT_PCR_PFE(1)|PORT_PCR_PE(1)|PORT_PCR_PS(0);
PORTC->PCR[16] |=(10<<16);

	  PORTC->PCR[1] = PORT_PCR_MUX(1); /* Port B1: MUX = GPIO */
	  PORTC->PCR[2] = PORT_PCR_MUX(1); /* Port B2: MUX = GPIO */
	  PORTC->PCR[3] = PORT_PCR_MUX(1); /* Port B3: MUX = GPIO */
	  PORTC->PCR[15] = PORT_PCR_MUX(1); /* Port B4: MUX = GPIO */
	 
	  PORTC->PCR[13] = PORT_PCR_MUX(1); /* Port B6: MUX = GPIO */
	  PORTC->PCR[7] = PORT_PCR_MUX(1); /* Port B7: MUX = GPIO */
		PORTC->PCR[8] = PORT_PCR_MUX(1); /* Port B5: MUX = GPIO */
	  PORTC->PCR[9] = PORT_PCR_MUX(1); /* Port B6: MUX = GPIO */
	  PORTC->PCR[10] = PORT_PCR_MUX(1); /* Port B7: MUX = GPIO */
	  PORTC->PCR[11] = PORT_PCR_MUX(1); /* Port B7: MUX = GPIO */
		 PORTC->PCR[12] = PORT_PCR_MUX(1); /* Port B5: MUX = GPIO */

	
	  

}




void text_lcd(char *mess){
   while(mess[i] != '\0'){
            if(i%16 == 0){
                lcdinput(0x80+0x40);
                if(i%32==0){
                    lcdinit();
                }
            	delay_us(500);
            }
            lcdcharinput(mess[i]); // 1(first) row text-char send to LCD module
            delay_us(200);
			i++;
		}
}

void seg (int nom){
	/*
	 	   A
		?????
	      ?	    ?
	 F ?	    ?B
	      ?    G  ?
	      ??????
	      ?	    ?
	  E?	    ?C
	      ?	    ?
	      ??????    ? DOT
		   D
	*/
   switch(nom){
   case 0:
         PTC-> PSOR |= 1<<1;//PTD1; // FND A ON
         PTC-> PSOR |= 1<<2;//PTD2; // FND B ON
         PTC-> PSOR |= 1<<3;//PTD3; // FND C ON
         PTC-> PSOR |= 1<<12;//PTD4; // FND D ON
         PTC-> PSOR |= 1<<13;//PTD5; // FND E ON
         PTC-> PSOR |= 1<<15;//PTD6; // FND F ON
         PTC-> PCOR |= 1<<7;//PTD7; // 
         break;
      case 1:
         PTC-> PCOR |= 1<<1;//PTD1; //
         PTC-> PSOR |= 1<<2;//PTD2; // FND B ON
         PTC-> PSOR |= 1<<3;//PTD3; // FND C ON
         PTC-> PCOR |= 1<<12;//PTD4; //
         PTC-> PCOR |= 1<<13;//PTD5; //
         PTC-> PCOR |= 1<<15;//PTD6; //
         PTC-> PCOR |= 1<<7;//PTD7; //
         break;
      case 2:
         PTC-> PSOR |= 1<<1;//PTD1; // FND A ON
         PTC-> PSOR |= 1<<2;//PTD2; // FND B ON
         PTC-> PCOR |= 1<<3;//PTD3; //
         PTC-> PSOR |= 1<<12;//PTD4; // FND D ON
         PTC-> PSOR |= 1<<13;//PTD5; // FND E ON
         PTC-> PCOR |= 1<<15;//PTD6; //
         PTC-> PSOR |= 1<<7;//PTD7; // FND G ON
         break;
      case 3:
         PTC-> PSOR |= 1<<1;//PTD1; // FND A ON
         PTC-> PSOR |= 1<<2;//PTD2; // FND B ON
         PTC-> PSOR |= 1<<3;//PTD3; // FND C ON
         PTC-> PSOR |= 1<<12;//PTD4; // FND D ON
         PTC-> PCOR |= 1<<13;//PTD5; //
         PTC-> PCOR |= 1<<15;//PTD6; //
         PTC-> PSOR |= 1<<7;//PTD7; // FND G ON
         break;
      case 4:
         PTC-> PCOR |= 1<<1;//PTD1; //
         PTC-> PSOR |= 1<<2;//PTD2; // FND B ON
         PTC-> PSOR |= 1<<3;//PTD3; // FND C ON
         PTC-> PCOR |= 1<<12;//PTD4; //
         PTC-> PCOR |= 1<<13;//PTD5; //
         PTC-> PSOR |= 1<<15;//PTD6; // FND F ON
         PTC-> PSOR |= 1<<7;//PTD7; // FND G ON
         break;
      case 5:
         PTC-> PSOR |= 1<<1;//PTD1; // FND A ON
         PTC-> PCOR |= 1<<2;//PTD2; // 
         PTC-> PSOR |= 1<<3;//PTD3; // FND C ON
         PTC-> PSOR |= 1<<12;//PTD4; // FND D ON
         PTC-> PCOR |= 1<<13;//PTD5; //
         PTC-> PSOR |= 1<<15;//PTD6; // FND F ON
         PTC-> PSOR |= 1<<7;//PTD7; // FND G ON
         break;
      case 6:
         PTC-> PSOR |= 1<<1;//PTD1; // FND A ON
         PTC-> PCOR |= 1<<2;//PTD2; // 
         PTC-> PSOR |= 1<<3;//PTD3; // FND C ON
         PTC-> PSOR |= 1<<12;//PTD4; // FND D ON
         PTC-> PSOR |= 1<<13;//PTD5; // FND E ON
         PTC-> PSOR |= 1<<15;//PTD6; // FND F ON
         PTC-> PSOR |= 1<<7;//PTD7; // FND G ON
         break;
      case 7:
         PTC-> PSOR |= 1<<1;//PTD1; // FND A ON
         PTC-> PSOR |= 1<<2;//PTD2; // FND B ON
         PTC-> PSOR |= 1<<3;//PTD3; // FND C ON
         PTC-> PCOR |= 1<<12;//PTD4; //
         PTC-> PCOR |= 1<<13;//PTD5; //
         PTC-> PSOR |= 1<<15;//PTD6; // FND F ON
         PTC-> PCOR |= 1<<7;//PTD7; //
         break;
      case 8:
         PTC-> PSOR |= 1<<1;//PTD1; // FND A ON
         PTC-> PSOR |= 1<<2;//PTD2; // FND B ON
         PTC-> PSOR |= 1<<3;//PTD3; // FND C ON
         PTC-> PSOR |= 1<<12;//PTD4; // FND D ON
         PTC-> PSOR |= 1<<13;//PTD5; // FND E ON
         PTC-> PSOR |= 1<<15;//PTD6; // FND F ON
         PTC-> PSOR |= 1<<7;//PTD7; // FND G ON
         break;
      case 9:
         PTC-> PSOR |= 1<<1;//PTD1; // FND A ON
         PTC-> PSOR |= 1<<2;//PTD2; // FND B ON
         PTC-> PSOR |= 1<<3;//PTD3; // FND C ON
         PTC-> PSOR |= 1<<12;//PTD4; // FND D ON
         PTC-> PCOR |= 1<<13;//PTD5; //
         PTC-> PSOR |= 1<<15;//PTD6; // FND F ON
         PTC-> PSOR |= 1<<7;//PTD7; // FND G ON
         break;

   }

}

void Seg_out(int number)
{
	Dtime = 1000;

	num3=(number/1000)%10;
	num2=(number/100)%10;
	num1=(number/10)%10;
	num0= number%10;


	// 1000??? ??
	PTC->PSOR = FND_SEL[j];
	PTC->PCOR =0x7f;
	seg(num3);
  delay_us(Dtime);
	PTC->PCOR = 0xfff;
	j++;

	// 100??? ??
	PTC->PSOR = FND_SEL[j];
	PTC->PCOR =0x7f;
	seg(num2);
  delay_us(Dtime);
	PTC->PCOR = 0xfff;
	j++;

	// 10??? ??
	PTC->PSOR = FND_SEL[j];
	PTC->PCOR =0x7f;
	seg(num1);
  delay_us(Dtime);
  PTC->PCOR = 0xfff;
	j++;

	// 1??? ??
	PTC->PSOR = FND_SEL[j];
	PTC->PCOR =0x7f;
	seg(num0);
  delay_us(Dtime);
	PTC->PCOR = 0xfff;
	j=0;
}
int Kbuff = 0;

int KeyScan(void)
{
	 
 
   Dtime = 1000;
 
   PTE->PSOR |=1<<12;
   delay_us(Dtime);
   if(PTE->PDIR &(1<<0))Kbuff=1;      //1
   if(PTE->PDIR &(1<<1))Kbuff=4;      //4
   if(PTE->PDIR &(1<<2))Kbuff=7;      //7
   if(PTE->PDIR &(1<<3))Kbuff=11;     //*
   PTE->PCOR |=1<<12;

   PTE->PSOR |=1<<14;
   delay_us(Dtime);
   if(PTE->PDIR & (1<<0))Kbuff=2;      //2
   if(PTE->PDIR & (1<<1))Kbuff=5;      //5
   if(PTE->PDIR & (1<<2))Kbuff=8;      //8
   if(PTE->PDIR & (1<<3))Kbuff=0;      //0
   PTE->PCOR |=1<<14;

   PTE->PSOR |=1<<15;
   delay_us(Dtime);
   if(PTE->PDIR & (1<<0))Kbuff=3;      //3
   if(PTE->PDIR & (1<<1))Kbuff=6;      //6
   if(PTE->PDIR & (1<<2))Kbuff=9;      //9
   if(PTE->PDIR & (1<<3))Kbuff=12;    //#
   PTE->PCOR |=1<<15;

   return Kbuff;
}

void PORTE_IRQHandler(void){

	


			
	
	// External input Check Behavior Assignment
	
	
  PORTE->PCR[0] |= 0x01000000; // Port Control Register ISF bit '1' set
	PORTE->PCR[1] |= 0x01000000; // Port Control Register ISF bit '1' set
	PORTE->PCR[2] |= 0x01000000; // Port Control Register ISF bit '1' set
	PORTE->PCR[3] |= 0x01000000; // Port Control Register ISF bit '1' set

}


int main(void)
{
WDOG_disable();
   PORT_init();            /* Configure ports */
   SOSC_init_8MHz();       /* Initialize system oscilator for 8 MHz xtal */
   SPLL_init_160MHz();     /* Initialize SPLL to 160 MHz with 8 MHz SOSC */
		NormalRUNmode_80MHz();  /* Init clocks: 80 MHz sysclk & core, 40 MHz bus, 20 MHz flash */
	 NVIC_init_IRQs();       /* Enable desired interrupts and priorities */
   LPIT0_init(10);
int asum=0;
	num=0;
	unsigned long long k;
	k=rand();
	char msg[50];
	PTD->PSOR|=(1<<0)|(1<<15)|(1<<16);
	sprintf(msg,"%llu",k);
	
	text_lcd("push the button");

	
     a1=(k/1000000)%10;
 a2=(k/1)%10;
	 a3=(k/100)%10;
 a4=(k/1000000000)%10;
 asum=a1*1000+a2*100+a3*10+a4;
 	asum%=10000;
 
	if(!k1){
		lcdinit();
		i=0;
	PTD->PCOR|=(1<<15);
		
		text_lcd(msg);
			while(1){	 
				key=KeyScan();
	
			if((prekey!=key)&(key<10))
		{
		
	sum=10*sum+key;
	sum%=10000;
	
		prekey=key;
	
   	
   
}
			Seg_out(sum);

		
if(sum==asum) {
	lcdinit();
		i=0;
	text_lcd("correct");
PTD->PCOR|=(1<<0);
	PTD->PSOR|=(1<<15);
	PTC->PSOR|=(1<<16);
	delay_us(100000);
	PTC->PCOR|=(1<<16);
    break;}

			}
	
	}
 



	else{
		text_lcd("push the button");
		
	}

	

}