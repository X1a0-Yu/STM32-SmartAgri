#include "board.h"

static volatile uint32_t *config_reg(GPIO_TypeDef *port, uint8_t pin)
{
    return pin < 8U ? &port->CRL : &port->CRH;
}

static void gpio_config(GPIO_TypeDef *port, uint8_t pin, uint8_t nibble)
{
    volatile uint32_t *reg = config_reg(port, pin);
    uint8_t shift = (uint8_t)((pin & 7U) * 4U);
    *reg = (*reg & ~(0xFU << shift)) | ((uint32_t)nibble << shift);
}

void gpio_config_output(GPIO_TypeDef *port, uint8_t pin, uint8_t mode)
{
    gpio_config(port, pin, mode);
}

void gpio_config_input(GPIO_TypeDef *port, uint8_t pin, uint8_t mode)
{
    gpio_config(port, pin, mode);
}

void gpio_write(GPIO_TypeDef *port, uint8_t pin, bool high)
{
    if (high) port->BSRR = 1UL << pin;
    else port->BRR = 1UL << pin;
}

bool gpio_read(GPIO_TypeDef *port, uint8_t pin)
{
    return (port->IDR & (1UL << pin)) != 0U;
}

void board_init(void)
{
    RCC->APB2ENR |= RCC_APB2ENR_AFIOEN | RCC_APB2ENR_IOPAEN | RCC_APB2ENR_IOPBEN | RCC_APB2ENR_IOPCEN;
    AFIO->MAPR = (AFIO->MAPR & ~(7UL << 24)) | (2UL << 24); /* JTAG off, SWD on */

    gpio_write(PUMP_PORT, PUMP_PIN, false);
    gpio_write(FAN_PORT, FAN_PIN, false);
    gpio_write(LIGHT_PORT, LIGHT_PIN, false);
    gpio_write(LED1_PORT, LED1_PIN, true);
    gpio_write(LED2_PORT, LED2_PIN, true);
    gpio_write(LED3_PORT, LED3_PIN, true);
    gpio_write(LED4_PORT, LED4_PIN, true);

    gpio_config_output(PUMP_PORT, PUMP_PIN, 0x2U);
    gpio_config_output(FAN_PORT, FAN_PIN, 0x2U);
    gpio_config_output(LIGHT_PORT, LIGHT_PIN, 0x2U);
    gpio_config_output(LED1_PORT, LED1_PIN, 0x2U);
    gpio_config_output(LED2_PORT, LED2_PIN, 0x2U);
    gpio_config_output(LED3_PORT, LED3_PIN, 0x2U);
    gpio_config_output(LED4_PORT, LED4_PIN, 0x2U);

    gpio_write(KEY1_PORT, KEY1_PIN, true);
    gpio_write(KEY2_PORT, KEY2_PIN, true);
    gpio_config_input(KEY1_PORT, KEY1_PIN, 0x8U);
    gpio_config_input(KEY2_PORT, KEY2_PIN, 0x8U);
    gpio_config_input(SOIL_DIGITAL_PORT, SOIL_DIGITAL_PIN, 0x8U);
    gpio_write(SOIL_DIGITAL_PORT, SOIL_DIGITAL_PIN, true);
}
