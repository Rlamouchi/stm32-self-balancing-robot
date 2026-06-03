#include "stm32f4xx.h"
#include <string.h>
#include <stdio.h>
#include <math.h>

uint8_t dummy;
int16_t x, y, z;
uint8_t data[6];
int16_t gx, gy, gz;
float Ax, Ay, Az;
float Gx, Gy, Gz;
float pitch, pitchfi;


float Kp       = 400.0f;
float Ki       = 200.0f;    
float Kd       = 10.0f;
float setpoint = -0.5f;  

float error, prev_error, integral, derivative, output;
float dt = 0.005f;        


void config_TIM2(void) {
    RCC->AHB1ENR |= (1 << 0);
    RCC->APB1ENR |= (1 << 0);
    GPIOA->MODER &= ~(0xFF);
    GPIOA->MODER |=  (0xAA);
    GPIOA->AFR[0] &= ~(0xFFFF);
    GPIOA->AFR[0] |=  (0x1111);
    TIM2->PSC   = 15;
    TIM2->ARR   = 999;
    TIM2->CCMR1 = 0x6868;
    TIM2->CCMR2 = 0x6868;
    TIM2->CCER  = 0x1111;
    TIM2->CR1  |= (1 << 0);
}

void avance(uint32_t pwm) {
    TIM2->CCR1 = pwm;
    TIM2->CCR2 = 0;
    TIM2->CCR3 = 0;
    TIM2->CCR4 = pwm;
}

void arriere(uint32_t pwm) {
    TIM2->CCR1 = 0;
    TIM2->CCR2 = pwm;
    TIM2->CCR3 = pwm;
    TIM2->CCR4 = 0;
}

void stop(void) {
    TIM2->CCR1 = TIM2->CCR2 = TIM2->CCR3 = TIM2->CCR4 = 0;
}


void command(void) {
    if (pitchfi > 12.0f || pitchfi < -12.0f) { stop(); return; }

    uint32_t pwm = (uint32_t)(output < 0.0f ? -output : output);
   
    if (pwm > 999) pwm = 999;
   
   
    if (pwm < 300) pwm = 0;  
   
    if (output < 0.0f) arriere(pwm);
    else               avance(pwm);
}


void SPI1_Config(void) {
    RCC->AHB1ENR |= (1 << 0);
    RCC->APB2ENR |= (1 << 12);
    GPIOA->MODER |= (2 << 10) | (2 << 12) | (2 << 14);
    GPIOA->MODER |= (1 << 8);
    GPIOA->AFR[0] |= (5 << 20) | (5 << 24) | (5 << 28);
    SPI1->CR1 |= (1<<2)|(1<<4)|(1<<0)|(1<<1)|(1<<9)|(1<<8)|(1<<6);
}

void NSS_LOW(void)  { GPIOA->ODR &= ~(1 << 4); }
void NSS_HIGH(void) { GPIOA->ODR |=  (1 << 4); }

void comunication_SPI(uint8_t a) {
    while (!(SPI1->SR & (1 << 1)));
    SPI1->DR = a | 0x80;
    while (!(SPI1->SR & (1 << 0)));
    dummy = SPI1->DR;
    for (uint8_t i = 0; i < 6; i++) {
        while (!(SPI1->SR & (1 << 1)));
        SPI1->DR = 0x00;
        while (!(SPI1->SR & (1 << 0)));
        data[i] = SPI1->DR;
        while (SPI1->SR & (1 << 7));
    }
}

void delay_ms(uint32_t ms) {
    for (uint32_t i = 0; i < ms * 1600; i++) __NOP();
}

void supdata(void) {
    for (uint8_t i = 0; i < 6; i++) data[i] = 0x00;
}


void traitement(void) {
    Ax = x / 16384.0f;
    Ay = y / 16384.0f;
    Az = z / 16384.0f;
    Gx = gx / 131.0f;
    Gy = gy / 131.0f;

   
    pitch = atan2f(Ay, sqrtf(Ax*Ax + Az*Az)) * 180.0f / 3.14159f;
    float alpha = 0.98f;
    pitchfi = alpha * (pitchfi + Gx * dt) + (1.0f - alpha) * pitch;
}

void pid(float input) {
    error     = setpoint - input;
    integral += error * dt;
    if (integral >  20.0f) integral =  20.0f;
    if (integral < -20.0f) integral = -20.0f;

    derivative = -Gx;

    output = Kp * error + Ki * integral + Kd * derivative;
    prev_error = error;
}


int main(void) {
    SPI1_Config();
    config_TIM2();
    delay_ms(500);

    while (1) {
        NSS_LOW();
        comunication_SPI(0x3B);
        NSS_HIGH();
        x = (data[0] << 8) | data[1];
        y = (data[2] << 8) | data[3];
        z = (data[4] << 8) | data[5];
        supdata();

        NSS_LOW();
        comunication_SPI(0x43);
        NSS_HIGH();
        gx = (data[0] << 8) | data[1];
        gy = (data[2] << 8) | data[3];
        gz = (data[4] << 8) | data[5];
        supdata();

        traitement();
        pid(pitchfi);
        command();

        delay_ms(5);
    }
}