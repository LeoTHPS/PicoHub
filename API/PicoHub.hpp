#pragma once
#include <cstdint>

#include <Firmware/Protocol.hpp>

#if defined(_PICO_HUB_API)
	#if defined(PICO_HUB_API_WIN32)
		#define PICO_HUB_EXPORT __declspec(dllexport)
	#else
		#define PICO_HUB_EXPORT 
	#endif
#else
	#if defined(PICO_HUB_API_WIN32)
		#define PICO_HUB_EXPORT __declspec(dllimport)
	#else
		#define PICO_HUB_EXPORT 
	#endif
#endif

struct pico_hub;
struct pico_hub_io;

enum PICO_HUB_ERRORS
{
	PICO_HUB_ERROR_NONE,
	PICO_HUB_ERROR_IO_ERROR,
	PICO_HUB_ERROR_REQUEST_FAILED,
	PICO_HUB_ERROR_,
};

typedef void(*pico_hub_i2c_scan_callback)(pico_hub* hub, uint8_t address, void* param);

extern "C"
{
	PICO_HUB_EXPORT pico_hub* pico_hub_open(const char* device);
	PICO_HUB_EXPORT void      pico_hub_close(pico_hub* hub);

	PICO_HUB_EXPORT int       pico_hub_get_id(pico_hub* hub, uint64_t* value);

	PICO_HUB_EXPORT int       pico_hub_get_led(pico_hub* hub, bool* value);
	PICO_HUB_EXPORT int       pico_hub_set_led(pico_hub* hub, bool value);

	PICO_HUB_EXPORT int       pico_hub_get_clock(pico_hub* hub, uint32_t* value);
	PICO_HUB_EXPORT int       pico_hub_set_clock(pico_hub* hub, uint32_t value);

	PICO_HUB_EXPORT int       pico_hub_get_latency_ms(pico_hub* hub, uint32_t* value);
	PICO_HUB_EXPORT int       pico_hub_get_latency_us(pico_hub* hub, uint32_t* value);

	PICO_HUB_EXPORT int       pico_hub_get_voltage(pico_hub* hub, PICO_HUB_VOLTAGE* value);
	PICO_HUB_EXPORT int       pico_hub_set_voltage(pico_hub* hub, PICO_HUB_VOLTAGE value);

	PICO_HUB_EXPORT int       pico_hub_restart(pico_hub* hub, bool restart_to_mass_storage);
	PICO_HUB_EXPORT int       pico_hub_shutdown(pico_hub* hub);

	PICO_HUB_EXPORT int       pico_hub_adc_init(pico_hub* hub, PICO_HUB_ADC channels);
	PICO_HUB_EXPORT int       pico_hub_adc_deinit(pico_hub* hub);
	PICO_HUB_EXPORT int       pico_hub_adc_get_channel(pico_hub* hub, PICO_HUB_ADC* value);
	PICO_HUB_EXPORT int       pico_hub_adc_set_channel(pico_hub* hub, PICO_HUB_ADC value);
	PICO_HUB_EXPORT int       pico_hub_adc_read(pico_hub* hub, uint16_t* value);

	PICO_HUB_EXPORT int       pico_hub_i2c_init(pico_hub* hub, PICO_HUB_I2C bus, uint8_t scl, uint8_t sda, uint32_t baud);
	PICO_HUB_EXPORT int       pico_hub_i2c_deinit(pico_hub* hub, PICO_HUB_I2C bus);
	PICO_HUB_EXPORT int       pico_hub_i2c_scan(pico_hub* hub, PICO_HUB_I2C bus, pico_hub_i2c_scan_callback callback, void* param);
	PICO_HUB_EXPORT int       pico_hub_i2c_read(pico_hub* hub, PICO_HUB_I2C bus, uint8_t address, void* buffer, size_t size, bool stop);
	PICO_HUB_EXPORT int       pico_hub_i2c_write(pico_hub* hub, PICO_HUB_I2C bus, uint8_t address, const void* buffer, size_t size, bool stop);
	PICO_HUB_EXPORT int       pico_hub_i2c_write_read(pico_hub* hub, PICO_HUB_I2C bus, uint8_t address, const void* tx, size_t tx_size, void* rx, size_t rx_size);

	// PICO_HUB_EXPORT int       pico_hub_pwm_init(pico_hub* hub);
	// PICO_HUB_EXPORT int       pico_hub_pwm_deinit(pico_hub* hub);
	// PICO_HUB_EXPORT int       pico_hub_pwm_(pico_hub* hub);

	PICO_HUB_EXPORT int       pico_hub_spi_init(pico_hub* hub, PICO_HUB_SPI bus, uint8_t miso, uint8_t mosi, uint8_t clock, uint8_t cs, uint32_t baud, bool slave);
	PICO_HUB_EXPORT int       pico_hub_spi_deinit(pico_hub* hub, PICO_HUB_SPI bus);
	PICO_HUB_EXPORT int       pico_hub_spi_read(pico_hub* hub, PICO_HUB_SPI bus, void* buffer, size_t size);
	PICO_HUB_EXPORT int       pico_hub_spi_write(pico_hub* hub, PICO_HUB_SPI bus, const void* buffer, size_t size);
	PICO_HUB_EXPORT int       pico_hub_spi_write_read(pico_hub* hub, PICO_HUB_SPI bus, const void* tx, void* rx, size_t size);

	PICO_HUB_EXPORT int       pico_hub_gpio_init(pico_hub* hub, uint8_t pin, bool direction, bool value);
	PICO_HUB_EXPORT int       pico_hub_gpio_deinit(pico_hub* hub, uint8_t pin);
	PICO_HUB_EXPORT int       pico_hub_gpio_get_pull_up(pico_hub* hub, uint8_t pin, bool* value);
	PICO_HUB_EXPORT int       pico_hub_gpio_set_pull_up(pico_hub* hub, uint8_t pin, bool value);
	PICO_HUB_EXPORT int       pico_hub_gpio_get_pull_down(pico_hub* hub, uint8_t pin, bool* value);
	PICO_HUB_EXPORT int       pico_hub_gpio_set_pull_down(pico_hub* hub, uint8_t pin, bool value);
	PICO_HUB_EXPORT int       pico_hub_gpio_get_direction(pico_hub* hub, uint8_t pin, bool* value);
	PICO_HUB_EXPORT int       pico_hub_gpio_set_direction(pico_hub* hub, uint8_t pin, bool value);
	PICO_HUB_EXPORT int       pico_hub_gpio_get_drive_strength(pico_hub* hub, uint8_t pin, int* value);
	PICO_HUB_EXPORT int       pico_hub_gpio_set_drive_strength(pico_hub* hub, uint8_t pin, int value);
	PICO_HUB_EXPORT int       pico_hub_gpio_read(pico_hub* hub, uint8_t pin, bool* value);
	PICO_HUB_EXPORT int       pico_hub_gpio_write(pico_hub* hub, uint8_t pin, bool value);

	PICO_HUB_EXPORT int       pico_hub_uart_init(pico_hub* hub, PICO_HUB_UART bus, uint8_t rx, uint8_t tx, uint32_t baud);
	PICO_HUB_EXPORT int       pico_hub_uart_deinit(pico_hub* hub, PICO_HUB_UART bus);
	PICO_HUB_EXPORT int       pico_hub_uart_read(pico_hub* hub, PICO_HUB_UART bus, void* buffer, size_t size);
	PICO_HUB_EXPORT int       pico_hub_uart_write(pico_hub* hub, PICO_HUB_UART bus, const void* buffer, size_t size);
}
