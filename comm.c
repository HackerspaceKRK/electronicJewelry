#include "comm.h"
#include <delay.h>

#include "hardware.h"

#pragma pack(1)
typedef struct
{
	uint8_t cmd;
	uint16_t data;
} TPacket;
#pragma pack()

#define TYPE_REQUEST_MASTER 'm'
#define TYPE_COLOR_DATA     'c'
#define TYPE_KEY_DATA       'k'

#define WAITING_FOR_PREAMBLE 0
#define WAITING_FOR_SYNC     1
#define WAITING_FOR_DATA     2
#define WAITING_FOR_CHK      3

// vars
//public
uint8_t commIsPaired = 0;
uint8_t commIsLocked = 0;
uint16_t mykey;

//private
uint8_t master = 0;
uint32_t nextBroadcastTime;
uint16_t commonColor;

uint32_t stateChangeTime, lastPing, lastSend;

//recv
int recvState = 0;
uint8_t mychk;
int dataIdx = 0;
TPacket recvPacket;

// funcs
void comm_sendPacket (TPacket* p);
void comm_processPacket ();
void comm_setupTx ();
void comm_setupSense ();

// public
void commInit ()
{
	comm_setupTx ();
	IO_ALT_OPEN_DRAIN(UART_RX);
	IO_ALT_SET(UART_TX, 1);
	IO_ALT_SET(UART_RX, 1);
	IO_PULLUP(UART_RX);
	USART1->BRR = USART_BRR(115200);
	USART1->CR1 = USART_CR1_UE | USART_CR1_TE | USART_CR1_RE | USART_CR1_RXNEIE;
	USART1->CR3 = USART_CR3_OVRDIS;

	ENABLE_INTERRUPT(USART1_IRQn);

	nextBroadcastTime = ticks + randminmax (5, 20) * 10;
	recvState = WAITING_FOR_PREAMBLE;
}
void commProcess ()
{
	if (commIsPaired)
	{
		comm_setupTx ();
		if (ticks - lastPing >= 2000)
		{
			commIsPaired = 0;
		}
		else if (ticks - lastSend >= 10)
		{
			TPacket p;
			if (master)
			{
				p.cmd = TYPE_COLOR_DATA;
				p.data = commonColor;
			}
			else
			{
				p.cmd = TYPE_KEY_DATA;
				p.data = mykey;
			}
			comm_sendPacket (&p);
			lastSend = ticks;
		}
	}
	else
	{
		if (ticks >= nextBroadcastTime)
		{
			comm_setupTx ();
			TPacket p;
			p.cmd = TYPE_REQUEST_MASTER;
			p.data = 0xabcd;
			USART1->CR1 &= ~USART_CR1_RE;
			comm_sendPacket (&p);
			USART1->CR1 |= USART_CR1_RE;
			nextBroadcastTime = ticks + randminmax (5, 20) * 10;
		}
		else
		{
			if (commIsLocked)
			{
				comm_setupSense ();
				if (IO_IS_LOW(UART_TX))
				{
					commIsLocked = 0;
					int i;
					for (i = 0; i < 6; i++)
					{
						TIM3->CCR1 = TIM3->CCR2 = TIM3->CCR4 = 255;
						_delay_ms (50);
						TIM3->CCR1 = TIM3->CCR2 = TIM3->CCR4 = 0;
						_delay_ms (50);
					}
				}
			}
		}
	}

	if (recvState != WAITING_FOR_PREAMBLE)
		if (ticks - stateChangeTime >= 200)
			recvState = WAITING_FOR_PREAMBLE;
}

// private
void comm_setupTx ()
{
	IO_ALT_OPEN_DRAIN(UART_TX);
}
void comm_setupSense ()
{
	IO_INPUT(UART_TX);
}

static inline void comm_sendByte (uint8_t b)
{
	USART1->TDR = b;
	while (!(USART1->ISR & USART_ISR_TC));
}
void comm_sendPacket (TPacket* p)
{
	int i;
	uint8_t chk = 0xcc;
	uint8_t *data = (uint8_t*)p;

	comm_sendByte (0xaa);
	comm_sendByte (0xab);
	for (i = 0; i < sizeof (TPacket); i++)
	{
		chk ^= data[i];
		comm_sendByte (data[i]);
	}
	comm_sendByte (chk);
}

void comm_processPacket ()
{
	TPacket *p = &recvPacket;
	switch (p->cmd)
	{
	case TYPE_REQUEST_MASTER:
		if (p->data == 0xabcd)
		{
			commIsPaired = 1;
			master = 0;
		}
		break;
	case TYPE_COLOR_DATA:
		if (commIsPaired && !master)
		{
			showColor (p->data);
			commIsLocked = 1;
		}
		break;
	case TYPE_KEY_DATA:
		{
			commIsPaired = 1;
			master = 1;

			uint16_t remoteKey = p->data;
			commonColor = mykey + remoteKey;
			showColor (commonColor);
			commIsLocked = 1;
		}
		break;
	}
	lastPing = ticks;
}

void USART1_Handler ()
{
	if (USART1->ISR & USART_ISR_ORE)
	{
		USART1->ICR = USART_ICR_ORECF;
	}
	if (USART1->ISR & USART_ISR_RXNE)
	{
		uint8_t d = USART1->RDR;
		
		uint8_t *packetData = (uint8_t*)&recvPacket;

		switch (recvState)
		{
		case WAITING_FOR_PREAMBLE:
			if (d == 0xaa)
			{
				recvState = WAITING_FOR_SYNC;
				stateChangeTime = ticks;
			}
			break;
		case WAITING_FOR_SYNC:
			if (d == 0xab)
			{
				recvState = WAITING_FOR_DATA;
				stateChangeTime = ticks;
				dataIdx = 0;
				mychk = 0xcc;
			}
			break;
		case WAITING_FOR_DATA:
			packetData[dataIdx] = d;
			dataIdx++;
			mychk ^= d;
			if (dataIdx == sizeof (TPacket))
			{
				recvState = WAITING_FOR_CHK;
				stateChangeTime = ticks;
			}
			break;
		case WAITING_FOR_CHK:
			if (mychk == d)
				comm_processPacket ();
			recvState = WAITING_FOR_PREAMBLE;
			break;
		}
	}
}
