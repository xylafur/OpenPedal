#include "cli.h"

#include "stm32f0xx_usart.h"
#include "stm32f0xx_gpio.h"
#include "stm32f0xx_rcc.h"
#include <stdio.h>

#define BAUD_RATE 9600

void init_usart() {
    USART_InitTypeDef USART_InitStructure = {
        .USART_BaudRate = BAUD_RATE,
        .USART_WordLength = USART_WordLength_8b,
        .USART_StopBits = USART_StopBits_1,
        .USART_Parity = USART_Parity_No,
        .USART_HardwareFlowControl = USART_HardwareFlowControl_None,
        .USART_Mode = USART_Mode_Rx | USART_Mode_Tx,
    };

    /* Setup the GPIO pins for alternate function, Pull Up */
    GPIO_InitTypeDef gpioStructure  = {
        .GPIO_Mode = GPIO_Mode_AF,
        .GPIO_Speed = GPIO_Speed_50MHz,
        .GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3,
        .GPIO_OType = GPIO_OType_PP,
        .GPIO_PuPd = GPIO_PuPd_UP,
    };

    /* Enable AHB and APB clocks */
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);

    /* Configure PA2 and PA3 as USART */
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource2, GPIO_AF_1);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource3, GPIO_AF_1);

    GPIO_Init(GPIOA, &gpioStructure);
    USART_Init(USART2, &USART_InitStructure);

    USART_Cmd(USART2,ENABLE);

    printf("\r\n\rInitializing CLI...\n\r");
}
