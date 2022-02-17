/*
 * File:   main.c
 * Author: Lourdes Ruiz
 *
 * Created on Jan 30, 2022, 18:50 PM
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
#include "ADC.h"
#include "SPI.h"

// Define la frecuencia para poder usar los delays
#define _XTAL_FREQ 8000000

/*
 * Constantes
 */

/*
 * Variables 
 */
uint8_t POT;
uint8_t voltaje = 0; 


/*
 * Prototipos de funciones
 */
void setup(void);
/*
 * Interrupcion 
 */

void __interrupt() isr (void)
{
    
    if(PIR1bits.ADIF) {
        if(ADCON0bits.CHS == 10)
            voltaje = ADRESH;
        PIR1bits.ADIF = 0;
    }
    
    if(SSPIF == 1){
        spiWrite(voltaje);
        SSPIF = 0;
    }
    
}

/*
 * Codigo Principal
 */

void main (void){
    
    setup();
    
    ADCON0bits.GO =1; //empezar una conversion inicial
    __delay_us(50);
    while(1){
        ADC_single();  //Conversi�n empleando un solo canal 
   
        PORTD = voltaje; 
    }
    
}

/*
 * Funciones
 */

void setup(void){
    // Configuraciones de entradas y salidas 
    ANSEL = 0;   
    ANSELH = 0b0100; //RB0  como entrada anal�gicas para ADC
    
    //TRISA = 0;
    
    TRISD = 0;
    TRISB = 0b00010;
    TRISE = 0;
    
    //valores iniciales
    PORTB = 0;
    //PORTA = 0;
    
    PORTD = 0;
    PORTE = 0;
   
     //Configuracion de oscilador
    OSCCONbits.IRCF = 0b0111; //8MHz
    OSCCONbits.SCS = 1; //ocsilador interno
    
    ADC_Init(8);
    
    //Configuracion de las interrupciones
    
    PIR1bits.ADIF = 0; //apagar la bandera de ADC
    PIE1bits.ADIE = 1; //habilitar interrupcion analogica
    INTCONbits.PEIE = 1; //interrupciones perifericas
    INTCONbits.GIE  = 1;
    PIR1bits.SSPIF = 0;         // Borramos bandera interrupci�n MSSP
    PIE1bits.SSPIE = 1;         // Habilitamos interrupci�n MSSP
    TRISAbits.TRISA5 = 1;       // Slave Select
    
    //Se configura como SLAVE
    spiInit(SPI_SLAVE_SS_EN, SPI_DATA_SAMPLE_MIDDLE, SPI_CLOCK_IDLE_LOW, SPI_IDLE_2_ACTIVE);
    
    return;
}
