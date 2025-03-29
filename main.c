/*
 * File:   main.c
 * Author: obe
 *
 * Created on March 27, 2025, 9:09 PM
 */


#include "xc.h"
#include "timer.h"

#define CS_ACC PORTBbits.RB3
#define CS_GYR PORTBbits.RB4
#define CS_MAG PORTDbits.RD6

#define MAX_INT_LEN 2

unsigned int spi_write(unsigned int data) {
    
    while (SPI1STATbits.SPITBF == 1);

    SPI1BUF = data;
    
    while (SPI1STATbits.SPIRBF == 0);

    return SPI1BUF; // read to prevent buffer overrun
}

void init_uart() {
    ANSELA = ANSELB = ANSELC = ANSELD = ANSELE = ANSELG = 0x0000;

    RPINR18bits.U1RXR = 0b1001011; // mapping pin RD11(RPI75) to UART RX
    RPOR0bits.RP64R = 0b000001; // mapping pin RD0(RP64) to UART TX
    U1BRG = 467; // baud rate to 9600 -> 72 000 000 / (16 * 9600) - 1
    
    U1MODEbits.UARTEN = 1; // enable UART
    U1STAbits.UTXEN = 1; // enable UART transmission
}

void init_spi() {
    ANSELA = ANSELB = ANSELC = ANSELD = ANSELE = ANSELG = 0x0000;

    SPI1CON1bits.MSTEN = 1; // master mode 
    SPI1CON1bits.MODE16 = 0; // 8-bit mode 
    SPI1CON1bits.PPRE = 3; // 1:1 primary prescaler
    SPI1CON1bits.SPRE = 3; // 5:1 secondary prescaler
    
    TRISAbits.TRISA1 = 1; // RA1-RPI17 MISO
    TRISFbits.TRISF12 = 0; // RF12-RP108 SCK
    TRISFbits.TRISF13 = 0; // RF13-RP109 MOSI
    
    RPINR20bits.SDI1R = 0b0010001; // MISO (SDI1) - RPI17
    RPOR12bits.RP109R = 0b000101;// MOSI (SDO1) - RF13;
    RPOR11bits.RP108R = 0b000110; // SCK1;
    
    SPI1STATbits.SPIEN = 1; // enable SPI
}


void print_int_to_uart(int v) {
    static char rev_str [MAX_INT_LEN];
    int crs = 0;
    
    if (!v) {
        U1TXREG = '0';
    }
    
    if (v < 0) {
        v = -v;
        U1TXREG = '-';
        ++crs;
    }
    
    
    while (v) {
        rev_str[crs++] = '0' + (v % 10);
        v /= 10;
    }
    
    for (int i = crs - 1; i >= 0; --i) {
        U1TXREG = rev_str[i];
    }
}


int main(void) {
    init_uart();
    init_spi();
    
    //selecting the magnetometer
    CS_ACC = 1;
    CS_GYR = 1;
    CS_MAG = 0;
    
    
    
    U1TXREG = 'A';
    spi_write(0x4B);
    spi_write(0x01); // changing the magnetometer to sleep state
    tmr_wait_ms(TIMER1, 3); //waiting for the magnetometer to go into sleep state
    
    U1TXREG = 'B';
    spi_write(0x4B);
    spi_write(0x00); // changing the magnetometer to active state
    
    tmr_wait_ms(TIMER1, 3); //TODO: TO REMOVE
    
    U1TXREG = 'C';
    spi_write(0x40 | 0x80);
    unsigned int chip_id = spi_write(0x00);
       
    tmr_wait_ms(TIMER1, 3); //TODO: TO REMOVE

    U1TXREG = 'D';
    U1TXREG = '\n';
    
    tmr_wait_ms(TIMER1, 3); //TODO: TO REMOVE
    print_int_to_uart(chip_id);
    
    U1TXREG = '\n';
    
    while(1);
    
    return 0;
}
