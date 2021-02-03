/*---------------------------------------------------------------------------------------------------------*/
/*                                                                                                         */
/* Copyright(c) 2019 Nuvoton Technology Corp. All rights reserved.                                         */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/

/***********************************************************************************************************/
/* Website: http://www.nuvoton.com                                                                         */
/*  E-Mail : MicroC-8bit@nuvoton.com                                                                       */
/*  Date   : Jan/21/2019                                                                                   */
/***********************************************************************************************************/

/************************************************************************************************************/
/*  File Function: MS51 DEMO project                                                                        */
/************************************************************************************************************/

#include "MS51_16K.h"

enum{
	TARGET_CH0 = 0 ,
	TARGET_CH1 ,
	TARGET_CH2 ,
	TARGET_CH3 ,	
	
	TARGET_CH4 ,
	TARGET_CH5 ,
	TARGET_CH6 ,
	TARGET_CH7 ,

	TARGET_CH_DEFAULT	
}Channel_TypeDef;

/*_____ M A C R O S ________________________________________________________*/

#define ABS(X)  									((X) > 0 ? (X) : -(X)) 

#define MAKE12BIT(v1,v2)         				((((uint16_t)(v1))<<4)+(uint16_t)(v2))      //v1,v2 is uint8_t

#define _debug_log_1S_							(1)


/*_____ D E F I N I T I O N S ______________________________________________*/
#define SYS_CLOCK 								(24000000ul)

#define PWM_ANALOG_PIN						(P05)
#define PWM_ANALOG_Freq_Num   				(9)		// 9 : 2.12K , 10 : 1.92K



/*_____ D E C L A R A T I O N S ____________________________________________*/
volatile uint8_t u8TH0_Tmp = 0;
volatile uint8_t u8TL0_Tmp = 0;

volatile uint8_t u8TH1_Tmp = 0;
volatile uint8_t u8TL1_Tmp = 0;

volatile uint16_t u16analog_pwm_Duty = 0;
volatile uint16_t u16period = 0;

//UART 0
bit BIT_TMP;
bit BIT_UART;
bit uart0_receive_flag=0;
unsigned char uart0_receive_data;


typedef enum{
	flag_START = 0 ,

	flag_duty_swap ,
	
	flag_200us ,
	flag_1ms ,
	flag_5ms ,
	flag_10ms ,	
	flag_DEFAULT	
}Flag_Index;

volatile uint32_t BitFlag = 0;
#define BitFlag_ON(flag)							(BitFlag|=flag)
#define BitFlag_OFF(flag)							(BitFlag&=~flag)
#define BitFlag_READ(flag)							((BitFlag&flag)?1:0)
#define ReadBit(bit)								(uint32_t)(1<<bit)

#define is_flag_set(idx)							(BitFlag_READ(ReadBit(idx)))
#define set_flag(idx,en)							( (en == 1) ? (BitFlag_ON(ReadBit(idx))) : (BitFlag_OFF(ReadBit(idx))))



/*_____ F U N C T I O N S __________________________________________________*/

void send_UARTString(uint8_t* Data)
{
	#if 1
	uint16_t i = 0;

	while (Data[i] != '\0')
	{
		#if 1
		SBUF = Data[i++];
		#else
		UART_Send_Data(UART0,Data[i++]);		
		#endif
	}

	#endif

	#if 0
	uint16_t i = 0;
	
	for(i = 0;i< (strlen(Data)) ;i++ )
	{
		UART_Send_Data(UART0,Data[i]);
	}
	#endif

	#if 0
    while(*Data)  
    {  
        UART_Send_Data(UART0, (unsigned char) *Data++);  
    } 
	#endif
}

void send_UARTASCII(uint16_t Temp)
{
    uint8_t print_buf[16];
    uint16_t i = 15, j;

    *(print_buf + i) = '\0';
    j = (uint16_t)Temp >> 31;
    if(j)
        (uint16_t) Temp = ~(uint16_t)Temp + 1;
    do
    {
        i--;
        *(print_buf + i) = '0' + (uint16_t)Temp % 10;
        (uint16_t)Temp = (uint16_t)Temp / 10;
    }
    while((uint16_t)Temp != 0);
    if(j)
    {
        i--;
        *(print_buf + i) = '-';
    }
    send_UARTString(print_buf + i);
}

void PWM0_CH2_Analog_SetDuty(uint16_t d)
{
	/*
		duty : 0 ~ 100 ,
		value 0 ~ 255 ,

		formula : 
		target value = duty * 255 / 100
		
	*/
	uint16_t value = 0;

//	value = (d*255) / 100;
	value = d;	

	u16analog_pwm_Duty = value ;

}

void PWM0_CH2_Analog_Init(void)
{
	P05_PUSHPULL_MODE;	

	P05 = 0;
}

void enable_PWM0_CH2_AnalogPWM(uint8_t enable ,uint8_t duty)
{
	if (enable)
	{
	    ENABLE_TIMER1_INTERRUPT;                       //enable Timer0 interrupt
	    set_TCON_TR1; 
		PWM0_CH2_Analog_SetDuty(duty);	
	}
	else
	{
	    DISABLE_TIMER1_INTERRUPT;
	    clr_TCON_TR1; 		
		PWM0_CH2_Analog_SetDuty(0);		
	}

}


void GPIO_Init(void)
{
	P17_QUASI_MODE;		
	P30_PUSHPULL_MODE;	
}

void task_10ms(void)
{	
	static uint16_t counter_duty = 0;
	static uint16_t counter_1sec = 0;
	static uint16_t log = 0;
	
	if (is_flag_set(flag_10ms))
	{
		set_flag(flag_10ms,Disable);
		
		//application
		P30 = ~P30;

		if (counter_duty++ >= 250)
		{
			counter_duty = 0;
			if (is_flag_set(flag_duty_swap))
			{
				set_flag(flag_duty_swap,Disable);
			}
			else
			{
				set_flag(flag_duty_swap,Enable);
			}
		}
		
		if (counter_1sec++ >= 100)
		{
			counter_1sec = 0;
			
			#if (_debug_log_1S_ == 1)
			send_UARTString("1000 ms log :");	
			send_UARTASCII(log++);
			send_UARTString("\r\n");
			#endif			
		}		
	}
}

void task_5ms(void)
{
	if (is_flag_set(flag_5ms))
	{
		set_flag(flag_5ms,Disable);

		//application
//		P17 = ~P17;


	
	}
}

void task_1ms(void)
{
	if (is_flag_set(flag_1ms))
	{
		set_flag(flag_1ms,Disable);

		//application
//		P17 = ~P17;

		
		
	}
}

void task_200us(void)
{
	if (is_flag_set(flag_200us))
	{
		set_flag(flag_200us,Disable);

		//application
//		P17 = ~P17;


	
	}
}

void task_loop(void)
{

	task_200us();
	task_1ms();
	task_5ms();
	task_10ms();

	if (is_flag_set(flag_duty_swap))
	{
//		enable_PWM0_CH2_AnalogPWM(Enable , 50);
		enable_PWM0_CH2_AnalogPWM(Enable , 85);		
	}
	else
	{
		enable_PWM0_CH2_AnalogPWM(Disable , 0);
	}

}


void Timer1_IRQHandler(void)
{
	uint16_t value = 0;

	/*
		formula : 
		counter / (24 000 000 / psc) = timing ( 1/ freq)

		X = 65536 - counter ,

		psc = 1 (sys clock) or 12 (divide 12)
		target timing = 500 us ( 1 / 2000)

		if psc = 12 , freq = 2000 , 
		counter / (24 000 000 / 12) = 1 / 2000 
		counter = 1000


	*/

	if(PWM_ANALOG_PIN)	// if PWM_Pin is high
	{
		PWM_ANALOG_PIN = 0;
		u16period = (100 - u16analog_pwm_Duty) * PWM_ANALOG_Freq_Num;

		value = (65536 - u16period);
		TH1  = HIBYTE(value);
		TL1  = LOBYTE(value);

	}
	else	     // if PWM_Pin is low
	{
		PWM_ANALOG_PIN = 1;
		u16period = u16analog_pwm_Duty * PWM_ANALOG_Freq_Num;

		value = (65536 - u16period);
		TH1  = HIBYTE(value);
		TL1  = LOBYTE(value);
	}
	
}

void Timer1_ISR (void) interrupt 3         // Vector @  0x0B
{
    _push_(SFRS);	
	
    clr_TCON_TR1;
	
	Timer1_IRQHandler();	

    clr_TCON_TF1;
    set_TCON_TR1;
	
    _pop_(SFRS);	
}

void TIMER1_Init(void)
{
	uint16_t res = 0;

	/*
		formula : 16bit 
		(0xFFFF+1 - target)  / (24MHz/psc) = time base 

	*/	
//	const uint16_t TIMER_DIVx_VALUE_xus_FOSC_240000 = 65536-400;

	ENABLE_TIMER1_MODE1;	// mode 0 : 13 bit , mode 1 : 16 bit
    TIMER1_FSYS_DIV12;
	
//	u8TH1_Tmp = HIBYTE(TIMER_DIVx_VALUE_xus_FOSC_240000);
//	u8TL1_Tmp = LOBYTE(TIMER_DIVx_VALUE_xus_FOSC_240000); 

//    TH1 = u8TH1_Tmp;
//    TL1 = u8TL1_Tmp;
    TH1 = 0;
    TL1 = 0;
	
//    ENABLE_TIMER1_INTERRUPT;                       //enable Timer0 interrupt
    ENABLE_GLOBAL_INTERRUPT;                       //enable interrupts
  
//    set_TCON_TR1;                                  //Timer0 run
}



void Timer0_IRQHandler(void)
{
	/*
	  	200us base	

	  	1ms = 1000 / 200 = 5
	  	5ms = 5000 / 200 = 25
	  	10ms = 10000 / 200 = 50
	  	
	*/

	const uint16_t div_1ms = 5;	
	static uint16_t cnt_1ms = 0;

	const uint16_t div_5ms = 25;	
	static uint16_t cnt_5ms = 0;	
	
	const uint16_t div_10ms = 50;		//accurate 42 , with ADC conv.
	static uint16_t cnt_10ms = 0;

	set_flag(flag_200us,Enable);

	if (++cnt_1ms >= div_1ms)	
	{
		set_flag(flag_1ms,Enable);	
		cnt_1ms = 0;	
	}
	
	if (++cnt_5ms >= div_5ms)	
	{
		set_flag(flag_5ms,Enable);
		cnt_5ms = 0;	
	}

	if (++cnt_10ms >= div_10ms)
	{
		set_flag(flag_10ms,Enable);
		cnt_10ms = 0;		
	}
	
}

void Timer0_ISR(void) interrupt 1        // Vector @  0x0B
{
    _push_(SFRS);	
	
    TH0 = u8TH0_Tmp;
    TL0 = u8TL0_Tmp;
    clr_TCON_TF0;
	
	Timer0_IRQHandler();

    _pop_(SFRS);	
}

void TIMER0_Init(void)
{
	uint16_t res = 0;

	/*
		formula : 16bit 
		(0xFFFF+1 - target)  / (24MHz/psc) = time base 

	*/	
	const uint16_t TIMER_DIVx_VALUE_200us_FOSC_240000 = 65536-400;

	ENABLE_TIMER0_MODE1;	// mode 0 : 13 bit , mode 1 : 16 bit
    TIMER0_FSYS_DIV12;
	
	u8TH0_Tmp = HIBYTE(TIMER_DIVx_VALUE_200us_FOSC_240000);
	u8TL0_Tmp = LOBYTE(TIMER_DIVx_VALUE_200us_FOSC_240000); 

    TH0 = u8TH0_Tmp;
    TL0 = u8TL0_Tmp;

    ENABLE_TIMER0_INTERRUPT;                       //enable Timer0 interrupt
    ENABLE_GLOBAL_INTERRUPT;                       //enable interrupts
  
    set_TCON_TR0;                                  //Timer0 run
}


void Serial_ISR (void) interrupt 4 
{
    _push_(SFRS);

    if (RI)
    {   
      uart0_receive_flag = 1;
      uart0_receive_data = SBUF;
      clr_SCON_RI;                                         // Clear RI (Receive Interrupt).
    }
    if  (TI)
    {
      if(!BIT_UART)
      {
          TI = 0;
      }
    }

    _pop_(SFRS);	
}

void UART0_Init(void)
{
	#if 1
	const unsigned long u32Baudrate = 115200;
	P06_QUASI_MODE;    //Setting UART pin as Quasi mode for transmit
	
	SCON = 0x50;          //UART0 Mode1,REN=1,TI=1
	set_PCON_SMOD;        //UART0 Double Rate Enable
	T3CON &= 0xF8;        //T3PS2=0,T3PS1=0,T3PS0=0(Prescale=1)
	set_T3CON_BRCK;        //UART0 baud rate clock source = Timer3

	RH3    = HIBYTE(65536 - (SYS_CLOCK/16/u32Baudrate));  
	RL3    = LOBYTE(65536 - (SYS_CLOCK/16/u32Baudrate));  
	
	set_T3CON_TR3;         //Trigger Timer3
	set_IE_ES;

	ENABLE_GLOBAL_INTERRUPT;

	set_SCON_TI;
	BIT_UART=1;
	#else	
    UART_Open(SYS_CLOCK,UART0_Timer3,115200);
    ENABLE_UART0_PRINTF; 
	#endif
}


void MODIFY_HIRC_24(void)
{
	unsigned char u8HIRCSEL = HIRC_24;
    unsigned char data hircmap0,hircmap1;
//    unsigned int trimvalue16bit;
    /* Check if power on reset, modify HIRC */
    SFRS = 0 ;
	#if 1
    IAPAL = 0x38;
	#else
    switch (u8HIRCSEL)
    {
      case HIRC_24:
        IAPAL = 0x38;
      break;
      case HIRC_16:
        IAPAL = 0x30;
      break;
      case HIRC_166:
        IAPAL = 0x30;
      break;
    }
	#endif
	
    set_CHPCON_IAPEN;
    IAPAH = 0x00;
    IAPCN = READ_UID;
    set_IAPTRG_IAPGO;
    hircmap0 = IAPFD;
    IAPAL++;
    set_IAPTRG_IAPGO;
    hircmap1 = IAPFD;
    clr_CHPCON_IAPEN;

	#if 0
    switch (u8HIRCSEL)
    {
		case HIRC_166:
		trimvalue16bit = ((hircmap0 << 1) + (hircmap1 & 0x01));
		trimvalue16bit = trimvalue16bit - 15;
		hircmap1 = trimvalue16bit & 0x01;
		hircmap0 = trimvalue16bit >> 1;

		break;
		default: break;
    }
	#endif
	
    TA = 0xAA;
    TA = 0x55;
    RCTRIM0 = hircmap0;
    TA = 0xAA;
    TA = 0x55;
    RCTRIM1 = hircmap1;
    clr_CHPCON_IAPEN;
    PCON &= CLR_BIT4;
}


void SYS_Init(void)
{
    MODIFY_HIRC_24();

    ALL_GPIO_QUASI_MODE;
    ENABLE_GLOBAL_INTERRUPT;                // global enable bit	
}

void main (void) 
{
    SYS_Init();

    UART0_Init();
	GPIO_Init();					

	//TimerService  : 200us , 1ms , 5ms , 10ms
	TIMER0_Init();	// us base
	TIMER1_Init();

	//(P0.5 , PWM0_CH2) :  freq (2k) 
	PWM0_CH2_Analog_Init();
	
    while(1)
    {
		task_loop();
		
    }
}



