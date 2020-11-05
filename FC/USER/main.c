#include "stm32f10x.h"
#include "string.h"
#include "delay.h"
#include "usart.h"
#include "timer.h"
#include "math.h"
#include "parameter.h"
#include "led.h"
#include "pwm.h"
#include "key.h"
#include "cpg.h"

void AnalyzePkg(void);
void MainWork(void);
void InitTestPin(void);

u16 tick[4]={0,0,0,0};
u16 cpucnt;

u8 state;
u8 subState;
u8 keyState;
u16 ledInterval;
u8 ledFlash;

u8 caliCnt;
u8 caliNext;
float caliTmp[3];

float pwmOut[4];

s8 armed;
s8 dir;
s8 position;

int main(void)
{	
	u8 i,j,k;

	u8 t;
		
	delay_init();
	NVIC_Configuration();
	uart_init(115200);	
	LEDInit();	
	KeyInit();
	USART_ClearITPendingBit(USART1, USART_IT_RXNE);
	printf("PPT FC\r\n");
	printf("cfrpg\r\n");
	printf("Read parameters.\r\n");	
	ParamRead();
	if(params.headFlag!=0xCFCF||params.tailFlag!=0xFCFC)
	{
		ParamReset();
		ParamWrite();
		printf("Reset parameters.\r\n");		
	}	
	MainClockInit();
	CPGInit();
	PWMInit(params.pwm_rate);
	PWMInInit();
	InitTestPin();
	delay_ms(50);
	ledInterval=5000;
	ledFlash=0;
	while(1)
	{	
		//main work
		//400Hz
		if(tick[2]>=25)
		{
			tick[2]=0;
		}				
		//LED
		if(tick[0]>=ledInterval)
		{				
			if(ledFlash)
			{
				LEDFlash(ledFlash);
				LEDSet(0);
			}
			else
			{
				LEDFlip();
			}
			tick[0]=0;
		}
		if(tick[1]>=5000)
		{						
			tick[1]=0;	
			if(USART_RX_STA&0x8000)
			{				
				AnalyzePkg();
				USART_RX_STA=0;
			}			
			PAout(4)=1;
			CPGUpdate();
			PAout(4)=0;
		}		
	}
}



void AnalyzePkg(void)
{
	u8 len=USART_RX_STA&0x3FFF;
	u8 t,i;
	s32 v;
	
	if(USART_RX_BUF[0]=='s'&&USART_RX_BUF[1]=='e'&&USART_RX_BUF[2]=='t')
	{
		printf("SET CMD\r\n");
		if(len!=12)
		{
			printf("Invalid length:%d.\r\n",len);		
			return;
		}
		t=(USART_RX_BUF[3]-'0')*10+(USART_RX_BUF[4]-'0');
		printf("ID:%d\r\n",t);
		v=0;
		for(i=0;i<6;i++)
		{
			v*=10;
			v+=USART_RX_BUF[6+i]-'0';
		}
		if(USART_RX_BUF[5]=='-')
			v*=-1;
		printf("Value:%d\r\n",v);
		if(ParamSet(t,v)==0)
		{
			printf("Complete.\r\n");
			LEDFlash(5);
		}
		else
		{
			printf("Failed.\r\n");
			LEDFlash(3);
		}
		return;
	}
	if(USART_RX_BUF[0]=='s'&&USART_RX_BUF[1]=='h'&&USART_RX_BUF[2]=='o'&&USART_RX_BUF[3]=='w')
	{
		printf("SHOW CMD\r\n");
		ParamShow();
	}
	if(USART_RX_BUF[0]=='r'&&USART_RX_BUF[1]=='s'&&USART_RX_BUF[2]=='t'&&USART_RX_BUF[3]=='p')
	{
		printf("RSTP CMD\r\n");
		ParamReset();
		ParamWrite();
		printf("Param reset complete.\r\n");
		ParamShow();
	}
}

void TIM4_IRQHandler(void)
{
	if (TIM_GetITStatus(TIM4, TIM_IT_Update) != RESET)
	{
		TIM_ClearITPendingBit(TIM4, TIM_IT_Update);
		u8 n=4;
		while(n--)
		{
			tick[n]++;
		}
	}
}

void InitTestPin(void)
{
	GPIO_InitTypeDef gi;	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
	gi.GPIO_Pin=GPIO_Pin_4;
	gi.GPIO_Mode=GPIO_Mode_Out_PP;
	gi.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_Init(GPIOA,&gi);
}
