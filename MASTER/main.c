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
#include "LCD.h"
#include "USART.h"
#include "SPI.h"


// Define la frecuencia para poder usar los delays
#define _XTAL_FREQ 8000000

/*
 * Constantes
 */

/*
 * Variables 
 */

uint8_t centena, decena, unidad;
//uint8_t centena2, decena2, unidad2;

uint8_t POT1 = 0;

//uint8_t counter = 0;
uint8_t contador = 0;
uint8_t volt = 0;
uint8_t volti = 0;
float temp = 0;
float temps;
uint8_t voltaje1, voltaje2;    //se utiliza float para poder obtener decimales
char values;
unsigned char sensores[10];
char flag;

/*
 * Prototipos de funciones
 */
void setup(void);
void division(uint8_t counter);
void string2(char *str);
void give(char bit_cadena);
void ADC_slave(void);
void counter_slave(void);
void temperature_slave(void); 

/*
 * Interrupcion 
 */



/*
 * Codigo Principal
 */

void main (void){
    
    setup();
    //unsigned int a;
    //Se inicializa el LCD
    
    TRISD = 0x00;
    Lcd_Clear();
    Lcd_Set_Cursor(1,1);
    Lcd_Write_String("S1:   S2:    S3:");
    
    while(1){
       //Se llaman las funciones de los esclavos
       ADC_slave();
       counter_slave();
       temperature_slave();
       
       //5/255 = 0.01961
       voltaje1 = (POT1*0.01961);
       
       //Conversión a temperatura (primero a voltaje y luego a temp = (5/255)*100)
       temps = (temp*1.961);
       
       //División del contador (SLAVE 2)
       division(contador); 
       
       //Se usa sprintf para pasar a ASCII los valores de los sensores
       //En la variable "sensores" (un array) se guarda el dato, tipo de dato, variable donde se encuentra el dato a traducir
       Lcd_Set_Cursor(2,1);
       sprintf(sensores, "%.2fV ", voltaje1); //se utiliza %.2f para que me dé los dos primeros dígitos despúes del punto
       Lcd_Write_String(sensores);
       string2(sensores);
       string2("     ");
       
       sprintf(sensores, "%d", centena);
       Lcd_Write_String(sensores);
       string2(sensores);
       sprintf(sensores, "%d", decena);
       Lcd_Write_String(sensores);
       string2(sensores);
       sprintf(sensores, "%d ", unidad);
       Lcd_Write_String(sensores);
       string2(sensores);
       string2("     ");
       
       sprintf(sensores, "%.2fC", temps);
       Lcd_Write_String(sensores);
       string2(sensores);
       string2("  ");
       string2("\n");
       
       //Semáforo de temperatura
       if (temps < 24){
           PORTAbits.RA0 = 1;
           PORTAbits.RA1 = 0;
           PORTAbits.RA2 = 0;
                   
       }
       
       if (temps > 24 && temps < 27){
           PORTAbits.RA0 = 0;
           PORTAbits.RA1 = 1;
           PORTAbits.RA2 = 0;
                   
       }
       
       if (temps > 27){
           PORTAbits.RA0 = 0;
           PORTAbits.RA1 = 0;
           PORTAbits.RA2 = 1;
                   
       }
       
  }
    
}


/*
 * Funciones
 */

void setup(void){
    // Configuraciones de entradas y salidas 
    ANSEL = 0b0110000;  //RE0 y RE1 como entradas analógicas para ADC 
    ANSELH = 0;
    
    TRISA = 0;
    TRISD = 0;
    TRISB = 0;
    //TRISE = 0b111;
    
    //valores iniciales
    PORTB = 0;
    PORTA = 0;
    //PORTC = 0;
    PORTD = 0;
    PORTE = 0;
    
    TRISC0 = 0;
    PORTCbits.RC0 = 1;
    TRISC1 = 0;
    PORTCbits.RC1 = 1;
    TRISC2 = 0;
    PORTCbits.RC2 = 1;
    
    //Se declara el PIC como maestro
    spiInit(SPI_MASTER_OSC_DIV4, SPI_DATA_SAMPLE_MIDDLE, SPI_CLOCK_IDLE_LOW, SPI_IDLE_2_ACTIVE);
    
     //Configuracion de oscilador
    OSCCONbits.IRCF = 0b0111; //8MHz
    OSCCONbits.SCS = 1; //ocsilador interno
    
    USART_Init(8);
    
    Lcd_Init();
    
    
    return;
}


void give(char bit_cadena)
{
    //while(TXSTAbits.TRMT ==0);
    while (PIR1bits.TXIF ==0); //se espera a que el registro del transmisor este vacio
        TXREG = bit_cadena;    //le envia el dato a cada "bit" de la cadena. 
           
}

void string2(char *str)  //regresa un pointer hacia un caracter
{
    while (*str != '\0')  //siempre y cuando el pointer sea distinto a caracter nulo (Que no se haya terminado la cadena)
    {
        give(*str);       //la funcion de give ira tomando caracter por caracter
        str++;            //aumentando para ir mandando bit por bit 
                
    }
}

void division(uint8_t counter)
//% es el operador modulo, el cual produce el residuo de una division
{
     centena = ((counter/100));
     decena = ((counter/10)%10);
     unidad = (counter%10);
     
     return;
}

void ADC_slave(void){
    PORTCbits.RC0 = 0;       //Slave Select -> empieza la comunicacion
    __delay_ms(1);
    //Escritura en SSPBUF   
    spiWrite(volti);         //no se mandan datos hacia los pics esclavos
    //Se leen los datos enviados por SLAVE 1 y se guardan en la variable 
    POT1 = spiRead();
       
    __delay_ms(1);
    PORTCbits.RC0 = 1;       //Slave Deselect 
       
    //__delay_ms(250);
}

void counter_slave(void){
    PORTCbits.RC1 = 0;       //Slave Select
    __delay_ms(1);
       
    spiWrite(volti);
    //Se leen los datos enviados por SLAVE 2 y se guardan en la variable
    contador = spiRead();
       
    __delay_ms(1);
    PORTCbits.RC1 = 1;       //Slave Deselect 
       
    //__delay_ms(250);
}

void temperature_slave(void){
    PORTCbits.RC2 = 0;       //Slave Select
    __delay_ms(1);
       
    spiWrite(volti);
    //Se leen los datos enviados por SLAVE 3 y se guardan en la variable
    temp = spiRead();
       
    __delay_ms(1);
    PORTCbits.RC2 = 1;       //Slave Deselect 
       
    //__delay_ms(250);
} 