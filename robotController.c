/*
*	Robot-Arm - robotController.c
*	----------------------------
*	Controls a 4 servo robot arm
*
*	by:
*	Cedric Seger @ced92
*	Erkki Kanninen @errrkki
*
*	Compiler:
*	B Knudsen Cc5x (not ANSI-C)
*/

#include "16f690.h"
#include "int16Cxx.h"
#pragma config |= 0x00D4

//### FUNCTION DECLARATIONS ###
void initserial( void );
void putchar( char );
char getchar( void ); 
void printf(const char *string, char variable);
void setup(void);
void printMainMenu(void);
void printControls(void);

//### GLOBAL VARIABLES ###
long servoDuty[4]; //16 bit array to store duty cycles for 4 servos
unsigned int servoIndex; //Index used to keep track of which servo to update
unsigned long duty; //Variable used to store temporary results for calculations
int servoSelect; //Index to keep track of which port to output PMW signal
int timerCount; //Counter to count number of interrupts by timer0
unsigned char key; //Variable to store incoming UART/USB character

//### INTERRUPT ROUTINE ###
#pragma origin 4
interrupt int_server( void ) {
	int_save_registers
  	char sv_FSR = FSR;
  /*
    Update PMW Duty Cycle for next period in the middle
    of current PMW signal period. I.e. every second time TIMER0
    overflows since it runs 2x the frequency of TIMER2.
    TIMER2 is reponsible for the PWM signal. Basically,
    is it time to update servo signal?
  */
	if( T0IF == 1 && !(timerCount % 2)) {
    /*
      If we have updated the 4 servos, then output no signal
      for the rest of the 20ms period. Each servo nees to get
      an update signal at a frequency of 50Hz.
    */
		if(servoIndex == 4) {
			servoIndex = 0;
			servoSelect = 1;
			CCPR1L = 0;
			CCP1CON = 0b00.00.1100;
			PSTRCON = 0b000.1.0.0.0.0;
			timerCount = -2;
		} else { //Else update PMW signal for the next period
			
			duty = servoDuty[servoIndex];
			duty = duty >> 2;
			duty = duty & 0xFF;
			CCPR1L = duty; //8 MSBs for duty cycle
			
			duty = servoDuty[servoIndex];
			duty = duty & 0x03;
			duty = duty << 4;
			CCP1CON = 0b00.00.1100; //2 LSBs for duty cycle
			CCP1CON |= duty;
			
			PSTRCON = 0b000.1.0.0.0.0; //reset register
			PSTRCON |= servoSelect; //update on which pin to output signal
			
			servoIndex++;
			servoSelect = servoSelect << 1; //shift by 1 bit to the left
		}
	} else if (T0IF && (servoSelect/2)) { //end of PMW period
    /*
      Routine to clear output on current pin when
      pulse steering the PMW signal to the next pin.
      If pin is not cleared, it tended to stay high.
    */
		char portToClear = servoSelect/2;
		if(portToClear == 1) {
			PORTC.5 = 0;
		}
		if(portToClear == 2) {
			PORTC.4 = 0;
		}
		if(portToClear == 4) {
			PORTC.3 = 0;
		}
		if(portToClear == 8) {
			PORTC.2 = 0;
		}
	}
	timerCount++;
  	T0IF = 0;    //Reset flag
  	FSR = sv_FSR;
  	int_restore_registers
}

//### MAIN FUNCTION ###
void main(void) {
  	char n = -1; //local index variable to keep track of what servo user want to operate
  	initserial();
  	setup();
  	printMainMenu();
  	printControls();

	/*
	main while loop for receiving controls and updating servo positions.
	Also prints feedback for user.
	*/
	while(1) {
		key = getchar(); //Get instructions from user on PC.
		//Examine and correct servo positions if they exceed maximum.
		if(servoDuty[0] >= 592)
			servoDuty[0] = 592;
		if(servoDuty[1] >= 502)
			servoDuty[1] = 502;
		if(servoDuty[2] >= 602)
			servoDuty[2] = 602;
		if(servoDuty[3] >= 322)
			servoDuty[3] = 322;
		//Examine and correct servo positions if they fall below minimum.
		if(servoDuty[0] <= 142)
			servoDuty[0] = 142;
		if(servoDuty[1] <= 342)
			servoDuty[1] = 342;
		if(servoDuty[2] <= 292)
			servoDuty[2] = 292;
		if(servoDuty[3] <= 202)
			servoDuty[3] = 202;
	
		//Instructions
		switch (key) {
	  		case '1':
				printf("You are now controlling Base Servo\r\n", 0);
				n = 0;
				break;

			case '2':
				printf("You are now controlling Right Servo\r\n", 0);
				n = 2;
				break;

			case '3':
				printf("You are now controlling Left Servo\r\n", 0);
				n = 1;
				break;

			case '4':
				printf("You are now controlling Gripper Servo\r\n", 0);
				n = 3;
				break;

			//Two simple controls
			case 'a':
				if(n == -1) {
					printf("Please select a servo\r\n", 0);
				} else {
					servoDuty[n] += 10; //Increase servo angle
				}
				break;

			case 'd':
				if(n == -1) {
					printf("Please select a servo\r\n", 0);
				} else {
					servoDuty[n] -= 10; //Decrease servo angle
				}
				break;

			default:
				printf("%c is an invalid command\r\n", key);

    } //end of switch
  } //end of while
} //end of main

//### HELPER FUNCTIONS ###
void printMainMenu(void) {
	printf("Welcome to Robo-Control\r\n", 0);
	printf("Select Servo: \r\n", 0);
	printf("(1) - Base Servo\r\n", 0);
	printf("(2) - Right Servo (Small Arm)\r\n", 0);
	printf("(3) - Left Servo (Large Arm)\r\n", 0);
	printf("(4) - Gripper Servo\r\n", 0);
}
  
void printControls(void) {
	printf("Controls are:\r\n", 0);
	printf("A - Forward\r\n", 0);
	printf("D - Backward\r\n", 0);
}

void setup(void) {
	//Initialize default servo positions
	servoDuty[0] = 312; //FOR RC5 (bottom servo)
	/*
	Lower Limit is 142 (0.55ms)
	Upper Limit is 592 (2.3ms)
	Default position is 312
	*/
	servoDuty[1] = 341; //FOR RC4 (big arm/left servo)
	/*
	Upper Limit is 562 (most forward)
	Lower Limit is 302 (most backward)
	Default Position is 342
	*/
	servoDuty[2] = 542; //FOR RC3 (small arm/right servo)
	/*
	Upper Limit is 572 (most backwards)
	Lower Limit is 222 (most forward)
	Default position is 432
	*/
	servoDuty[3] = 310; //FOR RC2 (gripper servo)
	/*
	Upper Limit is 322 (most CLOSED)
	Lower Limit is 202 (most OPEN)
	Default position is 310
	*/

	/*
	Initialize default values for global variables
	and clear appropriate ports
	*/ 
	timerCount = 2;
	servoIndex = 0;
	servoSelect = 1;

	TRISC.2 = 0;
	TRISC.3 = 0;
	TRISC.4 = 0;
	TRISC.5 = 0;

	/*TIMER0 SETUP FOR INTERRUPT SERVICES*/
	/*
	1.x.x.x.x.xxx weak pullups are disabled
	x.0.x.x.x.xxx interrupt on falling edge
	x.x.0.x.x.xxx Use FOSC/4 as source for TIMER0
	x.x.x.0.x.xxx Increment on low to high transition on TOCKI pin
	x.x.x.x.0.xxx assign prescalar to Timer0 mod
	x.x.x.x.x.010 Use prescalar of 8 (I.e. a 125kHz speed)->Twice as fast as TMR2
	*/
	T0IF = 0;
	TMR0 = 0;
	OPTION = 0b1.0.0.0.0.010;

	/* Setup TIMER2 */
	/*
	0.xxxx.x.xx  - unimplemented
	x.0000.x.xx  Postscaler 1:1 (not used)
	x.xxxx.1.xx  TMR2 is on
	x.xxxx.x.11  Prescalar of 16x --> Gives us 62.kHz
	*/
	TMR2IF = 0;2
	TMR2 = 0;
	PR2 = 0xFF; //Max period meaning resulting 244Hz final frequency(4ms period)
	T2CON = 0b0.0000.1.11;

	//Setup PMW UNIT
	/*
	00.xx.xxxx  - Single Output
	xx.00.xxxx  LSBs of the PMW duty cycle
	xx.xx.1100  PMW normal mode
	*/
	duty = servoDuty[servoIndex];
	duty = duty >> 2;
	duty = duty & 0xFF;
	CCPR1L = duty; //8 MSBs for duty cycle

	duty = servoDuty[servoIndex];
	duty = duty & 0x03;
	duty = duty << 4;
	CCP1CON = 0b00.00.1100;
	CCP1CON |= duty;

	//ENABLE INTERRUPTS
	GIE  = 1;
	T0IE = 1;

	//For pulse steering controls using PSTRCON register
	/*
	xxx.1.x.x.x.x	Output steering update occurs on next PMW period
	xxx.x.0.x.x.x Choose P1D pin for PMW output
	xxx.x.x.0.x.x Choose P1C pin for PMW output
	xxx.x.x.x.0.x Choose P1B pin for PMW output
	xxx.x.x.x.x.1 Choose P1A pin for PMW output
	*/
	PSTRCON = 0b000.1.0.0.0.1;
}

//Initialise PIC16F690 serialcom port *Borrowed from IE1206 Labs*
void initserial(void)
{
	/* One start bit, one stop bit, 8 data bit, no parity. 9600 Baud. */
	TXEN = 1;      /* transmit enable                   */
	SYNC = 0;      /* asynchronous operation            */
	TX9  = 0;      /* 8 bit transmission                */
	SPEN = 1;

	BRGH  = 0;     /* settings for 9600 Baud            */
	BRG16 = 1;     /* @ 4 MHz-clock frequency           */
	SPBRG = 25;

	CREN = 1;      /* Continuous receive                     */
	RX9  = 0;      /* 8 bit reception                        */
	ANSELH.3 = 0;  /* RB5 not AD-input but serial_in         */
	TRISB.5 = 0;   /* Tx output */
	TRISB.7 = 1;   /* Rx input  */
	TRISA.0 = 1;   /* using the PGD connection for UART-tool */
	TRISA.1 = 1;   /* using the PGC connection for UART-tool */
}

void putchar( char d_out )  /* sends one char */
{
	while (!TXIF) ;   /* wait until previus character transmitted */
	TXREG = d_out;
}

char getchar(void) {
	while(!RCIF); //wait for character to be transmitted from computer
	char data = RCREG;
	return data;
}

//Printf function *Borrowed from kth-IE1206 Labs*
void printf(const char *string, char variable)
{
  char i, k, m, a, b;
  for(i = 0 ; ; i++)
   {
     k = string[i];
     if( k == '\0') break;   // at end of string
     if( k == '%')           // insert variable in string
      {
        i++;
        k = string[i];
        switch(k)
         {
           case 'd':         // %d  signed 8bit
             if( variable.7 ==1) putchar('-');
             else putchar(' ');
             if( variable > 127) variable = -variable;  // no break!
           case 'u':         // %u unsigned 8bit
             a = variable/100;
             putchar('0'+a); // print 100's
             b = variable%100;
             a = b/10;
             putchar('0'+a); // print 10's
             a = b%10;        
             putchar('0'+a); // print 1's
             break;
           case 'b':         // %b BINARY 8bit
             for( m = 0 ; m < 8 ; m++ )
              {
                if (variable.7 == 1) putchar('1');
                else putchar('0');
                variable = rl(variable);
               }
              break;
           case 'c':         // %c  'char'
             putchar(variable);
             break;
           case '%':
             putchar('%');
             break;
           default:          // not implemented
             putchar('!');  
         }  
      }
      else putchar(k);
   }
}


          /* *********************************** */
          /*            HARDWARE                 */
          /* *********************************** */


          /*
                     ___________  ___________ 
                    |           \/           |
             +5V ---|Vdd      16F690      Vss|--- GND
                    |RA5        RA0/AN0/(PGD)|
                    |RA4            RA1/(PGC)|
                    |RA3/!MCLR/(Vpp)  RA2/INT|
             PMW -<-|RC5/CCP              RC0|
             PWM -<-|RC4/P1B              RC1|
             PWM -<-|RC3              RC2/P1D|->- PMW
                    |RC6                  RB4|
                    |RC7/AN9           RB5/Rx|->- (PC -> PIC) UART
UART (PC <- PIC) -<-|RB7/Tx               RB6|
                    |________________________| 
          */ 
