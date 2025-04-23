#include "avr_common/uart.h" // this includes the printf and initializes it
#include <util/delay.h>
#include <stdio.h>
#include <stdint.h>
#include <avr/io.h>

int main(){
    printf_init(); 
    while(1){
        printf("A ");
        _delay_ms(1000); //aspetta un secondo
    }
}
