/*=============================================================================
	File Name:	Test_RX_TX.c  
	Author:		NihalB
	Date:		17/03/2025
	Modified:	None
	©? Fanshawe College, 2025

	Description:  this code should constantly print all 0x20 to 0x7A values to
				a server on 192.168.4.1 WE are trying to test the functionality
				of the ESP01 module
=============================================================================*/
/* Preprocessor ===============================================================
   Hardware Configuration Bits ==============================================*/
#pragma config FOSC		= INTIO67
#pragma config PLLCFG	= OFF
#pragma config PRICLKEN = ON
#pragma config FCMEN	= OFF
#pragma config IESO		= OFF
#pragma config PWRTEN	= OFF 
#pragma config BOREN	= ON
#pragma config BORV		= 285 
#pragma config WDTEN	= OFF
#pragma config PBADEN	= OFF
#pragma config LVP		= OFF
#pragma config MCLRE	= EXTMCLR

// Libraries ==================================================================
#include <p18f45k22.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Constants  =================================================================
#define TRUE	1	
#define FALSE	0	
#define RC1READ RCREG1
#define RC1FLAG PIR1bits.RC1IF
#define ROW1 LATAbits.LATA0 // Row1 pin
#define ROW2 LATAbits.LATA1 // Row 2
#define ROW3 LATAbits.LATA2 // Row 3
#define ROW4 LATAbits.LATA3 // Row 4

#define COL1 PORTAbits.RA4 // Col 1
#define COL2 PORTAbits.RA5 // Col 2
#define COL3 PORTAbits.RA6 // Col 3
#define COL4 PORTAbits.RA7 // Col 4

#define BUF 10
#define BUFS 4

#define DELAY 2000

// Global Variables  ==========================================================
char atCWJAP2[]="AT+CWJAP?\r\n";
char atCWJAP[]="AT+CWJAP=\"Galaxy S23 E764\",\"12345678\"\r\n";
char atCIPSTART[]="AT+CIPSTART=\"TCP\",\"192.168.255.36\",80\r\n";
char atCIPSEND2[]="AT+CIPSEND=4\r\n";
char atCIPCLOSE[]="AT+CIPCLOSE\r\n";
char atCWMODE[]="AT+CWMODE=1\r\n";
char atCHECK[]="AT+CWAUTOCONN?\r\n";
char at[]="AT\r\n";
char atrst[]="AT+RST\r\n";
char rcByte=FALSE;
char is_ok=FALSE;

char closed[]="CLOSED";
char okay[]="OK";
char error[]="ERROR";

char value[3]="251";


typedef struct
{
	char is_okay;
	char count_ok;
	char is_ipd;
	char is_comma;
	char is_ready;
	char count_ready;
	char is_error;
	char is_sendVal[BUFS];
	char send_index;
}responseFlag_t;

responseFlag_t response;
responseFlag_t* resPtr=&response;

typedef struct
{
	char txArr[BUF];//hold last ten TXREG1 vals
	char rxArr[BUF];//hold last ten RCREG1 vals
	char compare[BUFS];//compare for OK and IP
	char index;
	char dataLen;
	char rxRdy;

}check_t;

check_t rxTxVals;
check_t* ptr=&rxTxVals;

// Functions  =================================================================
void isr();
void UART_EnableInterrupts();
void initCheck_t(check_t* ptr);
void initResponse_t(responseFlag_t* ptr);

void waitFor(char* response);
void sendData(char* data);
#pragma code interrupt_vector=0x0008
void interrupt_vector()
{
    _asm //reduce to assembly language
        GOTO isr //go find ISR function and execute it
    _endasm //return to C code
}
#pragma code

/*>>> intConfig: ===========================================================
Author:      Nihal Brarath
Date:        02/08/2024
Modified:    None
Desc:        Initialize the interrupts
Input:       None 
Returns:     None 
 ============================================================================*/
void intConfig(void)
{
    INTCON=0xC0;
    RCONbits.IPEN= FALSE;
    PIE1bits.RC1IE=TRUE;
} // eo intConfig::

/*>>> setOscTo4Mhz: ===========================================================
Author:        Nihal
Date:        24/05/2024
Modified:    None
Desc:        Sets the Osc to 4Mhz 
Input:         None 
Returns:    None
 ============================================================================*/
void setOscTo4Mhz(void)
{
    OSCCON=0x52; //4MHZ
    while(!OSCCONbits.HFIOFS);
} // eo setOscTo4Mhz::

/*>>> uart1Config: ===========================================================
Author:      Nihal Brarath
Date:        14/06/2024
Modified:    None
Desc:        Initialize the UART 8-bit, no parity and 9600 baud rate
Input:       None 
Returns:     None 
 ============================================================================*/
void uart1Config(void)
{
    SPBRG1=25;
    BAUDCON1=0x40;
    RCSTA1=0x90;
    TXSTA1=0x26;
} // eo uart1Config::

/*>>> systemInit: ===========================================================
Author:        Nihal
Date:        07/06/2024
Modified:    None
Desc:        Called once and initializes the PIC
Input:         None
Returns:    None
 ============================================================================*/
void systemInit(void)
{
    setOscTo4Mhz();
    uart1Config();
	intConfig();
	UART_EnableInterrupts();
	initCheck_t(ptr);
	initResponse_t(resPtr);
	sendData(atrst);
	Delay10KTCYx(2000);
	//waitFor(okay);
	sendData(atCWJAP);
	//Delay10KTCYx(5000);
	waitFor(okay);
} // eo systemInit::

/*>>> UART_EnableInterrupts: ===========================================================
Author:      Nihal Brarath
Date:        18/03/2025
Modified:    None
Desc:        enable global interrupts, set pins for UART tx and rx, keypad as well 
Input:       none  
Returns:     None 
 ============================================================================*/
void UART_EnableInterrupts()
{
    INTCONbits.GIE = 1;     // Enable Global Interrupts
    INTCONbits.PEIE = 1;    // Enable Peripheral Interrupts
    PIE1bits.RC1IE = 1;     // Enable UART Receive Interrupt
    PIR1bits.RC1IF = 0;     // Clear UART Receive Interrupt Flag

	ANSELCbits.ANSC6=0;
	ANSELCbits.ANSC7=0;
	TRISCbits.TRISC7 = 1;  // RX as input
    TRISCbits.TRISC6 = 0;  // TX as output

	//FOR KEYPAD/
	ANSELA =0x00;
    LATA   =0x0F;
    TRISA  =0xF0;
}//eo UART_EnableInterrupts::

/*>>> initResponse_t: ===========================================================
Author:      Nihal Brarath
Date:        20/03/2025
Modified:    None
Desc:        init the responseFlag_t variable response 
Input:       responseFlag* ptr  
Returns:     None 
 ============================================================================*/
void initResponse_t(responseFlag_t* ptr)
{
	ptr->is_okay=FALSE;
	ptr->count_ok=FALSE;
	ptr->is_ipd=FALSE;
	ptr->is_comma=FALSE;
	ptr->is_ready=FALSE;
	ptr->is_error=FALSE;
	ptr->send_index=FALSE;
	for(ptr->send_index=FALSE;ptr->send_index<BUFS;ptr->send_index++)
	{	
		ptr->is_sendVal[ptr->send_index]=FALSE;
	}
}//eo initResponse_t::

/*>>> initCheck_t: ===========================================================
Author:      Nihal Brarath
Date:        18/03/2025
Modified:    None
Desc:        init the check_t variable response 
Input:       check_t* ptr  
Returns:     None 
 ============================================================================*/
void initCheck_t(check_t* ptr)
{
	for(ptr->index=FALSE;ptr->index<BUF;ptr->index++)
	{
		ptr->txArr[ptr->index]=FALSE;//hold last ten TXREG1 vals
		ptr->rxArr[ptr->index]=FALSE;//hold last ten RCREG1 vals
		if(ptr->index<BUFS)
		{
			ptr->compare[ptr->index]=FALSE;
		}
	}
	ptr->index=FALSE;
	ptr->dataLen=FALSE;	
	ptr->rxRdy=FALSE;

}//eo initCheck_t::

/* >>> sendData: ===========================================================
Author:      Nihal Brarath
Date:        18/03/2025
Modified:    None
Desc:        send byte by byte data on the TXREG1, USART1 
Input:       char* data, str that needs to be sent  
Returns:     None 
 ============================================================================*/
void sendData(char* data)
{
    sprintf(data,"%s",data);
	while (*data) 
	{
    	while (!PIR1bits.TX1IF);  // Wait for the transmit buffer to be empty
    	TXREG1=*data++;         // Send the byte	
	}
	
}//eo sendData::

/*  >>> sendTCP: ===========================================================
Author:      Nihal Brarath
Date:        18/03/2025
Modified:    None
Desc:        start tcp connection and send data 
Input:       char value, that needs to be sent  
Returns:     None 
 ============================================================================*/
void sendTCP()
{
	sendData(atCIPSTART);
	waitFor(okay);
	sendData(atCIPSEND2);
	waitFor(okay);
	sendData(value);
	waitFor(okay);
	sendData(atCIPCLOSE);
	waitFor(okay);	
	
}//eo sendTCP::

/*>>> waitFor: ===========================================================
Author:      Nihal Brarath
Date:        03/04/2025
Modified:    None
Desc:        wait for a specific response, would be better if it were a ring buffer 
Input:       char* response, this is the array needs to be compared  
Returns:     None 
 ============================================================================*/
void waitFor(char* response)
{
	char check=TRUE;
	char* find;
	//sprintf(response,"%s",response);
	while(check)
	{	//WAIT FOR THE CHOSEN STR AND REREAD TILL IT GETS IT
		if(ptr->rxRdy==TRUE)
		{
			ptr->rxRdy=FALSE;
			find=strstr(ptr->rxArr,response);
			if(find)
			{
				check=FALSE;
				resPtr->is_okay=TRUE;
			}
			else
			{
				find=strstr(ptr->rxArr,error);
				if(find)
				{
					check=FALSE;
					resPtr->is_error=TRUE;
				}
			}
		}
	
	}
	
}// eo waitFor::

/*>>> isr: ===========================================================
Author:      Nihal Brarath
Date:        14/06/2024
Modified:    None
Desc:        Function for timer0 interrupt trigger event and receivers interupt trigger event
Input:       None 
Returns:     None 
 ============================================================================*/
void isr()
{
	if(RC1FLAG)
	{
		if(RCSTA1bits.OERR)//Check if over flow in rcreg1
		{
			RCSTA1bits.CREN=0;
			RCSTA1bits.CREN=1;
		}
		rcByte=FALSE;
		rcByte=RC1READ;
		
		//CHECKING IF ptr->index is in limit
		if(ptr->index>=BUF)
		{
			//sprintf(ptr->rxArr,"%s",ptr->rxArr);
			ptr->rxRdy=TRUE;
			ptr->index=FALSE;
		}
		ptr->rxArr[ptr->index]=rcByte;
		ptr->index++;
	}
	INTCON|=0xC0; 
}//eo isr::

/*=== MAIN: FUNCTION ==========================================================
 ============================================================================*/
void main( void )
{
	
	systemInit();
		
	//Delay10KTCYx(DELAY);
	//have to make an if else tree that checks ok, error, ready etc

	
	while(1)
	{
		sendTCP();	
	}
		
	/*
	
	sendTCP(send);
	sendData(atCWMODE);//MODE 1 
	
	waitFor(con);
	sendData(atCWJAP);//CONNECTED TO NET
	waitFor(con);//ADD TIMER0 TIMEOUT
		AT->OK
		AT+RST->ready
		AT+CWMODE=1->OK
		AT+CWJAP=''->OK
		AT+CIPSTART,SEND,CLOSE
	*/
	

} // eo main::