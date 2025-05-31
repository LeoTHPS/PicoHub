#include "Base16.hpp"
#include "PicoHub.hpp"
#include "Protocol.hpp"

#include <string>
#include <vector>
#include <cassert>
#include <cstring>

#include <tusb.h>

#include <pico/bootrom.h>
#include <pico/platform.h>
#include <pico/stdio_usb.h>
#include <pico/unique_id.h>

#include <hardware/adc.h>
#include <hardware/i2c.h>
#include <hardware/pwm.h>
#include <hardware/spi.h>
#include <hardware/gpio.h>
#include <hardware/uart.h>
#include <hardware/vreg.h>
#include <hardware/timer.h>
#include <hardware/clocks.h>
#include <hardware/resets.h>

// static_assert(PICO_HUB_GPIO_PULL_UP             == );
// static_assert(PICO_HUB_GPIO_PULL_DOWN           == );
static_assert(PICO_HUB_GPIO_LED                 == PICO_HUB_GPIO_25);
static_assert(PICO_HUB_GPIO_SLEW_SLOW           == (int)GPIO_SLEW_RATE_SLOW);
static_assert(PICO_HUB_GPIO_SLEW_FAST           == (int)GPIO_SLEW_RATE_FAST);
static_assert(PICO_HUB_GPIO_VALUE_LOW           == false);
static_assert(PICO_HUB_GPIO_VALUE_HIGH          == true);
static_assert(PICO_HUB_GPIO_DIRECTION_IN        == false);
static_assert(PICO_HUB_GPIO_DIRECTION_OUT       == true);
static_assert(PICO_HUB_GPIO_DRIVE_STRENGTH_2MA  == (int)GPIO_DRIVE_STRENGTH_2MA);
static_assert(PICO_HUB_GPIO_DRIVE_STRENGTH_4MA  == (int)GPIO_DRIVE_STRENGTH_4MA);
static_assert(PICO_HUB_GPIO_DRIVE_STRENGTH_8MA  == (int)GPIO_DRIVE_STRENGTH_8MA);
static_assert(PICO_HUB_GPIO_DRIVE_STRENGTH_12MA == (int)GPIO_DRIVE_STRENGTH_12MA);

static_assert(PICO_HUB_VOLTAGE_0_85             == (int)VREG_VOLTAGE_0_85);
static_assert(PICO_HUB_VOLTAGE_0_90             == (int)VREG_VOLTAGE_0_90);
static_assert(PICO_HUB_VOLTAGE_0_95             == (int)VREG_VOLTAGE_0_95);
static_assert(PICO_HUB_VOLTAGE_1_00             == (int)VREG_VOLTAGE_1_00);
static_assert(PICO_HUB_VOLTAGE_1_05             == (int)VREG_VOLTAGE_1_05);
static_assert(PICO_HUB_VOLTAGE_1_10             == (int)VREG_VOLTAGE_1_10);
static_assert(PICO_HUB_VOLTAGE_1_15             == (int)VREG_VOLTAGE_1_15);
static_assert(PICO_HUB_VOLTAGE_1_20             == (int)VREG_VOLTAGE_1_20);
static_assert(PICO_HUB_VOLTAGE_1_25             == (int)VREG_VOLTAGE_1_25);
static_assert(PICO_HUB_VOLTAGE_1_30             == (int)VREG_VOLTAGE_1_30);
static_assert(PICO_HUB_VOLTAGE_MIN              == (int)VREG_VOLTAGE_MIN);
static_assert(PICO_HUB_VOLTAGE_MAX              == (int)VREG_VOLTAGE_MAX);
static_assert(PICO_HUB_VOLTAGE_DEFAULT          == (int)VREG_VOLTAGE_DEFAULT);

typedef bool(*pico_hub_packet_handler)(const void* buffer, size_t size);

struct pico_hub_packet_context
{
	PICO_HUB_OPCODES        opcode;
	pico_hub_packet_handler handler;
};

bool pico_hub_packet_handler_get_id(const void* buffer, size_t size);
bool pico_hub_packet_handler_get_led(const void* buffer, size_t size);
bool pico_hub_packet_handler_set_led(const void* buffer, size_t size);
bool pico_hub_packet_handler_get_clock(const void* buffer, size_t size);
bool pico_hub_packet_handler_set_clock(const void* buffer, size_t size);
bool pico_hub_packet_handler_get_latency(const void* buffer, size_t size);
bool pico_hub_packet_handler_get_voltage(const void* buffer, size_t size);
bool pico_hub_packet_handler_set_voltage(const void* buffer, size_t size);
bool pico_hub_packet_handler_restart(const void* buffer, size_t size);
bool pico_hub_packet_handler_shutdown(const void* buffer, size_t size);
bool pico_hub_packet_handler_adc_init(const void* buffer, size_t size);
bool pico_hub_packet_handler_adc_deinit(const void* buffer, size_t size);
bool pico_hub_packet_handler_adc_get_channel(const void* buffer, size_t size);
bool pico_hub_packet_handler_adc_set_channel(const void* buffer, size_t size);
bool pico_hub_packet_handler_adc_read(const void* buffer, size_t size);
bool pico_hub_packet_handler_i2c_init(const void* buffer, size_t size);
bool pico_hub_packet_handler_i2c_deinit(const void* buffer, size_t size);
bool pico_hub_packet_handler_i2c_read(const void* buffer, size_t size);
bool pico_hub_packet_handler_i2c_write(const void* buffer, size_t size);
bool pico_hub_packet_handler_i2c_write_read(const void* buffer, size_t size);
bool pico_hub_packet_handler_pwm_(const void* buffer, size_t size);
bool pico_hub_packet_handler_spi_init(const void* buffer, size_t size);
bool pico_hub_packet_handler_spi_deinit(const void* buffer, size_t size);
bool pico_hub_packet_handler_spi_read(const void* buffer, size_t size);
bool pico_hub_packet_handler_spi_write(const void* buffer, size_t size);
bool pico_hub_packet_handler_spi_write_read(const void* buffer, size_t size);
bool pico_hub_packet_handler_gpio_init(const void* buffer, size_t size);
bool pico_hub_packet_handler_gpio_deinit(const void* buffer, size_t size);
bool pico_hub_packet_handler_gpio_get_pull_up(const void* buffer, size_t size);
bool pico_hub_packet_handler_gpio_set_pull_up(const void* buffer, size_t size);
bool pico_hub_packet_handler_gpio_get_pull_down(const void* buffer, size_t size);
bool pico_hub_packet_handler_gpio_set_pull_down(const void* buffer, size_t size);
bool pico_hub_packet_handler_gpio_get_direction(const void* buffer, size_t size);
bool pico_hub_packet_handler_gpio_set_direction(const void* buffer, size_t size);
bool pico_hub_packet_handler_gpio_get_drive_strength(const void* buffer, size_t size);
bool pico_hub_packet_handler_gpio_set_drive_strength(const void* buffer, size_t size);
bool pico_hub_packet_handler_gpio_read(const void* buffer, size_t size);
bool pico_hub_packet_handler_gpio_write(const void* buffer, size_t size);
bool pico_hub_packet_handler_uart_init(const void* buffer, size_t size);
bool pico_hub_packet_handler_uart_deinit(const void* buffer, size_t size);
bool pico_hub_packet_handler_uart_read(const void* buffer, size_t size);
bool pico_hub_packet_handler_uart_write(const void* buffer, size_t size);

constexpr const pico_hub_packet_context pico_hub_packet_handlers[PICO_HUB_OPCODES_COUNT] =
{
	{ PICO_HUB_OPCODE_GET_ID,                  &pico_hub_packet_handler_get_id },
	{ PICO_HUB_OPCODE_GET_LED,                 &pico_hub_packet_handler_get_led },
	{ PICO_HUB_OPCODE_SET_LED,                 &pico_hub_packet_handler_set_led },
	{ PICO_HUB_OPCODE_GET_CLOCK,               &pico_hub_packet_handler_get_clock },
	{ PICO_HUB_OPCODE_SET_CLOCK,               &pico_hub_packet_handler_set_clock },
	{ PICO_HUB_OPCODE_GET_LATENCY,             &pico_hub_packet_handler_get_latency },
	{ PICO_HUB_OPCODE_GET_VOLTAGE,             &pico_hub_packet_handler_get_voltage },
	{ PICO_HUB_OPCODE_SET_VOLTAGE,             &pico_hub_packet_handler_set_voltage },
	{ PICO_HUB_OPCODE_RESTART,                 &pico_hub_packet_handler_restart },
	{ PICO_HUB_OPCODE_SHUTDOWN,                &pico_hub_packet_handler_shutdown },

	{ PICO_HUB_OPCODE_ADC_INIT,                &pico_hub_packet_handler_adc_init },
	{ PICO_HUB_OPCODE_ADC_DEINIT,              &pico_hub_packet_handler_adc_deinit },
	{ PICO_HUB_OPCODE_ADC_GET_CHANNEL,         &pico_hub_packet_handler_adc_get_channel },
	{ PICO_HUB_OPCODE_ADC_SET_CHANNEL,         &pico_hub_packet_handler_adc_set_channel },
	{ PICO_HUB_OPCODE_ADC_READ,                &pico_hub_packet_handler_adc_read },

	{ PICO_HUB_OPCODE_I2C_INIT,                &pico_hub_packet_handler_i2c_init },
	{ PICO_HUB_OPCODE_I2C_DEINIT,              &pico_hub_packet_handler_i2c_deinit },
	{ PICO_HUB_OPCODE_I2C_READ,                &pico_hub_packet_handler_i2c_read },
	{ PICO_HUB_OPCODE_I2C_WRITE,               &pico_hub_packet_handler_i2c_write },
	{ PICO_HUB_OPCODE_I2C_WRITE_READ,          &pico_hub_packet_handler_i2c_write_read },

	{ PICO_HUB_OPCODE_PWM_INIT,                nullptr },
	{ PICO_HUB_OPCODE_PWM_DEINIT,              nullptr },
	{ PICO_HUB_OPCODE_PWM_,                    nullptr },

	{ PICO_HUB_OPCODE_SPI_INIT,                &pico_hub_packet_handler_spi_init },
	{ PICO_HUB_OPCODE_SPI_DEINIT,              &pico_hub_packet_handler_spi_deinit },
	{ PICO_HUB_OPCODE_SPI_READ,                &pico_hub_packet_handler_spi_read },
	{ PICO_HUB_OPCODE_SPI_WRITE,               &pico_hub_packet_handler_spi_write },
	{ PICO_HUB_OPCODE_SPI_WRITE_READ,          &pico_hub_packet_handler_spi_write_read },

	{ PICO_HUB_OPCODE_GPIO_INIT,               &pico_hub_packet_handler_gpio_init },
	{ PICO_HUB_OPCODE_GPIO_DEINIT,             &pico_hub_packet_handler_gpio_deinit },
	{ PICO_HUB_OPCODE_GPIO_GET_PULL_UP,        &pico_hub_packet_handler_gpio_get_pull_up },
	{ PICO_HUB_OPCODE_GPIO_SET_PULL_UP,        &pico_hub_packet_handler_gpio_set_pull_up },
	{ PICO_HUB_OPCODE_GPIO_GET_PULL_DOWN,      &pico_hub_packet_handler_gpio_get_pull_down },
	{ PICO_HUB_OPCODE_GPIO_SET_PULL_DOWN,      &pico_hub_packet_handler_gpio_set_pull_down },
	{ PICO_HUB_OPCODE_GPIO_GET_DIRECTION,      &pico_hub_packet_handler_gpio_get_direction },
	{ PICO_HUB_OPCODE_GPIO_SET_DIRECTION,      &pico_hub_packet_handler_gpio_set_direction },
	{ PICO_HUB_OPCODE_GPIO_GET_DRIVE_STRENGTH, &pico_hub_packet_handler_gpio_get_drive_strength },
	{ PICO_HUB_OPCODE_GPIO_SET_DRIVE_STRENGTH, &pico_hub_packet_handler_gpio_set_drive_strength },
	{ PICO_HUB_OPCODE_GPIO_READ,               &pico_hub_packet_handler_gpio_read },
	{ PICO_HUB_OPCODE_GPIO_WRITE,              &pico_hub_packet_handler_gpio_write },

	{ PICO_HUB_OPCODE_UART_INIT,               &pico_hub_packet_handler_uart_init },
	{ PICO_HUB_OPCODE_UART_DEINIT,             &pico_hub_packet_handler_uart_deinit },
	{ PICO_HUB_OPCODE_UART_READ,               &pico_hub_packet_handler_uart_read },
	{ PICO_HUB_OPCODE_UART_WRITE,              &pico_hub_packet_handler_uart_write }
};

template<size_t ... I>
consteval bool static_assert_pico_hub_packet_handlers(std::index_sequence<I ...>)
{
	return ((pico_hub_packet_handlers[I].opcode == I) && ...);
}
static_assert(static_assert_pico_hub_packet_handlers(std::make_index_sequence<PICO_HUB_OPCODES_COUNT> {}));

struct pico_hub_adc
{
	PICO_HUB_ADC channel;
	PICO_HUB_ADC channels;
};

struct pico_hub_i2c
{
	bool        is_initialized;

	i2c_inst_t* bus;
	uint32_t    baud;
};

struct pico_hub_pwm
{
	bool is_initialized;
};

struct pico_hub_spi
{
	bool        is_slave;
	bool        is_initialized;

	spi_inst_t* bus;
	uint32_t    baud;
};

struct pico_hub_gpio
{
	bool is_low;
	bool is_input;
	bool is_initialized;
};

struct pico_hub_uart
{
	bool         is_initialized;

	uart_inst_t* bus;
	uint32_t     baud;
};

struct
{
	bool                   is_created   = false;
	bool                   is_running   = false;
	bool                   is_stopping  = false;
	bool                   is_connected = false;

	pico_hub_adc           adc                           = {};
	pico_hub_i2c           i2c[PICO_HUB_I2C_BUS_COUNT]   = { { .bus = i2c0 }, { .bus = i2c1 } };
	pico_hub_spi           spi[PICO_HUB_SPI_BUS_COUNT]   = { { .bus = spi0 }, { .bus = spi1 } };
	pico_hub_gpio          gpio[PICO_HUB_GPIO_COUNT]     = {};
	pico_hub_uart          uart[PICO_HUB_UART_BUS_COUNT] = { { .bus = uart0 }, { .bus = uart1 } };

	uint64_t               device_id      = 0;
	uint32_t               device_clock   = SYS_CLK_HZ;
	PICO_HUB_VOLTAGE       device_voltage = PICO_HUB_VOLTAGE_DEFAULT;

	pico_hub_packets       packets;
	pico_hub_packet_header packets_header;
	char                   packets_buffer[((sizeof(pico_hub_packet_header) + sizeof(pico_hub_packets)) * 2) + 1] = {};
} pico_hub;

bool pico_hub_io_send(const void* buffer, size_t size)
{
	auto value = (const uint8_t*)buffer;
	char base16[2];

	for (size_t i = 0; i < size; ++i, ++value)
	{
		pico_hub_base16_encode(base16[0], base16[1], *value);
		stdio_put_string(base16, 2, false, false);
	}

	return true;
}
bool pico_hub_io_receive(void* buffer, size_t size)
{
	auto value = (uint8_t*)buffer;
	char base16[2];

	for (size_t i = 0; i < size; ++i, ++value)
	{
		base16[0] = stdio_getchar();
		base16[1] = stdio_getchar();

		if (!pico_hub_base16_decode(base16[0], base16[1], *value))
			return false;
	}

	return true;
}
template<PICO_HUB_OPCODES OPCODE>
auto pico_hub_io_get_request(const void* buffer, size_t size)
{
	const pico_hub_packet_request<OPCODE>* request = nullptr;

	if (size == sizeof(pico_hub_packet_request<OPCODE>))
		request = (const pico_hub_packet_request<OPCODE>*)buffer;

	return request;
}
template<PICO_HUB_OPCODES OPCODE>
bool pico_hub_io_send_response(const pico_hub_packet_response<OPCODE>& packet)
{
	pico_hub_packet_header header =
	{
		.opcode = (PICO_HUB_OPCODES)((PICO_HUB_PACKET_TYPE_RESPONSE << 6) | (OPCODE & 0x3F)),
		.length = sizeof(pico_hub_packet_response<OPCODE>)
	};

	if (!pico_hub_io_send(&header, sizeof(pico_hub_packet_header)))
		return false;

	if (!pico_hub_io_send(&packet, header.length))
		return false;

	stdio_flush();

	return true;
}
bool pico_hub_io_receive_and_execute_request()
{
	if (!pico_hub_io_receive(&pico_hub.packets_header, sizeof(pico_hub_packet_header)))
		return false;

	auto type   = (pico_hub.packets_header.opcode & 0x3) >> 6;
	auto opcode = pico_hub.packets_header.opcode & 0x3F;
	auto length = pico_hub.packets_header.length;

	if ((type != PICO_HUB_PACKET_TYPE_REQUEST) || (opcode >= PICO_HUB_OPCODES_COUNT) || (length > sizeof(pico_hub_packets)))
		return false;

	if (!pico_hub_io_receive(&pico_hub.packets, length))
		return false;

	if (!pico_hub_packet_handlers[opcode].handler(&pico_hub.packets, length))
		return false;

	return true;
}

bool             pico_hub_init()
{
	assert(!pico_hub.is_created);

	stdio_init_all();
	stdio_set_translate_crlf(&stdio_usb, false);

	pico_unique_board_id_t device_id;
	pico_get_unique_board_id(&device_id);
	memcpy(&pico_hub.device_id, device_id.id, PICO_UNIQUE_BOARD_ID_SIZE_BYTES);
	static_assert(sizeof(pico_hub.device_id == PICO_UNIQUE_BOARD_ID_SIZE_BYTES));

	pico_hub.device_clock   = SYS_CLK_HZ;
	pico_hub.device_voltage = PICO_HUB_VOLTAGE_DEFAULT;

	pico_hub.is_created = true;

	pico_hub_gpio_init(PICO_HUB_GPIO_LED, true, true);

	return true;
}
void             pico_hub_deinit()
{
	if (pico_hub.is_created)
	{
		pico_hub_adc_deinit();

		for (int i2c = 0; i2c < PICO_HUB_I2C_BUS_COUNT; ++i2c)
			pico_hub_i2c_deinit((PICO_HUB_I2C)i2c);

		for (int spi = 0; spi < PICO_HUB_SPI_BUS_COUNT; ++spi)
			pico_hub_spi_deinit((PICO_HUB_SPI)spi);

		for (int gpio = 0; gpio < PICO_HUB_GPIO_COUNT; ++gpio)
			if (gpio != PICO_HUB_GPIO_LED)
				pico_hub_gpio_deinit((PICO_HUB_GPIO)gpio);

		for (int uart = 0; uart < PICO_HUB_UART_BUS_COUNT; ++uart)
			pico_hub_uart_deinit((PICO_HUB_UART)uart);

		pico_hub_set_led(false);
		pico_hub_gpio_deinit(PICO_HUB_GPIO_LED);

		stdio_deinit_all();

		pico_hub.is_created = false;
	}
}

bool             pico_hub_run()
{
	assert(pico_hub.is_created);
	assert(!pico_hub.is_running);

	pico_hub.is_running = true;

	bool success = true;

	while (!pico_hub.is_stopping)
		if (!(success = pico_hub_io_receive_and_execute_request()))
			break;

	pico_hub.is_stopping = false;
	pico_hub.is_running  = false;

	return success;
}

uint64_t         pico_hub_get_id()
{
	return pico_hub.is_created ? pico_hub.device_id : 0;
}

bool             pico_hub_get_led(bool* value)
{
	return pico_hub.is_created && pico_hub_gpio_read(PICO_HUB_GPIO_LED, value);
}
bool             pico_hub_set_led(bool value)
{
	assert(pico_hub.is_created);

	return pico_hub_gpio_write(PICO_HUB_GPIO_LED, value);
}

uint32_t         pico_hub_get_clock()
{
	return pico_hub.is_created ? pico_hub.device_clock : 0;
}
bool             pico_hub_set_clock(uint32_t value, bool required)
{
	assert(pico_hub.is_created);

	if (!set_sys_clock_hz(value, required))
		return false;

	pico_hub.device_clock = value;

	return true;
}

PICO_HUB_VOLTAGE pico_hub_get_voltage()
{
	return pico_hub.is_created ? pico_hub.device_voltage : (PICO_HUB_VOLTAGE)0;
}
bool             pico_hub_set_voltage(PICO_HUB_VOLTAGE value)
{
	assert(pico_hub.is_created);

	vreg_set_voltage((vreg_voltage)value);

	pico_hub.device_voltage = value;

	return true;
}

void             pico_hub_restart()
{
	assert(pico_hub.is_created);

	*reinterpret_cast<volatile uint32_t*>(PPB_BASE + 0xED0C) = 0x05FA0004;
}
void             pico_hub_restart_to_mass_storage()
{
	assert(pico_hub.is_created);

	reset_usb_boot(0, 0);
}

void             pico_hub_shutdown()
{
	assert(pico_hub.is_created);

	if (pico_hub.is_running && !pico_hub.is_stopping)
		pico_hub.is_stopping = true;
}

bool             pico_hub_adc_init(PICO_HUB_ADC channels)
{
	assert(pico_hub.is_created);

	if (!channels)
		return false;

	if (!pico_hub.adc.channels)
	{
		adc_init();

		pico_hub.adc.channel = (PICO_HUB_ADC)0;
	}

	if (channels & PICO_HUB_ADC_GPIO_26)
		adc_gpio_init(26);
	if (channels & PICO_HUB_ADC_GPIO_27)
		adc_gpio_init(27);
	if (channels & PICO_HUB_ADC_GPIO_28)
		adc_gpio_init(28);
	if (channels & PICO_HUB_ADC_SYSTEM_VOLTAGE)
		adc_gpio_init(29);
	if (channels & PICO_HUB_ADC_SYSTEM_TEMPERATURE)
		adc_set_temp_sensor_enabled(true);

	pico_hub.adc.channels = (PICO_HUB_ADC)(pico_hub.adc.channels | channels);

	return true;
}
void             pico_hub_adc_deinit()
{
	assert(pico_hub.is_created);

	if (auto channels = pico_hub.adc.channels)
	{
		if (channels & PICO_HUB_ADC_GPIO_26)
			gpio_deinit(26);
		if (channels & PICO_HUB_ADC_GPIO_27)
			gpio_deinit(27);
		if (channels & PICO_HUB_ADC_GPIO_28)
			gpio_deinit(28);
		if (channels & PICO_HUB_ADC_SYSTEM_VOLTAGE)
			gpio_deinit(29);
		if (channels & PICO_HUB_ADC_SYSTEM_TEMPERATURE)
			adc_set_temp_sensor_enabled(false);

		pico_hub.adc.channel  = (PICO_HUB_ADC)0;
		pico_hub.adc.channels = (PICO_HUB_ADC)0;
	}
}
bool             pico_hub_adc_get_channel(PICO_HUB_ADC* value)
{
	assert(pico_hub.is_created);

	if (!pico_hub.adc.channel)
		return false;

	*value = pico_hub.adc.channel;

	return true;
}
bool             pico_hub_adc_set_channel(PICO_HUB_ADC value)
{
	assert(pico_hub.is_created);

	if (!(pico_hub.adc.channels & value))
		return false;

	switch (value)
	{
		case PICO_HUB_ADC_GPIO_26:            adc_select_input(0); break;
		case PICO_HUB_ADC_GPIO_27:            adc_select_input(1); break;
		case PICO_HUB_ADC_GPIO_28:            adc_select_input(2); break;
		case PICO_HUB_ADC_SYSTEM_VOLTAGE:     adc_select_input(3); break;
		case PICO_HUB_ADC_SYSTEM_TEMPERATURE: adc_select_input(4); break;
	}

	pico_hub.adc.channel = value;

	return true;
}
bool             pico_hub_adc_read(uint16_t* value)
{
	assert(pico_hub.is_created);

	if (!pico_hub.adc.channel)
		return false;

	switch (pico_hub.adc.channel)
	{
		case PICO_HUB_ADC_GPIO_26:
		case PICO_HUB_ADC_GPIO_27:
		case PICO_HUB_ADC_GPIO_28:
		case PICO_HUB_ADC_SYSTEM_VOLTAGE:
		case PICO_HUB_ADC_SYSTEM_TEMPERATURE:
			*value = adc_read();
			return true;
	}

	*value = 0;

	if (pico_hub.adc.channel & PICO_HUB_ADC_GPIO_26)
	{
		adc_select_input(0);

		*value += adc_read();
	}

	if (pico_hub.adc.channel & PICO_HUB_ADC_GPIO_27)
	{
		adc_select_input(1);

		*value += adc_read();
	}

	if (pico_hub.adc.channel & PICO_HUB_ADC_GPIO_28)
	{
		adc_select_input(2);

		*value += adc_read();
	}

	if (pico_hub.adc.channel & PICO_HUB_ADC_SYSTEM_VOLTAGE)
	{
		adc_select_input(3);

		*value += adc_read();
	}

	if (pico_hub.adc.channel & PICO_HUB_ADC_SYSTEM_TEMPERATURE)
	{
		adc_select_input(4);

		*value += adc_read();
	}

	return true;
}

bool             pico_hub_i2c_init(PICO_HUB_I2C bus, uint8_t scl, uint8_t sda, uint32_t baud)
{
	assert(pico_hub.is_created);

	if (bus >= PICO_HUB_I2C_BUS_COUNT)
		return false;

	auto i2c = &pico_hub.i2c[bus];

	if (i2c->is_initialized)
		return false;

	baud = i2c_init(i2c->bus, baud);
	gpio_set_function(sda, GPIO_FUNC_I2C);
	gpio_set_function(scl, GPIO_FUNC_I2C);
	gpio_pull_up(sda);
	gpio_pull_up(scl);

	i2c->baud           = baud;
	i2c->is_initialized = true;

	return true;
}
void             pico_hub_i2c_deinit(PICO_HUB_I2C bus)
{
	assert(pico_hub.is_created);

	if (bus < PICO_HUB_I2C_BUS_COUNT)
		if (auto i2c = &pico_hub.i2c[bus]; i2c->is_initialized)
		{
			i2c_deinit(i2c->bus);

			i2c->is_initialized = false;
		}
}
bool             pico_hub_i2c_read(PICO_HUB_I2C bus, uint8_t address, void* buffer, size_t size, bool stop)
{
	assert(pico_hub.is_created);

	if (bus < PICO_HUB_I2C_BUS_COUNT)
		if (auto i2c = &pico_hub.i2c[bus]; i2c->is_initialized)
			return i2c_read_blocking(i2c->bus, address, (uint8_t*)buffer, size, !stop) != PICO_ERROR_GENERIC;

	return false;
}
bool             pico_hub_i2c_write(PICO_HUB_I2C bus, uint8_t address, const void* buffer, size_t size, bool stop)
{
	assert(pico_hub.is_created);

	if (bus < PICO_HUB_I2C_BUS_COUNT)
		if (auto i2c = &pico_hub.i2c[bus]; i2c->is_initialized)
			return i2c_write_blocking(i2c->bus, address, (uint8_t*)buffer, size, !stop) != PICO_ERROR_GENERIC;

	return false;
}
bool             pico_hub_i2c_write_read(PICO_HUB_I2C bus, uint8_t address, const void* tx, size_t tx_size, void* rx, size_t rx_size)
{
	assert(pico_hub.is_created);

	if (bus < PICO_HUB_I2C_BUS_COUNT)
		if (auto i2c = &pico_hub.i2c[bus]; i2c->is_initialized)
		{
			if (i2c_write_blocking(i2c->bus, address, (const uint8_t*)tx, tx_size, true) == PICO_ERROR_GENERIC)
				return false;

			if (i2c_read_blocking(i2c->bus, address, (uint8_t*)rx, rx_size, false) == PICO_ERROR_GENERIC)
				return false;

			return true;
		}

	return false;
}

bool             pico_hub_spi_init(PICO_HUB_SPI bus, uint8_t miso, uint8_t mosi, uint8_t clock, uint8_t cs, uint32_t baud, bool slave)
{
	assert(pico_hub.is_created);

	if (bus >= PICO_HUB_SPI_BUS_COUNT)
		return false;

	auto spi = &pico_hub.spi[bus];

	if (spi->is_initialized)
		return false;

	baud = spi_init(spi->bus, baud);
	gpio_set_function(slave ? mosi : miso, GPIO_FUNC_SPI);
	gpio_set_function(clock, GPIO_FUNC_SPI);
	gpio_set_function(slave ? miso : mosi, GPIO_FUNC_SPI);
	gpio_set_function(cs, GPIO_FUNC_SPI);

	spi->baud           = baud;
	spi->is_slave       = slave;
	spi->is_initialized = true;

	return true;
}
void             pico_hub_spi_deinit(PICO_HUB_SPI bus)
{
	assert(pico_hub.is_created);

	if (bus < PICO_HUB_SPI_BUS_COUNT)
		if (auto spi = &pico_hub.spi[bus]; spi->is_initialized)
		{
			spi_deinit(spi->bus);

			spi->is_initialized = false;
		}
}
bool             pico_hub_spi_read(PICO_HUB_SPI bus, void* buffer, size_t size)
{
	assert(pico_hub.is_created);

	if (bus < PICO_HUB_SPI_BUS_COUNT)
		if (auto spi = &pico_hub.spi[bus]; spi->is_initialized)
		{
			spi_read_blocking(spi->bus, 0, (uint8_t*)buffer, size);

			return true;
		}

	return false;
}
bool             pico_hub_spi_write(PICO_HUB_SPI bus, const void* buffer, size_t size)
{
	assert(pico_hub.is_created);

	if (bus < PICO_HUB_SPI_BUS_COUNT)
		if (auto spi = &pico_hub.spi[bus]; spi->is_initialized)
		{
			spi_write_blocking(spi->bus, (const uint8_t*)buffer, size);

			return true;
		}

	return false;
}
bool             pico_hub_spi_write_read(PICO_HUB_SPI bus, const void* tx, void* rx, size_t size)
{
	assert(pico_hub.is_created);

	if (bus < PICO_HUB_SPI_BUS_COUNT)
		if (auto spi = &pico_hub.spi[bus]; spi->is_initialized)
		{
			spi_write_read_blocking(spi->bus, (const uint8_t*)tx, (uint8_t*)rx, size);

			return true;
		}

	return false;
}

bool             pico_hub_gpio_init(uint8_t pin, bool direction, bool value)
{
	assert(pico_hub.is_created);

	if (pin >= PICO_HUB_GPIO_COUNT)
		return false;

	auto gpio = &pico_hub.gpio[pin];

	if (gpio->is_initialized)
		return false;

	gpio_init(pin);
	gpio_set_dir(pin, direction);
	gpio_put(pin, value);

	gpio->is_low         = !value;
	gpio->is_input       = !direction;
	gpio->is_initialized = true;

	return true;
}
void             pico_hub_gpio_deinit(uint8_t pin)
{
	assert(pico_hub.is_created);

	if (pin < PICO_HUB_GPIO_COUNT)
		if (auto gpio = &pico_hub.gpio[pin]; gpio->is_initialized)
		{
			gpio_deinit(pin);

			gpio->is_initialized = false;
		}
}
bool             pico_hub_gpio_get_pull_up(uint8_t pin, bool* value)
{
	assert(pico_hub.is_created);

	if (pin < PICO_HUB_GPIO_COUNT)
		if (auto gpio = &pico_hub.gpio[pin]; gpio->is_initialized)
		{
			*value = gpio_is_pulled_up(pin);

			return true;
		}

	return false;
}
bool             pico_hub_gpio_set_pull_up(uint8_t pin, bool value)
{
	assert(pico_hub.is_created);

	if (pin < PICO_HUB_GPIO_COUNT)
		if (auto gpio = &pico_hub.gpio[pin]; gpio->is_initialized)
		{
			gpio_set_pulls(pin, value, gpio_is_pulled_down(pin));

			return true;
		}

	return false;
}
bool             pico_hub_gpio_get_pull_down(uint8_t pin, bool* value)
{
	assert(pico_hub.is_created);

	if (pin < PICO_HUB_GPIO_COUNT)
		if (auto gpio = &pico_hub.gpio[pin]; gpio->is_initialized)
		{
			*value = gpio_is_pulled_down(pin);

			return true;
		}

	return false;
}
bool             pico_hub_gpio_set_pull_down(uint8_t pin, bool value)
{
	assert(pico_hub.is_created);

	if (pin < PICO_HUB_GPIO_COUNT)
		if (auto gpio = &pico_hub.gpio[pin]; gpio->is_initialized)
		{
			gpio_set_pulls(pin, gpio_is_pulled_up(pin), value);

			return true;
		}

	return false;
}
bool             pico_hub_gpio_get_direction(uint8_t pin, bool* value)
{
	assert(pico_hub.is_created);

	if (pin < PICO_HUB_GPIO_COUNT)
		if (auto gpio = &pico_hub.gpio[pin]; gpio->is_initialized)
		{
			*value = !gpio->is_input;

			return true;
		}

	return false;
}
bool             pico_hub_gpio_set_direction(uint8_t pin, bool value)
{
	assert(pico_hub.is_created);

	if (pin < PICO_HUB_GPIO_COUNT)
		if (auto gpio = &pico_hub.gpio[pin]; gpio->is_initialized)
		{
			gpio_set_dir(pin, value);

			gpio->is_input = !value;

			return true;
		}

	return false;
}
bool             pico_hub_gpio_get_drive_strength(uint8_t pin, int* value)
{
	assert(pico_hub.is_created);

	if (pin < PICO_HUB_GPIO_COUNT)
		if (auto gpio = &pico_hub.gpio[pin]; gpio->is_initialized && !gpio->is_input)
		{
			*value = gpio_get_drive_strength(pin);

			return true;
		}

	return false;
}
bool             pico_hub_gpio_set_drive_strength(uint8_t pin, int value)
{
	assert(pico_hub.is_created);

	if (pin < PICO_HUB_GPIO_COUNT)
		if (auto gpio = &pico_hub.gpio[pin]; gpio->is_initialized && !gpio->is_input)
		{
			gpio_set_drive_strength(pin, (gpio_drive_strength)value);

			return true;
		}

	return false;
}
bool             pico_hub_gpio_read(uint8_t pin, bool* value)
{
	assert(pico_hub.is_created);

	if (pin < PICO_HUB_GPIO_COUNT)
		if (auto gpio = &pico_hub.gpio[pin]; gpio->is_initialized)
		{
			*value = gpio->is_input ? gpio_get(pin) : !gpio->is_low;

			return true;
		}

	return false;
}
bool             pico_hub_gpio_write(uint8_t pin, bool value)
{
	assert(pico_hub.is_created);

	if (pin < PICO_HUB_GPIO_COUNT)
		if (auto gpio = &pico_hub.gpio[pin]; gpio->is_initialized && !gpio->is_input)
		{
			gpio_put(pin, value);

			gpio->is_low = !value;

			return true;
		}

	return false;
}

bool             pico_hub_uart_init(PICO_HUB_UART bus, uint8_t rx, uint8_t tx, uint32_t baud)
{
	assert(pico_hub.is_created);

	if (bus >= PICO_HUB_UART_BUS_COUNT)
		return false;

	auto uart = &pico_hub.uart[bus];

	if (uart->is_initialized)
		return false;

	baud = uart_init(uart->bus, baud);
	gpio_set_function(tx, UART_FUNCSEL_NUM(uart->bus, tx));
	gpio_set_function(rx, UART_FUNCSEL_NUM(uart->bus, rx));

	uart->baud           = baud;
	uart->is_initialized = true;

	return true;
}
void             pico_hub_uart_deinit(PICO_HUB_UART bus)
{
	assert(pico_hub.is_created);

	if (bus < PICO_HUB_UART_BUS_COUNT)
		if (auto uart = &pico_hub.uart[bus]; uart->is_initialized)
		{
			uart_deinit(uart->bus);

			uart->is_initialized = false;
		}
}
bool             pico_hub_uart_read(PICO_HUB_UART bus, void* buffer, size_t size)
{
	assert(pico_hub.is_created);

	if (bus >= PICO_HUB_UART_BUS_COUNT)
		return false;

	auto uart = &pico_hub.uart[bus];

	if (!uart->is_initialized)
		return false;

	uart_read_blocking(uart->bus, (uint8_t*)buffer, size);

	return true;
}
bool             pico_hub_uart_write(PICO_HUB_UART bus, const void* buffer, size_t size)
{
	assert(pico_hub.is_created);

	if (bus >= PICO_HUB_UART_BUS_COUNT)
		return false;

	auto uart = &pico_hub.uart[bus];

	if (!uart->is_initialized)
		return false;

	uart_write_blocking(uart->bus, (const uint8_t*)buffer, size);

	return true;
}

bool             pico_hub_packet_handler_get_id(const void* buffer, size_t size)
{
	if (auto request = pico_hub_io_get_request<PICO_HUB_OPCODE_GET_ID>(buffer, size))
		return pico_hub_io_send_response<PICO_HUB_OPCODE_GET_ID>({
			.value = pico_hub_get_id()
		});

	return false;
}
bool             pico_hub_packet_handler_get_led(const void* buffer, size_t size)
{
	if (auto request = pico_hub_io_get_request<PICO_HUB_OPCODE_GET_LED>(buffer, size))
	{
		pico_hub_packet_response<PICO_HUB_OPCODE_GET_LED> response;
		response.success = pico_hub_get_led(&response.value);

		return pico_hub_io_send_response(response);
	}

	return false;
}
bool             pico_hub_packet_handler_set_led(const void* buffer, size_t size)
{
	if (auto request = pico_hub_io_get_request<PICO_HUB_OPCODE_SET_LED>(buffer, size))
		return pico_hub_io_send_response<PICO_HUB_OPCODE_SET_LED>({
			.success = pico_hub_set_led(request->value)
		});

	return false;
}
bool             pico_hub_packet_handler_get_clock(const void* buffer, size_t size)
{
	if (auto request = pico_hub_io_get_request<PICO_HUB_OPCODE_GET_CLOCK>(buffer, size))
		return pico_hub_io_send_response<PICO_HUB_OPCODE_GET_CLOCK>({
			.value   = pico_hub.device_clock
		});

	return false;
}
bool             pico_hub_packet_handler_set_clock(const void* buffer, size_t size)
{
	if (auto request = pico_hub_io_get_request<PICO_HUB_OPCODE_SET_CLOCK>(buffer, size))
		return pico_hub_io_send_response<PICO_HUB_OPCODE_SET_CLOCK>({
			.success = pico_hub_set_clock(request->value, false)
		});

	return false;
}
bool             pico_hub_packet_handler_get_latency(const void* buffer, size_t size)
{
	if (auto request = pico_hub_io_get_request<PICO_HUB_OPCODE_GET_LATENCY>(buffer, size))
		return pico_hub_io_send_response<PICO_HUB_OPCODE_GET_LATENCY>({});

	return false;
}
bool             pico_hub_packet_handler_get_voltage(const void* buffer, size_t size)
{
	if (auto request = pico_hub_io_get_request<PICO_HUB_OPCODE_GET_VOLTAGE>(buffer, size))
		return pico_hub_io_send_response<PICO_HUB_OPCODE_GET_VOLTAGE>({
			.value = pico_hub_get_voltage()
		});

	return false;
}
bool             pico_hub_packet_handler_set_voltage(const void* buffer, size_t size)
{
	if (auto request = pico_hub_io_get_request<PICO_HUB_OPCODE_SET_VOLTAGE>(buffer, size))
		return pico_hub_io_send_response<PICO_HUB_OPCODE_SET_VOLTAGE>({
			.success = pico_hub_set_voltage(request->value)
		});

	return false;
}
bool             pico_hub_packet_handler_restart(const void* buffer, size_t size)
{
	if (auto request = pico_hub_io_get_request<PICO_HUB_OPCODE_RESTART>(buffer, size))
	{
		pico_hub_io_send_response<PICO_HUB_OPCODE_RESTART>({});

		if (request->restart_to_mass_storage)
			pico_hub_restart_to_mass_storage();

		pico_hub_restart();

		return true;
	}

	return false;
}
bool             pico_hub_packet_handler_shutdown(const void* buffer, size_t size)
{
	if (auto request = pico_hub_io_get_request<PICO_HUB_OPCODE_SHUTDOWN>(buffer, size))
	{
		pico_hub_io_send_response<PICO_HUB_OPCODE_SHUTDOWN>({});
		pico_hub_shutdown();

		return true;
	}

	return false;
}
bool             pico_hub_packet_handler_adc_init(const void* buffer, size_t size)
{
	if (auto request = pico_hub_io_get_request<PICO_HUB_OPCODE_ADC_INIT>(buffer, size))
		return pico_hub_io_send_response<PICO_HUB_OPCODE_ADC_INIT>({
			.success = pico_hub_adc_init(request->channels)
		});

	return false;
}
bool             pico_hub_packet_handler_adc_deinit(const void* buffer, size_t size)
{
	if (auto request = pico_hub_io_get_request<PICO_HUB_OPCODE_ADC_DEINIT>(buffer, size))
	{
		pico_hub_adc_deinit();

		return pico_hub_io_send_response<PICO_HUB_OPCODE_ADC_DEINIT>({});
	}

	return false;
}
bool             pico_hub_packet_handler_adc_get_channel(const void* buffer, size_t size)
{
	if (auto request = pico_hub_io_get_request<PICO_HUB_OPCODE_ADC_GET_CHANNEL>(buffer, size))
	{
		pico_hub_packet_response<PICO_HUB_OPCODE_ADC_GET_CHANNEL> response;
		response.success = pico_hub_adc_get_channel(&response.value);

		return pico_hub_io_send_response(response);
	}

	return false;
}
bool             pico_hub_packet_handler_adc_set_channel(const void* buffer, size_t size)
{
	if (auto request = pico_hub_io_get_request<PICO_HUB_OPCODE_ADC_SET_CHANNEL>(buffer, size))
		return pico_hub_io_send_response<PICO_HUB_OPCODE_ADC_SET_CHANNEL>({
			.success = pico_hub_adc_set_channel(request->value)
		});

	return false;
}
bool             pico_hub_packet_handler_adc_read(const void* buffer, size_t size)
{
	if (auto request = pico_hub_io_get_request<PICO_HUB_OPCODE_ADC_READ>(buffer, size))
	{
		pico_hub_packet_response<PICO_HUB_OPCODE_ADC_READ> response;
		response.success = pico_hub_adc_read(&response.value);

		return pico_hub_io_send_response(response);
	}

	return false;
}
bool             pico_hub_packet_handler_i2c_init(const void* buffer, size_t size)
{
	if (auto request = pico_hub_io_get_request<PICO_HUB_OPCODE_I2C_INIT>(buffer, size))
	{
		pico_hub_packet_response<PICO_HUB_OPCODE_I2C_INIT> response;
		response.success = pico_hub_i2c_init(request->bus, request->scl, request->sda, request->baud);

		return pico_hub_io_send_response(response);
	}

	return false;
}
bool             pico_hub_packet_handler_i2c_deinit(const void* buffer, size_t size)
{
	if (auto request = pico_hub_io_get_request<PICO_HUB_OPCODE_I2C_DEINIT>(buffer, size))
	{
		pico_hub_i2c_deinit(request->bus);

		pico_hub_packet_response<PICO_HUB_OPCODE_I2C_DEINIT> response;

		return pico_hub_io_send_response(response);
	}

	return false;
}
bool             pico_hub_packet_handler_i2c_read(const void* buffer, size_t size)
{
	if (auto request = pico_hub_io_get_request<PICO_HUB_OPCODE_I2C_READ>(buffer, size))
	{
		std::vector<uint8_t> data(request->size, 0);
		pico_hub_packet_response<PICO_HUB_OPCODE_I2C_READ> response;
		response.success = pico_hub_i2c_read(request->bus, request->address, &data[0], request->size, request->stop);

		return pico_hub_io_send_response(response) && (!response.success || pico_hub_io_send(&data[0], request->size));
	}

	return false;
}
bool             pico_hub_packet_handler_i2c_write(const void* buffer, size_t size)
{
	if (auto request = pico_hub_io_get_request<PICO_HUB_OPCODE_I2C_WRITE>(buffer, size))
	{
		std::vector<uint8_t> data(request->size, 0);

		if (pico_hub_io_receive(&data[0], request->size))
		{
			pico_hub_packet_response<PICO_HUB_OPCODE_I2C_WRITE> response =
			{
				.success = pico_hub_i2c_write(request->bus, request->address, &data[0], request->size, request->stop)
			};

			return pico_hub_io_send_response(response);
		}
	}

	return false;
}
bool             pico_hub_packet_handler_i2c_write_read(const void* buffer, size_t size)
{
	if (auto request = pico_hub_io_get_request<PICO_HUB_OPCODE_I2C_WRITE_READ>(buffer, size))
	{
		std::vector<uint8_t> data_rx(request->size[1], 0);
		std::vector<uint8_t> data_tx(request->size[0], 0);

		if (pico_hub_io_receive(&data_tx[0], request->size[0]))
		{
			pico_hub_packet_response<PICO_HUB_OPCODE_I2C_WRITE_READ> response =
			{
				.success = pico_hub_i2c_write_read(request->bus, request->address, &data_tx[0], request->size[0], &data_rx[0], request->size[1])
			};

			return pico_hub_io_send_response(response) && (!response.success || pico_hub_io_send(&data_rx[0], request->size[1]));
		}
	}

	return false;
}
bool             pico_hub_packet_handler_spi_init(const void* buffer, size_t size)
{
	if (auto request = pico_hub_io_get_request<PICO_HUB_OPCODE_SPI_INIT>(buffer, size))
	{
		pico_hub_packet_response<PICO_HUB_OPCODE_SPI_INIT> response;
		response.success = pico_hub_spi_init(request->bus, request->miso, request->mosi, request->clock, request->cs, request->baud, request->slave);

		return pico_hub_io_send_response(response);
	}

	return false;
}
bool             pico_hub_packet_handler_spi_deinit(const void* buffer, size_t size)
{
	if (auto request = pico_hub_io_get_request<PICO_HUB_OPCODE_SPI_DEINIT>(buffer, size))
	{
		pico_hub_spi_deinit(request->bus);

		pico_hub_packet_response<PICO_HUB_OPCODE_SPI_DEINIT> response;

		return pico_hub_io_send_response(response);
	}

	return false;
}
bool             pico_hub_packet_handler_spi_read(const void* buffer, size_t size)
{
	if (auto request = pico_hub_io_get_request<PICO_HUB_OPCODE_SPI_READ>(buffer, size))
	{
		std::vector<uint8_t> data(request->size, 0);
		pico_hub_packet_response<PICO_HUB_OPCODE_SPI_READ> response;
		response.success = pico_hub_spi_read(request->bus, &data[0], request->size);

		return pico_hub_io_send_response(response) && (!response.success || pico_hub_io_send(&data[0], request->size));
	}

	return false;
}
bool             pico_hub_packet_handler_spi_write(const void* buffer, size_t size)
{
	if (auto request = pico_hub_io_get_request<PICO_HUB_OPCODE_SPI_WRITE>(buffer, size))
	{
		std::vector<uint8_t> data(request->size, 0);

		if (pico_hub_io_receive(&data[0], request->size))
		{
			pico_hub_packet_response<PICO_HUB_OPCODE_SPI_WRITE> response =
			{
				.success = pico_hub_spi_write(request->bus, &data[0], request->size)
			};

			return pico_hub_io_send_response(response);
		}
	}

	return false;
}
bool             pico_hub_packet_handler_spi_write_read(const void* buffer, size_t size)
{
	if (auto request = pico_hub_io_get_request<PICO_HUB_OPCODE_SPI_WRITE_READ>(buffer, size))
	{
		std::vector<uint8_t> data_rx(request->size, 0);
		std::vector<uint8_t> data_tx(request->size, 0);

		if (pico_hub_io_receive(&data_tx[0], request->size))
		{
			pico_hub_packet_response<PICO_HUB_OPCODE_SPI_WRITE_READ> response =
			{
				.success = pico_hub_spi_write_read(request->bus, &data_tx[0], &data_rx[0], request->size)
			};

			return pico_hub_io_send_response(response) && (!response.success || pico_hub_io_send(&data_rx[0], request->size));
		}
	}

	return false;
}
bool             pico_hub_packet_handler_gpio_init(const void* buffer, size_t size)
{
	if (auto request = pico_hub_io_get_request<PICO_HUB_OPCODE_GPIO_INIT>(buffer, size))
	{
		pico_hub_packet_response<PICO_HUB_OPCODE_GPIO_INIT> response;
		response.success = pico_hub_gpio_init(request->pin, request->direction, request->value);

		return pico_hub_io_send_response(response);
	}

	return false;
}
bool             pico_hub_packet_handler_gpio_deinit(const void* buffer, size_t size)
{
	if (auto request = pico_hub_io_get_request<PICO_HUB_OPCODE_GPIO_DEINIT>(buffer, size))
	{
		pico_hub_gpio_deinit(request->pin);

		pico_hub_packet_response<PICO_HUB_OPCODE_GPIO_DEINIT> response;

		return pico_hub_io_send_response(response);
	}

	return false;
}
bool             pico_hub_packet_handler_gpio_get_pull_up(const void* buffer, size_t size)
{
	if (auto request = pico_hub_io_get_request<PICO_HUB_OPCODE_GPIO_GET_PULL_UP>(buffer, size))
	{
		pico_hub_packet_response<PICO_HUB_OPCODE_GPIO_GET_PULL_UP> response;
		response.success = pico_hub_gpio_get_pull_up(request->pin, &response.value);

		return pico_hub_io_send_response(response);
	}

	return false;
}
bool             pico_hub_packet_handler_gpio_set_pull_up(const void* buffer, size_t size)
{
	if (auto request = pico_hub_io_get_request<PICO_HUB_OPCODE_GPIO_SET_PULL_UP>(buffer, size))
	{
		pico_hub_packet_response<PICO_HUB_OPCODE_GPIO_SET_PULL_UP> response;
		response.success = pico_hub_gpio_set_pull_up(request->pin, request->value);

		return pico_hub_io_send_response(response);
	}

	return false;
}
bool             pico_hub_packet_handler_gpio_get_pull_down(const void* buffer, size_t size)
{
	if (auto request = pico_hub_io_get_request<PICO_HUB_OPCODE_GPIO_GET_PULL_DOWN>(buffer, size))
	{
		pico_hub_packet_response<PICO_HUB_OPCODE_GPIO_GET_PULL_DOWN> response;
		response.success = pico_hub_gpio_get_pull_down(request->pin, &response.value);

		return pico_hub_io_send_response(response);
	}

	return false;
}
bool             pico_hub_packet_handler_gpio_set_pull_down(const void* buffer, size_t size)
{
	if (auto request = pico_hub_io_get_request<PICO_HUB_OPCODE_GPIO_SET_PULL_DOWN>(buffer, size))
	{
		pico_hub_packet_response<PICO_HUB_OPCODE_GPIO_SET_PULL_DOWN> response;
		response.success = pico_hub_gpio_set_pull_down(request->pin, request->value);

		return pico_hub_io_send_response(response);
	}

	return false;
}
bool             pico_hub_packet_handler_gpio_get_direction(const void* buffer, size_t size)
{
	if (auto request = pico_hub_io_get_request<PICO_HUB_OPCODE_GPIO_GET_DIRECTION>(buffer, size))
	{
		pico_hub_packet_response<PICO_HUB_OPCODE_GPIO_GET_DIRECTION> response;
		response.success = pico_hub_gpio_get_direction(request->pin, &response.value);

		return pico_hub_io_send_response(response);
	}

	return false;
}
bool             pico_hub_packet_handler_gpio_set_direction(const void* buffer, size_t size)
{
	if (auto request = pico_hub_io_get_request<PICO_HUB_OPCODE_GPIO_SET_DIRECTION>(buffer, size))
	{
		pico_hub_packet_response<PICO_HUB_OPCODE_GPIO_SET_DIRECTION> response;
		response.success = pico_hub_gpio_set_direction(request->pin, request->value);

		return pico_hub_io_send_response(response);
	}

	return false;
}
bool             pico_hub_packet_handler_gpio_get_drive_strength(const void* buffer, size_t size)
{
	if (auto request = pico_hub_io_get_request<PICO_HUB_OPCODE_GPIO_GET_DRIVE_STRENGTH>(buffer, size))
	{
		int drive_strength;

		pico_hub_packet_response<PICO_HUB_OPCODE_GPIO_GET_DRIVE_STRENGTH> response;
		response.success = pico_hub_gpio_get_drive_strength(request->pin, &drive_strength);
		response.value   = (uint8_t)drive_strength;

		return pico_hub_io_send_response(response);
	}

	return false;
}
bool             pico_hub_packet_handler_gpio_set_drive_strength(const void* buffer, size_t size)
{
	if (auto request = pico_hub_io_get_request<PICO_HUB_OPCODE_GPIO_SET_DRIVE_STRENGTH>(buffer, size))
	{
		pico_hub_packet_response<PICO_HUB_OPCODE_GPIO_SET_DRIVE_STRENGTH> response;
		response.success = pico_hub_gpio_set_drive_strength(request->pin, request->value);

		return pico_hub_io_send_response(response);
	}

	return false;
}
bool             pico_hub_packet_handler_gpio_read(const void* buffer, size_t size)
{
	if (auto request = pico_hub_io_get_request<PICO_HUB_OPCODE_GPIO_READ>(buffer, size))
	{
		pico_hub_packet_response<PICO_HUB_OPCODE_GPIO_READ> response;
		response.success = pico_hub_gpio_write(request->pin, &response.value);

		return pico_hub_io_send_response(response);
	}

	return false;
}
bool             pico_hub_packet_handler_gpio_write(const void* buffer, size_t size)
{
	if (auto request = pico_hub_io_get_request<PICO_HUB_OPCODE_GPIO_WRITE>(buffer, size))
	{
		pico_hub_packet_response<PICO_HUB_OPCODE_GPIO_WRITE> response;
		response.success = pico_hub_gpio_write(request->pin, request->value);

		return pico_hub_io_send_response(response);
	}

	return false;
}
bool             pico_hub_packet_handler_uart_init(const void* buffer, size_t size)
{
	if (auto request = pico_hub_io_get_request<PICO_HUB_OPCODE_UART_INIT>(buffer, size))
	{
		pico_hub_packet_response<PICO_HUB_OPCODE_UART_INIT> response;
		response.success = pico_hub_uart_init(request->bus, request->rx, request->tx, request->baud);

		return pico_hub_io_send_response(response);
	}

	return false;
}
bool             pico_hub_packet_handler_uart_deinit(const void* buffer, size_t size)
{
	if (auto request = pico_hub_io_get_request<PICO_HUB_OPCODE_UART_DEINIT>(buffer, size))
	{
		pico_hub_uart_deinit(request->bus);

		pico_hub_packet_response<PICO_HUB_OPCODE_UART_DEINIT> response;

		return pico_hub_io_send_response(response);
	}

	return false;
}
bool             pico_hub_packet_handler_uart_read(const void* buffer, size_t size)
{
	if (auto request = pico_hub_io_get_request<PICO_HUB_OPCODE_UART_READ>(buffer, size))
	{
		std::vector<uint8_t> data(request->size, 0);
		pico_hub_packet_response<PICO_HUB_OPCODE_UART_READ> response;
		response.success = pico_hub_uart_read(request->bus, &data[0], request->size);

		return pico_hub_io_send_response(response) && (!response.success || pico_hub_io_send(&data[0], request->size));
	}

	return false;
}
bool             pico_hub_packet_handler_uart_write(const void* buffer, size_t size)
{
	if (auto request = pico_hub_io_get_request<PICO_HUB_OPCODE_UART_WRITE>(buffer, size))
	{
		std::vector<uint8_t> data(request->size, 0);

		if (pico_hub_io_receive(&data[0], request->size))
		{
			pico_hub_packet_response<PICO_HUB_OPCODE_UART_WRITE> response =
			{
				.success = pico_hub_uart_write(request->bus, &data[0], request->size)
			};

			return pico_hub_io_send_response(response);
		}
	}

	return false;
}
