#include <msp430.h>

unsigned char clock_cycles = 0;

unsigned char MOTOR_PINS[4] = {BIT0, BIT5, BIT6, BIT7};
unsigned char duty_cycles[4] = {10, 10, 10, 10};


int main(void) {

    WDTCTL = WDTPW + WDTHOLD;       // Stop watchdog timer

    DCOCTL = 0;                         // Select lowest DCO settings
    BCSCTL1 = CALBC1_1MHZ;              // Set DCO to 1 MHz
    DCOCTL = CALDCO_1MHZ;

    P1SEL = BIT1 + BIT2 ;               // Select UART RX/TX function on P1.1,P1.2
    P1SEL2 = BIT1 + BIT2;

    UCA0CTL1 |= UCSSEL_2;               // UART Clock -> SMCLK
    UCA0BR0 = 104;                      // Baud Rate Setting for 1MHz 9600
    UCA0BR1 = 0;                        // Baud Rate Setting for 1MHz 9600
    UCA0MCTL = UCBRS_1;                 // Modulation Setting for 1MHz 9600
    UCA0CTL1 &= ~UCSWRST;               // Initialize UART Module
    IE2 |= UCA0RXIE;                    // Enable RX interrupt

    /*** GPIO Set-Up ***/
    P1DIR |= BIT0;                  // P1.0 set as output
    P1OUT &= 0;                 // P1.0 set LOW (Red LED)
    for (unsigned int i = 0; i < 4; i++) {
        P1DIR |= MOTOR_PINS[i];
    }

    /*** Timer0_A Set-Up ***/
    TA0CCR0 |= 100;                    // Counter value
    TA0CCTL0 |= CCIE;               // Enable Timer0_A interrupts
    TA0CTL |= TASSEL_1 + MC_1 + ID_3;      // ACLK, Up Mode (Counts to TA0CCR0)

    _BIS_SR(LPM0_bits + GIE);       // Enter Low power mode 0 with interrupts enabled
}


void __attribute__((interrupt(TIMER0_A0_VECTOR))) TIMERA_ISR(void) {

    for (unsigned int i = 4; i > 0; i--) {
        if (clock_cycles <= 10 + duty_cycles[i - 1]) {
            P1OUT |= MOTOR_PINS[i - 1];
        } else {
            P1OUT &= ~MOTOR_PINS[i - 1];
        }
    }

    clock_cycles++;
    if (clock_cycles > 20) {
        clock_cycles -= 20;
    }
}


#pragma vector=USCIAB0RX_VECTOR         // UART RX Interrupt Vector
__interrupt void USCI0RX_ISR(void) {
    int received = UCA0RXBUF;

    while (!(IFG2&UCA0TXIFG));          // Check if TX is ongoing
    UCA0TXBUF = received;          // TX -> Received Char

    int new_duty_cycle = (received & 0b00001111) > 15? 15: received & 0b00001111;

    if (received & 0b10000000) {
        duty_cycles[0] = new_duty_cycle;
    }
    if (received & 0b01000000) {
        duty_cycles[1] = new_duty_cycle;
    }
    if (received & 0b00100000) {
        duty_cycles[2] = new_duty_cycle;
    }
    if (received & 0b00010000) {
        duty_cycles[3] = new_duty_cycle;
    }

    UC0IFG &= ~UCA0RXIFG;
}

