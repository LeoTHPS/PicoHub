#pragma once
#include "Protocol.hpp"

bool             pico_hub_init();
void             pico_hub_deinit();

bool             pico_hub_run();

uint64_t         pico_hub_get_id();

bool             pico_hub_get_led(bool* value);
bool             pico_hub_set_led(bool value);

uint32_t         pico_hub_get_clock();
bool             pico_hub_set_clock(uint32_t value, bool required);

PICO_HUB_VOLTAGE pico_hub_get_voltage();
bool             pico_hub_set_voltage(PICO_HUB_VOLTAGE value);

void             pico_hub_restart();
void             pico_hub_restart_to_mass_storage();

void             pico_hub_shutdown();

bool             pico_hub_adc_init(PICO_HUB_ADC channels);
void             pico_hub_adc_deinit();
bool             pico_hub_adc_get_channel(PICO_HUB_ADC* value);
bool             pico_hub_adc_set_channel(PICO_HUB_ADC value);
bool             pico_hub_adc_read(uint16_t* value);

bool             pico_hub_i2c_init(PICO_HUB_I2C bus, uint8_t scl, uint8_t sda, uint32_t baud);
void             pico_hub_i2c_deinit(PICO_HUB_I2C bus);
bool             pico_hub_i2c_read(PICO_HUB_I2C bus, uint8_t address, void* buffer, size_t size, bool stop);
bool             pico_hub_i2c_write(PICO_HUB_I2C bus, uint8_t address, const void* buffer, size_t size, bool stop);
bool             pico_hub_i2c_write_read(PICO_HUB_I2C bus, uint8_t address, const void* tx, size_t tx_size, void* rx, size_t rx_size);

// bool             pico_hub_pwm_init();
// void             pico_hub_pwm_deinit();
// bool             pico_hub_pwm_();

bool             pico_hub_spi_init(PICO_HUB_SPI bus, uint8_t miso, uint8_t mosi, uint8_t clock, uint8_t cs, uint32_t baud, bool slave);
void             pico_hub_spi_deinit(PICO_HUB_SPI bus);
bool             pico_hub_spi_read(PICO_HUB_SPI bus, void* buffer, size_t size);
bool             pico_hub_spi_write(PICO_HUB_SPI bus, const void* buffer, size_t size);
bool             pico_hub_spi_write_read(PICO_HUB_SPI bus, const void* tx, void* rx, size_t size);

bool             pico_hub_gpio_init(uint8_t pin, bool direction, bool value);
void             pico_hub_gpio_deinit(uint8_t pin);
bool             pico_hub_gpio_get_pull_up(uint8_t pin, bool* value);
bool             pico_hub_gpio_set_pull_up(uint8_t pin, bool value);
bool             pico_hub_gpio_get_pull_down(uint8_t pin, bool* value);
bool             pico_hub_gpio_set_pull_down(uint8_t pin, bool value);
bool             pico_hub_gpio_get_direction(uint8_t pin, bool* value);
bool             pico_hub_gpio_set_direction(uint8_t pin, bool value);
bool             pico_hub_gpio_get_drive_strength(uint8_t pin, int* value);
bool             pico_hub_gpio_set_drive_strength(uint8_t pin, int value);
bool             pico_hub_gpio_read(uint8_t pin, bool* value);
bool             pico_hub_gpio_write(uint8_t pin, bool value);

bool             pico_hub_uart_init(PICO_HUB_UART bus, uint8_t rx, uint8_t tx, uint32_t baud);
void             pico_hub_uart_deinit(PICO_HUB_UART bus);
bool             pico_hub_uart_read(PICO_HUB_UART bus, void* buffer, size_t size);
bool             pico_hub_uart_write(PICO_HUB_UART bus, const void* buffer, size_t size);
