#include <stdint.h>
#include <stdio.h>




//***PROJECT DESIGN***//

/*

General View
1. Create DAC 2R-R 3 bit  for a 100 hz analog wave, 16 Mhz (16.000.000 cycles per second) , we want to make 1600 Hz to match the array's components, so 10.000 ticks are needed for this to happen.
2. Using 5 30 kOhm Resistors, and 2 15 kOhm Resistors
3. 3 bit sets at PK1,PK2,PK3
4. Using Audio Jack Female 3,5 mm, resistor series with the Tip, and GND MCU to the Sleeve of the female audio jack
5. As per that setup, so per bit we will get this amp list:

0 0A
1 
2
3
4
5
6
7

so as bit increases, current increases

Software Plan
1. Find the Addresses of the registers in Data sheet
2. Init DEN, RCGCGPIO, DIR
3. Create an Array that match a sine wave based on the bit and a variable for indexing
4. Init Systick with 1600
5. Create a DAC function that changes the GPIO K Data that match the bit that will run like a sinewave
6. In SysTick_Handler, 


*/


//**ADDRESSES**//

//GPIO K 
#define SYSCTL_RCGCGPIO_R       (*((volatile uint32_t *)0x400FE608))
#define GPIO_PORTK_DATA_R       (*((volatile uint32_t *)0x400613FC))
#define GPIO_PORTK_DIR_R        (*((volatile uint32_t *)0x40061400))
#define GPIO_PORTK_DEN_R        (*((volatile uint32_t *)0x4006151C))

//SysTick
#define NVIC_STCTRL_R						(*((volatile uint32_t *)0xE000E010)) //Offset 0x010
#define NVIC_STRELOAD_R					(*((volatile uint32_t *)0xE000E014)) //Offset 0x014
#define NVIC_STCURRENT_R				(*((volatile uint32_t *)0xE000E018)) //Offset 0x018
#define NVIC_SYS_PRI3_R  				(*((volatile uint32_t *)0xE000E40C)) //PRI3 because SysTick interrupt ( OFFSET 0X40C)

//**GLOBAL VAR**//

const uint8_t SineWave[16] = {4,5,6,7,7,7,6,5,4,3,2,1,1,1,2,3}; //to match a sinewave form
uint8_t Index = 0;


//**FUNCTIONS**//
// Enable global interrupts
void EnableInterrupts(void) {
    __asm("CPSIE I");  // CPSIE I = Clear Interrupt Disable bit, enabling interrupts
}

// Wait for interrupt 
void WaitForInterrupts(void) {
    __asm("WFI");  // WFI = Wait For Interrupt instruction
}

void GPIO_K_Init(void)
{
	SYSCTL_RCGCGPIO_R = 0x0200; //0000 0010 0000 0000, Port K Activated
	GPIO_PORTK_DIR_R = 0x0E; //0000 1110, PK1 PK2 PK3 Output
	GPIO_PORTK_DEN_R = 0X0E; // Enable digital function for PK1,PK2,PK3
}

void Systick_Init(void)
{
	NVIC_STCTRL_R = 0;
	NVIC_STRELOAD_R = 10000-1; //1600 Hz, with 16Mhz base frequency
	NVIC_STCURRENT_R = 0;
	NVIC_STCTRL_R = 0x07; // enable bit 0-2
}

void DAC_Funct(uint8_t value)
{
	GPIO_PORTK_DATA_R = (value << 1); //shift bit 1 to left because we are using PK1,PK2,PK3
	
}

void SysTick_Handler(void)
{
	
	Index = (Index + 1 )& 0x0F;
	DAC_Funct(SineWave[Index]);
}

int main (void)
{
	GPIO_K_Init();
	Systick_Init();
	EnableInterrupts();
	while (1)
	{
		WaitForInterrupts();
	}
}

