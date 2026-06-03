# STM32 Two-Wheel Self-Balancing Robot

A self-balancing robot built with STM32 and MPU9250, using a real-time PID control loop to maintain dynamic stability.

## Hardware
- STM32F407 microcontroller
- MPU9250 IMU sensor (SPI)
- 2x DC motors with L298N driver
- 2-wheel chassis

## How it works
The MPU9250 reads pitch angle via sensor fusion. A PID controller calculates the correction needed and generates PWM signals to the motors to maintain balance and reject disturbances in real time.

## Tech Stack
- STM32 Bare-Metal (SPI, PWM, TIM)
- C
-  Keil uVision

## Demo
[Watch demo video](f5013b8a-f2be-4281-9500-684aee6aa372.mp4)
