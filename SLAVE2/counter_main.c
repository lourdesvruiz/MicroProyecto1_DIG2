/*
 * File:   interrupts_libraries.c
 * Author: Lourdes Ruiz
 *
 * Created on Jan 24, 2022, 1:52 PM
 * 
 * Descripcion: 
 */


// PIC16F887 Configuration Bit Settings

// 'C' source line config statements

// CONFIG1
#pragma config FOSC = INTRC_NOCLKOUT// Oscillator Selection bits (INTOSCIO oscillator: I/O function on RA6/OSC2/CLKOUT pin, I/O function on RA7/OSC1/CLKIN)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled and can be enabled by SWDTEN bit of the WDTCON register)
#pragma config PWRTE = OFF      // Power-up Timer Enable bit (PWRT disabled)
#pragma config MCLRE = OFF      // RE3/MCLR pin function select bit (RE3/MCLR pin function is digital input, MCLR internally tied to VDD)
#pragma config CP = OFF         // Code Protection bit (Program memory code protection is disabled)
#pragma config CPD = OFF        // Data Code Protection bit (Data memory code protection is disabled)
#pragma config BOREN = OFF      // Brown Out Reset Selection bits (BOR disabled)
#pragma config IESO = OFF       // Internal External Switchover bit (Internal/External Switchover mode is disabled)
#pragma config FCMEN = OFF       // Fail-Safe Clock Monitor Enabled bit (Fail-Safe Clock Monitor is enabled)
#pragma config LVP = OFF         // Low Voltage Programming Enable bit (RB3/PGM pin has PGM function, low voltage programming enabled)

// CONFIG2
#pragma config BOR4V = BOR40V   // Brown-out Reset Selection bit (Brown-out Reset set to 4.0V)
#pragma config WRT = OFF        // Flash Program Memory Self Write Enable bits (Write protection off)

// #pragma config statements should precede project file includes.
// Use project enums instead of #define for ON and OFF.

#include <xc.h>
#include <stdint.h>
#include <stdio.h>
#include <pic16f887.h>
#include "SPI.h"


// Define la frecuencia para poder usar los delays
#define _XTAL_FREQ 8000000

/*
 * Constantes
 */

#define _tmr0_value 60

/*
 * Variables 
 */

uint8_t antirrebote1;
uint8_t antirrebote2;
uint8_t counter_button;

/*
 * Prototipos de funciones
 */
void setup(void);

/*
 * Interrupcion 
 */

void __interrupt() isr (void)
{
    //Se mandan los datos al master
    if(SSPIF == 1){
        spiWrite(counter_button);
        SSPIF = 0;
    }
}
    
/*
 * Codigo Principal
 */
void main (void)
{
    setup();
    counter_button = 0;
    antirrebote1 = 0;
    antirrebote2 = 0;
    while(1)
    {
        //Implementación de antirrebotes en contador (por medio de banderas)
        
        if (PORTBbits.RB0 == 0){
            antirrebote1 = 1;
        }
        if (PORTBbits.RB0 == 1 && antirrebote1 == 1){
            counter_button++;
            antirrebote1 = 0;
        }
        
        if (PORTBbits.RB1 == 0){
            antirrebote2 = 1;
        }
        if (PORTBbits.RB1 == 1 && antirrebote2 == 1){
            counter_button--;
            antirrebote2 = 0;
        }
 
        PORTD = counter_button;
 
    }
    
}

/*
 * Funciones
 */
void setup(void) 
{   // Configuraciones de entradas y salidas 
    ANSEL = 0;   
    ANSELH = 0;
    
    TRISA = 0;
    TRISB = 0b00000011;        //botones
    
    TRISD = 0;
    
    
    //Weak Pull-ups
    OPTION_REGbits.nRBPU = 0; //entrada negada
    WPUB = 0b0011;
   
    
    //valores iniciales
    PORTB = 0;
    PORTA = 0;
    
    PORTD = 0;
    PORTE = 0;
    
    //Configuracion de oscilador
    OSCCONbits.IRCF = 0b0111; //4MHz
    OSCCONbits.SCS = 1; //ocsilador interno
    
    //Configuracion de las interrupciones
   
    INTCONbits.PEIE = 1; //interrupciones perifericas
    INTCONbits.GIE  = 1;
    PIR1bits.SSPIF = 0;         // Borramos bandera interrupción MSSP
    PIE1bits.SSPIE = 1;         // Habilitamos interrupción MSSP
    TRISAbits.TRISA5 = 1;       // Slave Select
    
    //Se configura como SLAVE
    spiInit(SPI_SLAVE_SS_EN, SPI_DATA_SAMPLE_MIDDLE, SPI_CLOCK_IDLE_LOW, SPI_IDLE_2_ACTIVE);
    return;
    }