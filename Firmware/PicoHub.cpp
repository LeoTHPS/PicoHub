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

#ifdef LIB_PICO_CYW43_ARCH
	#include <pico/cyw43_arch.h>
#endif

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

static_assert(PICO_HUB_PWM_CHANNEL_A            == (int)PWM_CHAN_A);
static_assert(PICO_HUB_PWM_CHANNEL_B            == (int)PWM_CHAN_B);

// static_assert(PICO_HUB_GPIO_PULL_UP             == );
// static_assert(PICO_HUB_GPIO_PULL_DOWN           == );
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
bool pico_hub_packet_handler_get_pinout(const void* buffer, size_t size);
bool pico_hub_packet_handler_get_latency(const void* buffer, size_t size);
bool pico_hub_packet_handler_get_voltage(const void* buffer, size_t size);
bool pico_hub_packet_handler_set_voltage(const void* buffer, size_t size);
bool pico_hub_packet_handler_get_features(const void* buffer, size_t size);
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
bool pico_hub_packet_handler_pwm_get_slice_and_channel(const void* buffer, size_t size);
bool pico_hub_packet_handler_pwm_init(const void* buffer, size_t size);
bool pico_hub_packet_handler_pwm_deinit(const void* buffer, size_t size);
bool pico_hub_packet_handler_pwm_get_wrap(const void* buffer, size_t size);
bool pico_hub_packet_handler_pwm_get_level(const void* buffer, size_t size);
bool pico_hub_packet_handler_pwm_get_clkdiv(const void* buffer, size_t size);
bool pico_hub_packet_handler_pwm_get_enabled(const void* buffer, size_t size);
bool pico_hub_packet_handler_pwm_set_wrap(const void* buffer, size_t size);
bool pico_hub_packet_handler_pwm_set_level(const void* buffer, size_t size);
bool pico_hub_packet_handler_pwm_set_clkdiv(const void* buffer, size_t size);
bool pico_hub_packet_handler_pwm_set_enabled(const void* buffer, size_t size);
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
bool pico_hub_packet_handler_wifi_scan(const void* buffer, size_t size);
bool pico_hub_packet_handler_wifi_ap_open(const void* buffer, size_t size);
bool pico_hub_packet_handler_wifi_ap_close(const void* buffer, size_t size);
bool pico_hub_packet_handler_wifi_station_connect(const void* buffer, size_t size);
bool pico_hub_packet_handler_wifi_station_disconnect(const void* buffer, size_t size);

constexpr const pico_hub_packet_context pico_hub_packet_handlers[PICO_HUB_OPCODES_COUNT] =
{
	{ PICO_HUB_OPCODE_GET_ID,                    &pico_hub_packet_handler_get_id },
	{ PICO_HUB_OPCODE_GET_LED,                   &pico_hub_packet_handler_get_led },
	{ PICO_HUB_OPCODE_SET_LED,                   &pico_hub_packet_handler_set_led },
	{ PICO_HUB_OPCODE_GET_CLOCK,                 &pico_hub_packet_handler_get_clock },
	{ PICO_HUB_OPCODE_SET_CLOCK,                 &pico_hub_packet_handler_set_clock },
	{ PICO_HUB_OPCODE_GET_PINOUT,                &pico_hub_packet_handler_get_pinout },
	{ PICO_HUB_OPCODE_GET_LATENCY,               &pico_hub_packet_handler_get_latency },
	{ PICO_HUB_OPCODE_GET_VOLTAGE,               &pico_hub_packet_handler_get_voltage },
	{ PICO_HUB_OPCODE_SET_VOLTAGE,               &pico_hub_packet_handler_set_voltage },
	{ PICO_HUB_OPCODE_RESTART,                   &pico_hub_packet_handler_restart },
	{ PICO_HUB_OPCODE_SHUTDOWN,                  &pico_hub_packet_handler_shutdown },

	{ PICO_HUB_OPCODE_ADC_INIT,                  &pico_hub_packet_handler_adc_init },
	{ PICO_HUB_OPCODE_ADC_DEINIT,                &pico_hub_packet_handler_adc_deinit },
	{ PICO_HUB_OPCODE_ADC_GET_CHANNEL,           &pico_hub_packet_handler_adc_get_channel },
	{ PICO_HUB_OPCODE_ADC_SET_CHANNEL,           &pico_hub_packet_handler_adc_set_channel },
	{ PICO_HUB_OPCODE_ADC_READ,                  &pico_hub_packet_handler_adc_read },

	{ PICO_HUB_OPCODE_I2C_INIT,                  &pico_hub_packet_handler_i2c_init },
	{ PICO_HUB_OPCODE_I2C_DEINIT,                &pico_hub_packet_handler_i2c_deinit },
	{ PICO_HUB_OPCODE_I2C_READ,                  &pico_hub_packet_handler_i2c_read },
	{ PICO_HUB_OPCODE_I2C_WRITE,                 &pico_hub_packet_handler_i2c_write },
	{ PICO_HUB_OPCODE_I2C_WRITE_READ,            &pico_hub_packet_handler_i2c_write_read },

	{ PICO_HUB_OPCODE_PWM_GET_SLICE_AND_CHANNEL, &pico_hub_packet_handler_pwm_get_slice_and_channel },
	{ PICO_HUB_OPCODE_PWM_INIT,                  &pico_hub_packet_handler_pwm_init },
	{ PICO_HUB_OPCODE_PWM_DEINIT,                &pico_hub_packet_handler_pwm_deinit },
	{ PICO_HUB_OPCODE_PWM_GET_WRAP,              &pico_hub_packet_handler_pwm_get_wrap },
	{ PICO_HUB_OPCODE_PWM_GET_LEVEL,             &pico_hub_packet_handler_pwm_get_level },
	{ PICO_HUB_OPCODE_PWM_GET_CLKDIV,            &pico_hub_packet_handler_pwm_get_clkdiv },
	{ PICO_HUB_OPCODE_PWM_GET_ENABLED,           &pico_hub_packet_handler_pwm_get_enabled },
	{ PICO_HUB_OPCODE_PWM_SET_WRAP,              &pico_hub_packet_handler_pwm_set_wrap },
	{ PICO_HUB_OPCODE_PWM_SET_LEVEL,             &pico_hub_packet_handler_pwm_set_level },
	{ PICO_HUB_OPCODE_PWM_SET_CLKDIV,            &pico_hub_packet_handler_pwm_set_clkdiv },
	{ PICO_HUB_OPCODE_PWM_SET_ENABLED,           &pico_hub_packet_handler_pwm_set_enabled },

	{ PICO_HUB_OPCODE_SPI_INIT,                  &pico_hub_packet_handler_spi_init },
	{ PICO_HUB_OPCODE_SPI_DEINIT,                &pico_hub_packet_handler_spi_deinit },
	{ PICO_HUB_OPCODE_SPI_READ,                  &pico_hub_packet_handler_spi_read },
	{ PICO_HUB_OPCODE_SPI_WRITE,                 &pico_hub_packet_handler_spi_write },
	{ PICO_HUB_OPCODE_SPI_WRITE_READ,            &pico_hub_packet_handler_spi_write_read },

	{ PICO_HUB_OPCODE_GPIO_INIT,                 &pico_hub_packet_handler_gpio_init },
	{ PICO_HUB_OPCODE_GPIO_DEINIT,               &pico_hub_packet_handler_gpio_deinit },
	{ PICO_HUB_OPCODE_GPIO_GET_PULL_UP,          &pico_hub_packet_handler_gpio_get_pull_up },
	{ PICO_HUB_OPCODE_GPIO_SET_PULL_UP,          &pico_hub_packet_handler_gpio_set_pull_up },
	{ PICO_HUB_OPCODE_GPIO_GET_PULL_DOWN,        &pico_hub_packet_handler_gpio_get_pull_down },
	{ PICO_HUB_OPCODE_GPIO_SET_PULL_DOWN,        &pico_hub_packet_handler_gpio_set_pull_down },
	{ PICO_HUB_OPCODE_GPIO_GET_DIRECTION,        &pico_hub_packet_handler_gpio_get_direction },
	{ PICO_HUB_OPCODE_GPIO_SET_DIRECTION,        &pico_hub_packet_handler_gpio_set_direction },
	{ PICO_HUB_OPCODE_GPIO_GET_DRIVE_STRENGTH,   &pico_hub_packet_handler_gpio_get_drive_strength },
	{ PICO_HUB_OPCODE_GPIO_SET_DRIVE_STRENGTH,   &pico_hub_packet_handler_gpio_set_drive_strength },
	{ PICO_HUB_OPCODE_GPIO_READ,                 &pico_hub_packet_handler_gpio_read },
	{ PICO_HUB_OPCODE_GPIO_WRITE,                &pico_hub_packet_handler_gpio_write },

	{ PICO_HUB_OPCODE_UART_INIT,                 &pico_hub_packet_handler_uart_init },
	{ PICO_HUB_OPCODE_UART_DEINIT,               &pico_hub_packet_handler_uart_deinit },
	{ PICO_HUB_OPCODE_UART_READ,                 &pico_hub_packet_handler_uart_read },
	{ PICO_HUB_OPCODE_UART_WRITE,                &pico_hub_packet_handler_uart_write },

	{ PICO_HUB_OPCODE_WIFI_SCAN,                 &pico_hub_packet_handler_wifi_scan },

	{ PICO_HUB_OPCODE_WIFI_AP_OPEN,              &pico_hub_packet_handler_wifi_ap_open },
	{ PICO_HUB_OPCODE_WIFI_AP_CLOSE,             &pico_hub_packet_handler_wifi_ap_close },

	{ PICO_HUB_OPCODE_WIFI_STATION_CONNECT,      &pico_hub_packet_handler_wifi_station_connect },
	{ PICO_HUB_OPCODE_WIFI_STATION_DISCONNECT,   &pico_hub_packet_handler_wifi_station_disconnect }
};

template<size_t ... I>
consteval bool static_assert_pico_hub_packet_handlers(std::index_sequence<I ...>)
{
	return ((pico_hub_packet_handlers[I].opcode == I) && ...);
}
static_assert(static_assert_pico_hub_packet_handlers(std::make_index_sequence<PICO_HUB_OPCODES_COUNT> {}));

struct pico_hub_adc
{
	PICO_HUB_ADC channel;  // selected channel(s)
	PICO_HUB_ADC channels; // initialized channel(s)
};

struct pico_hub_i2c
{
	bool        is_slave;
	bool        is_initialized;

	i2c_inst_t* bus;
	uint32_t    baud;
	uint8_t     address;

	uint8_t     pin_scl;
	uint8_t     pin_sda;
};

struct pico_hub_pwm_channel
{
	uint8_t  pin;
	uint16_t level;
};
struct pico_hub_pwm
{
	bool                 is_enabled;
	bool                 is_initialized;

	uint16_t             wrap;
	float                clkdiv;
	pico_hub_pwm_channel channels[PICO_HUB_PWM_CHANNELS_COUNT];
};

struct pico_hub_spi
{
	bool        is_slave;
	bool        is_initialized;

	spi_inst_t* bus;
	uint32_t    baud;

	uint8_t     pin_cs;
	uint8_t     pin_miso;
	uint8_t     pin_mosi;
	uint8_t     pin_sclk;
};

struct pico_hub_gpio
{
	bool    is_low;
	bool    is_input;
	bool    is_initialized;

	uint8_t pin;
	int     function;
};

struct pico_hub_uart
{
	bool         is_initialized;

	uart_inst_t* bus;
	uint32_t     baud;

	uint8_t      pin_rx;
	uint8_t      pin_tx;
};

struct pico_hub_wifi
{
	struct
	{
		bool               is_open;

		PICO_HUB_WIFI_AUTH auth;
		std::string        ssid;
		std::string        passwd;
		uint8_t            channel;
	} ap;

	struct
	{
		bool               is_open;
		bool               is_connected;

		PICO_HUB_WIFI_AUTH auth;
		std::string        ssid;
		std::string        passwd;
	} station;
};

struct
{
	bool                   is_created   = false;
	bool                   is_running   = false;
	bool                   is_stopping  = false;
	bool                   is_connected = false;

	pico_hub_adc           adc;
	pico_hub_i2c           i2c[PICO_HUB_I2C_BUS_COUNT];
	pico_hub_pwm           pwm[PICO_HUB_PWM_SLICES_COUNT];
	pico_hub_spi           spi[PICO_HUB_SPI_BUS_COUNT];
	pico_hub_gpio          gpio[PICO_HUB_GPIO_COUNT];
	pico_hub_uart          uart[PICO_HUB_UART_BUS_COUNT];
	pico_hub_wifi          wifi;

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

bool pico_hub_debug_blink(uint32_t count = 1, uint32_t delay = 500)
{
	bool state;

	if (!pico_hub_get_led(&state))
		return false;

	for (uint32_t i = 0; i < count; )
	{
		if (!pico_hub_set_led(!state))
			return false;

		sleep_ms(delay);

		if (!pico_hub_set_led(state))
			return false;

		if (++i < count)
			sleep_ms(delay);
	}

	return true;
}

auto pico_hub_wifi_auth_to_cyw43(PICO_HUB_WIFI_AUTH value)
{
	switch (value)
	{
		case PICO_HUB_WIFI_AUTH_OPEN:              return CYW43_AUTH_OPEN;
		case PICO_HUB_WIFI_AUTH_WPA_TKIP_PSK:      return CYW43_AUTH_WPA_TKIP_PSK;
		case PICO_HUB_WIFI_AUTH_WPA2_AES_PSK:      return CYW43_AUTH_WPA2_AES_PSK;
		case PICO_HUB_WIFI_AUTH_WPA2_MIXED_PSK:    return CYW43_AUTH_WPA2_MIXED_PSK;
		case PICO_HUB_WIFI_AUTH_WPA3_SAE_AES_PSK:  return CYW43_AUTH_WPA3_SAE_AES_PSK;
		case PICO_HUB_WIFI_AUTH_WPA3_WPA2_AES_PSK: return CYW43_AUTH_WPA3_WPA2_AES_PSK;
	}

	return CYW43_AUTH_OPEN;
}
auto pico_hub_wifi_auth_from_cyw43(int value)
{
	switch (value)
	{
		case CYW43_AUTH_OPEN:              return PICO_HUB_WIFI_AUTH_OPEN;
		case CYW43_AUTH_WPA_TKIP_PSK:      return PICO_HUB_WIFI_AUTH_WPA_TKIP_PSK;
		case CYW43_AUTH_WPA2_AES_PSK:      return PICO_HUB_WIFI_AUTH_WPA2_AES_PSK;
		case CYW43_AUTH_WPA2_MIXED_PSK:    return PICO_HUB_WIFI_AUTH_WPA2_MIXED_PSK;
		case CYW43_AUTH_WPA3_SAE_AES_PSK:  return PICO_HUB_WIFI_AUTH_WPA3_SAE_AES_PSK;
		case CYW43_AUTH_WPA3_WPA2_AES_PSK: return PICO_HUB_WIFI_AUTH_WPA3_WPA2_AES_PSK;
	}

	return PICO_HUB_WIFI_AUTH_OPEN;
}

bool             pico_hub_init()
{
	assert(!pico_hub.is_created);

	memset(&pico_hub.adc, 0, sizeof(pico_hub_adc));
	pico_hub.i2c[PICO_HUB_I2C_BUS_0] = { .bus = i2c0 };
	pico_hub.i2c[PICO_HUB_I2C_BUS_1] = { .bus = i2c1 };
	memset(pico_hub.pwm, 0, sizeof(pico_hub_pwm) * PICO_HUB_PWM_SLICES_COUNT);
	pico_hub.spi[PICO_HUB_SPI_BUS_0] = { .bus = spi0 };
	pico_hub.spi[PICO_HUB_SPI_BUS_1] = { .bus = spi1 };
	memset(pico_hub.gpio, 0, sizeof(pico_hub_gpio) * PICO_HUB_GPIO_COUNT);
	pico_hub.uart[PICO_HUB_UART_BUS_0] = { .bus = uart0 };
	pico_hub.uart[PICO_HUB_UART_BUS_1] = { .bus = uart1 };

	stdio_init_all();
	stdio_set_translate_crlf(&stdio_usb, false);

	pico_unique_board_id_t device_id;
	pico_get_unique_board_id(&device_id);
	memcpy(&pico_hub.device_id, device_id.id, PICO_UNIQUE_BOARD_ID_SIZE_BYTES);
	static_assert(sizeof(pico_hub.device_id == PICO_UNIQUE_BOARD_ID_SIZE_BYTES));

	set_sys_clock_hz(SYS_CLK_HZ, true);
	pico_hub.device_clock = SYS_CLK_HZ;

	vreg_set_voltage(VREG_VOLTAGE_DEFAULT);
	pico_hub.device_voltage = PICO_HUB_VOLTAGE_DEFAULT;

	pico_hub.is_created   = true;
	pico_hub.is_running   = false;
	pico_hub.is_stopping  = false;
	pico_hub.is_connected = false;

#ifdef LIB_PICO_CYW43_ARCH
	cyw43_arch_init_with_country(CYW43_COUNTRY_USA); // TODO: country code
	cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, true);
#else
	pico_hub_gpio_init(PICO_HUB_GPIO_25, true, true);
#endif

	return true;
}
void             pico_hub_deinit()
{
	if (pico_hub.is_created)
	{
		pico_hub_adc_deinit();

		for (int slice = 0; slice < PICO_HUB_PWM_SLICES_COUNT; ++slice)
			pico_hub_pwm_deinit(slice);

		for (int i2c = 0; i2c < PICO_HUB_I2C_BUS_COUNT; ++i2c)
			pico_hub_i2c_deinit((PICO_HUB_I2C)i2c);

		for (int spi = 0; spi < PICO_HUB_SPI_BUS_COUNT; ++spi)
			pico_hub_spi_deinit((PICO_HUB_SPI)spi);

		for (int gpio = 0; gpio < PICO_HUB_GPIO_COUNT; ++gpio)
#ifndef LIB_PICO_CYW43_ARCH
			if (gpio != PICO_HUB_GPIO_25)
#endif
				pico_hub_gpio_deinit((PICO_HUB_GPIO)gpio);

		for (int uart = 0; uart < PICO_HUB_UART_BUS_COUNT; ++uart)
			pico_hub_uart_deinit((PICO_HUB_UART)uart);

		pico_hub_set_led(false);
#ifndef LIB_PICO_CYW43_ARCH
		pico_hub_gpio_deinit(PICO_HUB_GPIO_25);
#endif

#ifdef LIB_PICO_CYW43_ARCH
		pico_hub_wifi_ap_close();
		pico_hub_wifi_station_disconnect();

		cyw43_arch_deinit();
#endif

		stdio_flush();
		stdio_deinit_all();

		pico_hub.is_created = false;
	}
}

bool             pico_hub_poll()
{
#if defined(LIB_PICO_CYW43_ARCH) && CYW43_LWIP
	if (pico_hub.wifi.ap.is_open)
	{
		cyw43_arch_lwip_begin();

		// TODO: update connected station list

		cyw43_arch_lwip_end();
	}

	if (pico_hub.wifi.station.is_open)
	{
		cyw43_arch_lwip_begin();

		auto status = cyw43_wifi_link_status(&cyw43_state, CYW43_ITF_STA);

		cyw43_arch_lwip_end();

		switch (status)
		{
			case CYW43_LINK_UP:
			case CYW43_LINK_JOIN:
			case CYW43_LINK_NOIP:
				pico_hub.wifi.station.is_connected = true;
				break;

			case CYW43_LINK_DOWN:
			case CYW43_LINK_FAIL:
			case CYW43_LINK_NONET:
				pico_hub.wifi.station.is_connected = false;
				if (!cyw43_arch_wifi_connect_blocking(pico_hub.wifi.station.ssid.c_str(), pico_hub.wifi.station.passwd.c_str(), pico_hub_wifi_auth_to_cyw43(pico_hub.wifi.station.auth)))
					pico_hub.wifi.station.is_connected = true;
				break;

			case CYW43_LINK_BADAUTH:
				pico_hub_wifi_station_disconnect();
				break;
		}
	}
#endif

	if (!pico_hub_io_receive_and_execute_request())
		return false;

	return true;
}
bool             pico_hub_run()
{
	assert(pico_hub.is_created);
	assert(!pico_hub.is_running);

	pico_hub.is_running = true;

	bool success = true;

	while (!pico_hub.is_stopping)
		if (!(success = pico_hub_poll()))
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
	if (!pico_hub.is_created)
		return false;

#ifndef LIB_PICO_CYW43_ARCH
	return pico_hub_gpio_read(PICO_HUB_GPIO_25, value);
#else
	return cyw43_arch_gpio_get(CYW43_WL_GPIO_LED_PIN);
#endif
}
bool             pico_hub_set_led(bool value)
{
	assert(pico_hub.is_created);

#ifndef LIB_PICO_CYW43_ARCH
	return pico_hub_gpio_write(PICO_HUB_GPIO_25, value);
#else
	cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, value);

	return true;
#endif
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

void             pico_hub_get_pinout_adc(pico_hub_pinout* value)
{
	value->adc.channels = pico_hub.adc.channels;
}
void             pico_hub_get_pinout_i2c(pico_hub_pinout* value)
{
	static_assert((sizeof(pico_hub_pinout::i2c) / sizeof(pico_hub_pinout::i2c[0])) == PICO_HUB_I2C_BUS_COUNT);

	for (int bus = 0; bus < PICO_HUB_I2C_BUS_COUNT; ++bus)
	{
		if (!pico_hub.i2c[bus].is_initialized)
			continue;

		value->i2c[bus] =
		{
			.is_open = true,

			.baud    = pico_hub.i2c[bus].baud,
			.slave   = pico_hub.i2c[bus].is_slave,
			.address = pico_hub.i2c[bus].address,
			.pin_scl = pico_hub.i2c[bus].pin_scl,
			.pin_sda = pico_hub.i2c[bus].pin_sda
		};
	}
}
void             pico_hub_get_pinout_pwm(pico_hub_pinout* value)
{
	static_assert((sizeof(pico_hub_pinout::pwm) / sizeof(pico_hub_pinout::pwm[0])) == PICO_HUB_PWM_SLICES_COUNT);

	for (int slice = 0; slice < PICO_HUB_PWM_SLICES_COUNT; ++slice)
	{
		if (!pico_hub.pwm[slice].is_initialized)
			continue;

		value->pwm[slice] =
		{
			.is_open    = true,
			.is_enabled = pico_hub.pwm[slice].is_enabled,

			.wrap       = pico_hub.pwm[slice].wrap,
			.clkdiv     = pico_hub.pwm[slice].clkdiv,
			.channels   =
			{
				{
					.pin   = pico_hub.pwm[slice].channels[PICO_HUB_PWM_CHANNEL_A].pin,
					.level = pico_hub.pwm[slice].channels[PICO_HUB_PWM_CHANNEL_A].level
				},
				{
					.pin   = pico_hub.pwm[slice].channels[PICO_HUB_PWM_CHANNEL_B].pin,
					.level = pico_hub.pwm[slice].channels[PICO_HUB_PWM_CHANNEL_B].level
				}
			}
		};
	}
}
void             pico_hub_get_pinout_spi(pico_hub_pinout* value)
{
	static_assert((sizeof(pico_hub_pinout::spi) / sizeof(pico_hub_pinout::spi[0])) == PICO_HUB_SPI_BUS_COUNT);

	for (int bus = 0; bus < PICO_HUB_SPI_BUS_COUNT; ++bus)
	{
		if (!pico_hub.spi[bus].is_initialized)
			continue;

		value->spi[bus] =
		{
			.is_open   = true,

			.baud      = pico_hub.spi[bus].baud,
			.slave     = pico_hub.spi[bus].is_slave,
			.pin_cs    = pico_hub.spi[bus].pin_cs,
			.pin_miso  = pico_hub.spi[bus].pin_miso,
			.pin_mosi  = pico_hub.spi[bus].pin_mosi,
			.pin_clock = pico_hub.spi[bus].pin_sclk 
		};
	}
}
void             pico_hub_get_pinout_gpio(pico_hub_pinout* value)
{
	static_assert((sizeof(pico_hub_pinout::gpio) / sizeof(pico_hub_pinout::gpio[0])) == PICO_HUB_GPIO_COUNT);

	for (int pin = 0; pin < PICO_HUB_GPIO_COUNT; ++pin)
	{
		if (!pico_hub.gpio[pin].is_initialized)
			continue;

		int flags = 0;

		if (pico_hub.gpio[pin].function == PICO_HUB_GPIO_FUNCTION_GPIO)
		{
			if (pico_hub.gpio[pin].is_low)
				flags |= PICO_HUB_GPIO_FLAG_VALUE_LOW;
			else
				flags |= PICO_HUB_GPIO_FLAG_VALUE_HIGH;

			if (pico_hub.gpio[pin].is_input)
				flags |= PICO_HUB_GPIO_FLAG_DIRECTION_IN;
			else
				flags |= PICO_HUB_GPIO_FLAG_DIRECTION_OUT;
		}

		switch (pico_hub.gpio[pin].function)
		{
			case PICO_HUB_GPIO_FUNCTION_NONE: break;
			case PICO_HUB_GPIO_FUNCTION_ADC:  flags |= PICO_HUB_GPIO_FLAG_FUNCTION_ADC;  break;
			case PICO_HUB_GPIO_FUNCTION_I2C:  flags |= PICO_HUB_GPIO_FLAG_FUNCTION_I2C;  break;
			case PICO_HUB_GPIO_FUNCTION_PWM:  flags |= PICO_HUB_GPIO_FLAG_FUNCTION_PWM;  break;
			case PICO_HUB_GPIO_FUNCTION_SPI:  flags |= PICO_HUB_GPIO_FLAG_FUNCTION_SPI;  break;
			case PICO_HUB_GPIO_FUNCTION_GPIO: flags |= PICO_HUB_GPIO_FLAG_FUNCTION_GPIO; break;
			case PICO_HUB_GPIO_FUNCTION_UART: flags |= PICO_HUB_GPIO_FLAG_FUNCTION_UART; break;
		}

		value->gpio[pin] =
		{
			.is_open = true,
			.flags   = (PICO_HUB_GPIO_FLAGS)flags
		};
	}
}
void             pico_hub_get_pinout_uart(pico_hub_pinout* value)
{
	static_assert((sizeof(pico_hub_pinout::uart) / sizeof(pico_hub_pinout::uart[0])) == PICO_HUB_UART_BUS_COUNT);

	for (int bus = 0; bus < PICO_HUB_UART_BUS_COUNT; ++bus)
	{
		if (!pico_hub.uart[bus].is_initialized)
			continue;

		value->uart[bus] =
		{
			.is_open = true,

			.baud    = pico_hub.uart[bus].baud,
			.pin_rx  = pico_hub.uart[bus].pin_rx,
			.pin_tx  = pico_hub.uart[bus].pin_tx
		};
	}
}
void             pico_hub_get_pinout(pico_hub_pinout* value)
{
	memset(value, 0, sizeof(pico_hub_pinout));

	pico_hub_get_pinout_adc(value);
	pico_hub_get_pinout_i2c(value);
	pico_hub_get_pinout_pwm(value);
	pico_hub_get_pinout_spi(value);
	pico_hub_get_pinout_gpio(value);
	pico_hub_get_pinout_uart(value);
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

	if (!pico_hub.adc.channels)
	{
		adc_init();

		pico_hub.adc.channel = (PICO_HUB_ADC)0;
	}

	if (channels & PICO_HUB_ADC_GPIO_26)
	{
		if (pico_hub_gpio_is_in_use(26))
			return false;

		adc_gpio_init(26);
		pico_hub_gpio_set_function(26, PICO_HUB_GPIO_FUNCTION_ADC);
	}

	if (channels & PICO_HUB_ADC_GPIO_27)
	{
		if (pico_hub_gpio_is_in_use(27))
		{
			if (channels & PICO_HUB_ADC_GPIO_26)
			{
				gpio_deinit(26);
				pico_hub_gpio_set_function(26, PICO_HUB_GPIO_FUNCTION_NONE);
			}

			return false;
		}

		adc_gpio_init(27);
		pico_hub_gpio_set_function(27, PICO_HUB_GPIO_FUNCTION_ADC);
	}

	if (channels & PICO_HUB_ADC_GPIO_28)
	{
		if (pico_hub_gpio_is_in_use(28))
		{
			if (channels & PICO_HUB_ADC_GPIO_27)
			{
				gpio_deinit(27);
				pico_hub_gpio_set_function(27, PICO_HUB_GPIO_FUNCTION_NONE);
			}

			if (channels & PICO_HUB_ADC_GPIO_26)
			{
				gpio_deinit(26);
				pico_hub_gpio_set_function(26, PICO_HUB_GPIO_FUNCTION_NONE);
			}

			return false;
		}

		adc_gpio_init(28);
		pico_hub_gpio_set_function(28, PICO_HUB_GPIO_FUNCTION_ADC);
	}

	if (channels & PICO_HUB_ADC_SYSTEM_VOLTAGE)
	{
		if (pico_hub_gpio_is_in_use(29))
		{
			if (channels & PICO_HUB_ADC_GPIO_28)
			{
				gpio_deinit(28);
				pico_hub_gpio_set_function(28, PICO_HUB_GPIO_FUNCTION_NONE);
			}

			if (channels & PICO_HUB_ADC_GPIO_27)
			{
				gpio_deinit(27);
				pico_hub_gpio_set_function(27, PICO_HUB_GPIO_FUNCTION_NONE);
			}

			if (channels & PICO_HUB_ADC_GPIO_26)
			{
				gpio_deinit(26);
				pico_hub_gpio_set_function(26, PICO_HUB_GPIO_FUNCTION_NONE);
			}

			return false;
		}

		adc_gpio_init(29);
		pico_hub_gpio_set_function(29, PICO_HUB_GPIO_FUNCTION_ADC);
	}

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
		{
			gpio_deinit(26);
			pico_hub_gpio_set_function(26, PICO_HUB_GPIO_FUNCTION_NONE);
		}

		if (channels & PICO_HUB_ADC_GPIO_27)
		{
			gpio_deinit(27);
			pico_hub_gpio_set_function(27, PICO_HUB_GPIO_FUNCTION_NONE);
		}

		if (channels & PICO_HUB_ADC_GPIO_28)
		{
			gpio_deinit(28);
			pico_hub_gpio_set_function(28, PICO_HUB_GPIO_FUNCTION_NONE);
		}

		if (channels & PICO_HUB_ADC_SYSTEM_VOLTAGE)
		{
			gpio_deinit(29);
			pico_hub_gpio_set_function(29, PICO_HUB_GPIO_FUNCTION_NONE);
		}

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

bool             pico_hub_i2c_init(PICO_HUB_I2C bus, uint8_t scl, uint8_t sda, uint32_t baud, uint8_t address, bool slave)
{
	assert(pico_hub.is_created);

	if (bus >= PICO_HUB_I2C_BUS_COUNT)
		return false;

	auto i2c = &pico_hub.i2c[bus];

	if (i2c->is_initialized)
		return false;

	if (pico_hub_gpio_is_in_use(scl) || pico_hub_gpio_is_in_use(sda))
		return false;

	baud = i2c_init(i2c->bus, baud);
	gpio_set_function(sda, GPIO_FUNC_I2C);
	gpio_set_function(scl, GPIO_FUNC_I2C);
	gpio_pull_up(sda);
	gpio_pull_up(scl);

	if (slave)
		i2c_set_slave_mode(i2c->bus, slave, address);

	i2c->baud           = baud;
	i2c->pin_scl        = scl;
	i2c->pin_sda        = sda;
	i2c->address        = address;
	i2c->is_slave       = slave;
	i2c->is_initialized = true;

	pico_hub_gpio_set_function(scl, PICO_HUB_GPIO_FUNCTION_I2C);
	pico_hub_gpio_set_function(sda, PICO_HUB_GPIO_FUNCTION_I2C);

	return true;
}
void             pico_hub_i2c_deinit(PICO_HUB_I2C bus)
{
	assert(pico_hub.is_created);

	if (bus < PICO_HUB_I2C_BUS_COUNT)
		if (auto i2c = &pico_hub.i2c[bus]; i2c->is_initialized)
		{
			i2c_deinit(i2c->bus);

			pico_hub_gpio_set_function(i2c->pin_scl, PICO_HUB_GPIO_FUNCTION_NONE);
			pico_hub_gpio_set_function(i2c->pin_sda, PICO_HUB_GPIO_FUNCTION_NONE);

			i2c->is_initialized = false;
		}
}
bool             pico_hub_i2c_read(PICO_HUB_I2C bus, uint8_t address, void* buffer, size_t size, bool stop)
{
	assert(pico_hub.is_created);

	if (bus < PICO_HUB_I2C_BUS_COUNT)
		if (auto i2c = &pico_hub.i2c[bus]; i2c->is_initialized)
			return i2c_read_blocking(i2c->bus, address ? address : i2c->address, (uint8_t*)buffer, size, !stop) != PICO_ERROR_GENERIC;

	return false;
}
bool             pico_hub_i2c_write(PICO_HUB_I2C bus, uint8_t address, const void* buffer, size_t size, bool stop)
{
	assert(pico_hub.is_created);

	if (bus < PICO_HUB_I2C_BUS_COUNT)
		if (auto i2c = &pico_hub.i2c[bus]; i2c->is_initialized)
			return i2c_write_blocking(i2c->bus, address ? address : i2c->address, (uint8_t*)buffer, size, !stop) != PICO_ERROR_GENERIC;

	return false;
}
bool             pico_hub_i2c_write_read(PICO_HUB_I2C bus, uint8_t address, const void* tx, size_t tx_size, void* rx, size_t rx_size)
{
	assert(pico_hub.is_created);

	if (bus < PICO_HUB_I2C_BUS_COUNT)
		if (auto i2c = &pico_hub.i2c[bus]; i2c->is_initialized)
		{
			if (i2c_write_blocking(i2c->bus, address ? address : i2c->address, (const uint8_t*)tx, tx_size, true) == PICO_ERROR_GENERIC)
				return false;

			if (i2c_read_blocking(i2c->bus, address ? address : i2c->address, (uint8_t*)rx, rx_size, false) == PICO_ERROR_GENERIC)
				return false;

			return true;
		}

	return false;
}

bool             pico_hub_pwm_get_slice_and_channel(uint8_t pin, uint8_t* slice, uint8_t* channel)
{
	if (pin >= PICO_HUB_GPIO_COUNT)
		return false;

	*slice   = pwm_gpio_to_slice_num(pin);
	*channel = pwm_gpio_to_channel(pin);

	return true;
}
bool             pico_hub_pwm_init(uint8_t slice, uint16_t wrap, uint16_t level, float clkdiv)
{
	assert(pico_hub.is_created);

	if (slice >= PICO_HUB_PWM_SLICES_COUNT)
		return false;

	auto pwm = &pico_hub.pwm[slice];

	if (pwm->is_initialized)
		return false;

	uint8_t ch_a = slice >> 1;
	uint8_t ch_b = ch_a + 1;

	if (pico_hub_gpio_is_in_use(ch_a) || pico_hub_gpio_is_in_use(ch_b))
		return false;

	gpio_set_function(ch_a, GPIO_FUNC_PWM);
	gpio_set_function(ch_b, GPIO_FUNC_PWM);

	pwm_set_wrap(slice, wrap);
	pwm_set_clkdiv(slice, clkdiv);
	pwm_set_chan_level(slice, PWM_CHAN_A, level);
	pwm_set_chan_level(slice, PWM_CHAN_B, level);
	pwm_set_enabled(slice, false);

	pwm->wrap                             = wrap;
	pwm->clkdiv                           = clkdiv;
	pwm->channels[PICO_HUB_PWM_CHANNEL_A] = { .pin = ch_a, .level = level };
	pwm->channels[PICO_HUB_PWM_CHANNEL_B] = { .pin = ch_b, .level = level };
	pwm->is_enabled                       = false;
	pwm->is_initialized                   = true;

	pico_hub_gpio_set_function(ch_a, PICO_HUB_GPIO_FUNCTION_PWM);
	pico_hub_gpio_set_function(ch_b, PICO_HUB_GPIO_FUNCTION_PWM);

	return true;
}
void             pico_hub_pwm_deinit(uint8_t slice)
{
	assert(pico_hub.is_created);

	if (slice < PICO_HUB_PWM_SLICES_COUNT)
		if (auto pwm = &pico_hub.pwm[slice]; pwm->is_initialized)
		{
			pwm_set_enabled(slice, false);

			gpio_deinit(pwm->channels[PICO_HUB_PWM_CHANNEL_A].pin);
			gpio_deinit(pwm->channels[PICO_HUB_PWM_CHANNEL_B].pin);

			pico_hub_gpio_set_function(pwm->channels[PICO_HUB_PWM_CHANNEL_A].pin, PICO_HUB_GPIO_FUNCTION_NONE);
			pico_hub_gpio_set_function(pwm->channels[PICO_HUB_PWM_CHANNEL_B].pin, PICO_HUB_GPIO_FUNCTION_NONE);

			pwm->is_initialized = false;
		}
}
bool             pico_hub_pwm_get_wrap(uint8_t slice, uint16_t* value)
{
	assert(pico_hub.is_created);

	if (slice < PICO_HUB_PWM_SLICES_COUNT)
		if (auto pwm = &pico_hub.pwm[slice]; pwm->is_initialized)
		{
			*value = pwm->wrap;

			return true;
		}

	return false;
}
bool             pico_hub_pwm_get_level(uint8_t slice, uint8_t channel, uint16_t* value)
{
	assert(pico_hub.is_created);

	if ((slice < PICO_HUB_PWM_SLICES_COUNT) && (channel < PICO_HUB_PWM_CHANNELS_COUNT))
		if (auto pwm = &pico_hub.pwm[slice]; pwm->is_initialized)
		{
			*value = pwm->channels[channel].level;

			return true;
		}

	return false;
}
bool             pico_hub_pwm_get_clkdiv(uint8_t slice, float* value)
{
	assert(pico_hub.is_created);

	if (slice < PICO_HUB_PWM_SLICES_COUNT)
		if (auto pwm = &pico_hub.pwm[slice]; pwm->is_initialized)
		{
			*value = pwm->clkdiv;

			return true;
		}

	return false;
}
bool             pico_hub_pwm_get_enabled(uint8_t slice, bool* value)
{
	assert(pico_hub.is_created);

	if (slice < PICO_HUB_PWM_SLICES_COUNT)
		if (auto pwm = &pico_hub.pwm[slice]; pwm->is_initialized)
		{
			*value = pwm->is_enabled;

			return true;
		}

	return false;
}
bool             pico_hub_pwm_set_wrap(uint8_t slice, uint16_t value)
{
	assert(pico_hub.is_created);

	if (slice < PICO_HUB_PWM_SLICES_COUNT)
		if (auto pwm = &pico_hub.pwm[slice]; pwm->is_initialized)
		{
			pwm_set_wrap(slice, value);

			pwm->wrap = value;

			return true;
		}

	return false;
}
bool             pico_hub_pwm_set_level(uint8_t slice, uint8_t channel, uint16_t value)
{
	assert(pico_hub.is_created);

	if ((slice < PICO_HUB_PWM_SLICES_COUNT) && (channel < PICO_HUB_PWM_CHANNELS_COUNT))
		if (auto pwm = &pico_hub.pwm[slice]; pwm->is_initialized)
		{
			pwm_set_chan_level(slice, channel, value);

			pwm->channels[channel].level = value;

			return true;
		}

	return false;
}
bool             pico_hub_pwm_set_clkdiv(uint8_t slice, float value)
{
	assert(pico_hub.is_created);

	if (slice < PICO_HUB_PWM_SLICES_COUNT)
		if (auto pwm = &pico_hub.pwm[slice]; pwm->is_initialized)
		{
			pwm_set_clkdiv(slice, value);

			pwm->clkdiv = value;

			return true;
		}

	return false;
}
bool             pico_hub_pwm_set_enabled(uint8_t slice, bool set)
{
	assert(pico_hub.is_created);

	if (slice < PICO_HUB_PWM_SLICES_COUNT)
		if (auto pwm = &pico_hub.pwm[slice]; pwm->is_initialized)
		{
			pwm_set_enabled(slice, set);

			pwm->is_enabled = set;

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

	if (pico_hub_gpio_is_in_use(miso) || pico_hub_gpio_is_in_use(mosi) || pico_hub_gpio_is_in_use(clock) || pico_hub_gpio_is_in_use(cs))
		return false;

	baud = spi_init(spi->bus, baud);
	gpio_set_function(slave ? mosi : miso, GPIO_FUNC_SPI);
	gpio_set_function(clock, GPIO_FUNC_SPI);
	gpio_set_function(slave ? miso : mosi, GPIO_FUNC_SPI);
	gpio_set_function(cs, GPIO_FUNC_SPI);

	spi->baud           = baud;
	spi->pin_cs         = cs;
	spi->pin_miso       = miso;
	spi->pin_mosi       = mosi;
	spi->pin_sclk       = clock;
	spi->is_slave       = slave;
	spi->is_initialized = true;

	pico_hub_gpio_set_function(cs, PICO_HUB_GPIO_FUNCTION_SPI);
	pico_hub_gpio_set_function(miso, PICO_HUB_GPIO_FUNCTION_SPI);
	pico_hub_gpio_set_function(mosi, PICO_HUB_GPIO_FUNCTION_SPI);
	pico_hub_gpio_set_function(clock, PICO_HUB_GPIO_FUNCTION_SPI);

	return true;
}
void             pico_hub_spi_deinit(PICO_HUB_SPI bus)
{
	assert(pico_hub.is_created);

	if (bus < PICO_HUB_SPI_BUS_COUNT)
		if (auto spi = &pico_hub.spi[bus]; spi->is_initialized)
		{
			spi_deinit(spi->bus);

			pico_hub_gpio_set_function(spi->pin_cs, PICO_HUB_GPIO_FUNCTION_NONE);
			pico_hub_gpio_set_function(spi->pin_miso, PICO_HUB_GPIO_FUNCTION_NONE);
			pico_hub_gpio_set_function(spi->pin_mosi, PICO_HUB_GPIO_FUNCTION_NONE);
			pico_hub_gpio_set_function(spi->pin_sclk, PICO_HUB_GPIO_FUNCTION_NONE);

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

	if (pico_hub_gpio_is_in_use(pin))
		return false;

	auto gpio = &pico_hub.gpio[pin];

	gpio_init(pin);
	gpio_set_dir(pin, direction);
	gpio_put(pin, value);

	gpio->is_low   = !value;
	gpio->is_input = !direction;

	pico_hub_gpio_set_function(pin, PICO_HUB_GPIO_FUNCTION_GPIO);

	return true;
}
void             pico_hub_gpio_deinit(uint8_t pin)
{
	assert(pico_hub.is_created);

	if (pin < PICO_HUB_GPIO_COUNT)
		if (auto gpio = &pico_hub.gpio[pin]; gpio->is_initialized && (gpio->function == PICO_HUB_GPIO_FUNCTION_GPIO))
		{
			gpio_deinit(pin);

			pico_hub_gpio_set_function(pin, PICO_HUB_GPIO_FUNCTION_NONE);
		}
}
bool             pico_hub_gpio_is_in_use(uint8_t pin)
{
	if (pin >= PICO_HUB_GPIO_COUNT)
		return false;

	return pico_hub.gpio[pin].is_initialized;
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
int              pico_hub_gpio_get_function(uint8_t pin)
{
	assert(pico_hub.is_created);

	if (pin < PICO_HUB_GPIO_COUNT)
		if (auto gpio = &pico_hub.gpio[pin]; gpio->is_initialized)
			return gpio->function;

	return PICO_HUB_GPIO_FUNCTION_NONE;
}
bool             pico_hub_gpio_set_function(uint8_t pin, int value)
{
	assert(pico_hub.is_created);

	if (pin >= PICO_HUB_GPIO_COUNT)
		return false;

	auto gpio = &pico_hub.gpio[pin];

	if (gpio->is_initialized)
		return false;

	gpio->pin            = pin;
	gpio->function       = value;
	gpio->is_initialized = value != PICO_HUB_GPIO_FUNCTION_NONE;

	return true;
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

	if (pico_hub_gpio_is_in_use(rx) || pico_hub_gpio_is_in_use(tx))
		return false;

	baud = uart_init(uart->bus, baud);
	gpio_set_function(tx, UART_FUNCSEL_NUM(uart->bus, tx));
	gpio_set_function(rx, UART_FUNCSEL_NUM(uart->bus, rx));

	uart->baud           = baud;
	uart->pin_rx         = rx;
	uart->pin_tx         = tx;
	uart->is_initialized = true;

	pico_hub_gpio_set_function(rx, PICO_HUB_GPIO_FUNCTION_UART);
	pico_hub_gpio_set_function(tx, PICO_HUB_GPIO_FUNCTION_UART);

	return true;
}
void             pico_hub_uart_deinit(PICO_HUB_UART bus)
{
	assert(pico_hub.is_created);

	if (bus < PICO_HUB_UART_BUS_COUNT)
		if (auto uart = &pico_hub.uart[bus]; uart->is_initialized)
		{
			uart_deinit(uart->bus);

			pico_hub_gpio_set_function(uart->pin_rx, PICO_HUB_GPIO_FUNCTION_NONE);
			pico_hub_gpio_set_function(uart->pin_tx, PICO_HUB_GPIO_FUNCTION_NONE);

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

bool             pico_hub_wifi_scan(pico_hub_wifi_scan_callback callback, void* param)
{
#if defined(LIB_PICO_CYW43_ARCH) && CYW43_LWIP
	struct wifi_scan_context
	{
		bool                        error;

		pico_hub_wifi_network       network;
		std::string                 network_ssid;

		pico_hub_wifi_scan_callback callback;
		void*                       callback_param;
	};

	cyw43_wifi_scan_options_t options = {};
	wifi_scan_context         context = { .error = false, .callback = callback, .callback_param = param };

	auto on_result = [](void* param, const cyw43_ev_scan_result_t* result)->int
	{
		auto context = (wifi_scan_context*)param;

		context->network_ssid.assign((const char*)result->ssid, result->ssid_len);

		context->network.auth    = pico_hub_wifi_auth_from_cyw43(result->auth_mode);
		context->network.ssid    = context->network_ssid.c_str();
		context->network.channel = (uint8_t)result->channel;
		memcpy(context->network.bssid, result->bssid, 6);

		if (!context->error && !context->callback(&context->network, context->callback_param))
			context->error = true;

		return 0;
	};

	cyw43_arch_lwip_begin();

	if (auto error = cyw43_wifi_scan(&cyw43_state, &options, &context, on_result))
	{
		cyw43_arch_lwip_end();

		return false;
	}

	cyw43_arch_lwip_end();

	return true;
#else
	return false;
#endif
}
bool             pico_hub_wifi_ap_open(const char* ssid, const char* passwd, PICO_HUB_WIFI_AUTH auth, uint8_t channel)
{
#if defined(LIB_PICO_CYW43_ARCH) && CYW43_LWIP
	if (pico_hub.wifi.ap.is_open)
		pico_hub_wifi_ap_close();

	cyw43_arch_lwip_begin();

	if (channel)
		cyw43_wifi_ap_set_channel(&cyw43_state, channel);
	else
		channel = cyw43_state.ap_channel;

	cyw43_arch_lwip_end();

	cyw43_arch_enable_ap_mode(ssid, passwd, pico_hub_wifi_auth_to_cyw43(auth));

	pico_hub.wifi.ap =
	{
		.is_open = true,

		.auth    = auth,
		.ssid    = ssid,
		.passwd  = passwd,
		.channel = channel
	};

	return true;
#else
	return false;
#endif
}
void             pico_hub_wifi_ap_close()
{
#if defined(LIB_PICO_CYW43_ARCH) && CYW43_LWIP
	if (pico_hub.wifi.ap.is_open)
	{
		cyw43_arch_disable_ap_mode();

		pico_hub.wifi.ap.is_open = false;
	}
#endif
}
bool             pico_hub_wifi_station_connect(const char* ssid, const char* passwd, PICO_HUB_WIFI_AUTH auth)
{
#if defined(LIB_PICO_CYW43_ARCH) && CYW43_LWIP
	if (pico_hub.wifi.station.is_open)
		pico_hub_wifi_station_disconnect();

	cyw43_arch_enable_sta_mode();

	if (auto error = cyw43_arch_wifi_connect_blocking(ssid, passwd, pico_hub_wifi_auth_to_cyw43(auth)))
	{
		cyw43_arch_disable_sta_mode();

		return false;
	}

	pico_hub.wifi.station =
	{
		.is_open      = true,
		.is_connected = true,

		.auth         = auth,
		.ssid         = ssid,
		.passwd       = passwd
	};

	return true;
#else
	return false;
#endif
}
void             pico_hub_wifi_station_disconnect()
{
#if defined(LIB_PICO_CYW43_ARCH) && CYW43_LWIP
	if (pico_hub.wifi.station.is_open)
	{
		cyw43_arch_disable_sta_mode();

		pico_hub.wifi.station.is_open      = false;
		pico_hub.wifi.station.is_connected = false;
	}
#endif
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
bool             pico_hub_packet_handler_get_pinout(const void* buffer, size_t size)
{
	if (auto request = pico_hub_io_get_request<PICO_HUB_OPCODE_GET_PINOUT>(buffer, size))
	{
		pico_hub_packet_response<PICO_HUB_OPCODE_GET_PINOUT> response;
		pico_hub_get_pinout(&response.value);

		return pico_hub_io_send_response(response);
	}

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
		response.success = pico_hub_i2c_init(request->bus, request->scl, request->sda, request->baud, request->address, request->slave);

		return pico_hub_io_send_response(response);
	}

	return false;
}
bool             pico_hub_packet_handler_i2c_deinit(const void* buffer, size_t size)
{
	if (auto request = pico_hub_io_get_request<PICO_HUB_OPCODE_I2C_DEINIT>(buffer, size))
	{
		pico_hub_i2c_deinit(request->bus);

		return pico_hub_io_send_response<PICO_HUB_OPCODE_I2C_DEINIT>({});
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
bool             pico_hub_packet_handler_pwm_get_slice_and_channel(const void* buffer, size_t size)
{
	if (auto request = pico_hub_io_get_request<PICO_HUB_OPCODE_PWM_GET_SLICE_AND_CHANNEL>(buffer, size))
	{
		pico_hub_packet_response<PICO_HUB_OPCODE_PWM_GET_SLICE_AND_CHANNEL> response;
		response.success = pico_hub_pwm_get_slice_and_channel(request->pin, &response.slice, &response.channel);

		return pico_hub_io_send_response(response);
	}

	return false;
}
bool             pico_hub_packet_handler_pwm_init(const void* buffer, size_t size)
{
	if (auto request = pico_hub_io_get_request<PICO_HUB_OPCODE_PWM_INIT>(buffer, size))
		return pico_hub_io_send_response<PICO_HUB_OPCODE_PWM_INIT>({
			.success = pico_hub_pwm_init(request->slice, request->wrap, request->level, request->clkdiv)
		});

	return false;
}
bool             pico_hub_packet_handler_pwm_deinit(const void* buffer, size_t size)
{
	if (auto request = pico_hub_io_get_request<PICO_HUB_OPCODE_PWM_DEINIT>(buffer, size))
	{
		pico_hub_pwm_deinit(request->slice);

		return pico_hub_io_send_response<PICO_HUB_OPCODE_PWM_DEINIT>({});
	}

	return false;
}
bool             pico_hub_packet_handler_pwm_get_wrap(const void* buffer, size_t size)
{
	if (auto request = pico_hub_io_get_request<PICO_HUB_OPCODE_PWM_GET_WRAP>(buffer, size))
	{
		pico_hub_packet_response<PICO_HUB_OPCODE_PWM_GET_WRAP> response;
		response.success = pico_hub_pwm_get_wrap(request->slice, &response.value);

		return pico_hub_io_send_response(response);
	}

	return false;
}
bool             pico_hub_packet_handler_pwm_get_level(const void* buffer, size_t size)
{
	if (auto request = pico_hub_io_get_request<PICO_HUB_OPCODE_PWM_GET_LEVEL>(buffer, size))
	{
		pico_hub_packet_response<PICO_HUB_OPCODE_PWM_GET_LEVEL> response;
		response.success = pico_hub_pwm_get_level(request->slice, request->channel, &response.value);

		return pico_hub_io_send_response(response);
	}

	return false;
}
bool             pico_hub_packet_handler_pwm_get_clkdiv(const void* buffer, size_t size)
{
	if (auto request = pico_hub_io_get_request<PICO_HUB_OPCODE_PWM_GET_CLKDIV>(buffer, size))
	{
		pico_hub_packet_response<PICO_HUB_OPCODE_PWM_GET_CLKDIV> response;
		response.success = pico_hub_pwm_get_clkdiv(request->slice, &response.value);

		return pico_hub_io_send_response(response);
	}

	return false;
}
bool             pico_hub_packet_handler_pwm_get_enabled(const void* buffer, size_t size)
{
	if (auto request = pico_hub_io_get_request<PICO_HUB_OPCODE_PWM_GET_ENABLED>(buffer, size))
	{
		pico_hub_packet_response<PICO_HUB_OPCODE_PWM_GET_ENABLED> response;
		response.success = pico_hub_pwm_get_enabled(request->slice, &response.value);

		return pico_hub_io_send_response(response);
	}

	return false;
}
bool             pico_hub_packet_handler_pwm_set_wrap(const void* buffer, size_t size)
{
	if (auto request = pico_hub_io_get_request<PICO_HUB_OPCODE_PWM_SET_WRAP>(buffer, size))
		return pico_hub_io_send_response<PICO_HUB_OPCODE_PWM_SET_WRAP>({
			.success = pico_hub_pwm_set_wrap(request->slice, request->value)
		});

	return false;
}
bool             pico_hub_packet_handler_pwm_set_level(const void* buffer, size_t size)
{
	if (auto request = pico_hub_io_get_request<PICO_HUB_OPCODE_PWM_SET_LEVEL>(buffer, size))
		return pico_hub_io_send_response<PICO_HUB_OPCODE_PWM_SET_LEVEL>({
			.success = pico_hub_pwm_set_level(request->slice, request->channel, request->value)
		});

	return false;
}
bool             pico_hub_packet_handler_pwm_set_clkdiv(const void* buffer, size_t size)
{
	if (auto request = pico_hub_io_get_request<PICO_HUB_OPCODE_PWM_SET_CLKDIV>(buffer, size))
		return pico_hub_io_send_response<PICO_HUB_OPCODE_PWM_SET_CLKDIV>({
			.success = pico_hub_pwm_set_clkdiv(request->slice, request->value)
		});

	return false;
}
bool             pico_hub_packet_handler_pwm_set_enabled(const void* buffer, size_t size)
{
	if (auto request = pico_hub_io_get_request<PICO_HUB_OPCODE_PWM_SET_ENABLED>(buffer, size))
		return pico_hub_io_send_response<PICO_HUB_OPCODE_PWM_SET_ENABLED>({
			.success = pico_hub_pwm_set_enabled(request->slice, request->value)
		});

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

		return pico_hub_io_send_response<PICO_HUB_OPCODE_SPI_DEINIT>({});
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

		return pico_hub_io_send_response<PICO_HUB_OPCODE_GPIO_DEINIT>({});
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

		return pico_hub_io_send_response<PICO_HUB_OPCODE_UART_DEINIT>({});
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
bool             pico_hub_packet_handler_wifi_scan(const void* buffer, size_t size)
{
	if (auto request = pico_hub_io_get_request<PICO_HUB_OPCODE_WIFI_SCAN>(buffer, size))
	{
		pico_hub_packet_response<PICO_HUB_OPCODE_WIFI_SCAN> response = { .success = true, .end = false };
		pico_hub_wifi_scan_callback                         callback([](const pico_hub_wifi_network* network, void* param) {
			auto response = (pico_hub_packet_response<PICO_HUB_OPCODE_WIFI_SCAN>*)param;

			response->auth    = network->auth;
			response->channel = network->channel;

			if (auto length = strlen(network->ssid))
			{
				if (length >= sizeof(response->ssid))
					length = sizeof(response->ssid) - 1;

				memcpy(response->ssid, network->ssid, length);
			}

			memcpy(response->bssid, network->bssid, 6);

			return pico_hub_io_send_response(*response);
		});

		response.end     = true;
		response.success = pico_hub_wifi_scan(callback, &response);

		return pico_hub_io_send_response(response);
	}

	return false;
}
bool             pico_hub_packet_handler_wifi_ap_open(const void* buffer, size_t size)
{
	if (auto request = pico_hub_io_get_request<PICO_HUB_OPCODE_WIFI_AP_OPEN>(buffer, size))
		return pico_hub_io_send_response<PICO_HUB_OPCODE_WIFI_AP_OPEN>({
			.success = pico_hub_wifi_ap_open(request->ssid, request->passwd, request->auth, request->channel)
		});

	return false;
}
bool             pico_hub_packet_handler_wifi_ap_close(const void* buffer, size_t size)
{
	if (auto request = pico_hub_io_get_request<PICO_HUB_OPCODE_WIFI_AP_CLOSE>(buffer, size))
	{
		pico_hub_wifi_ap_close();

		return pico_hub_io_send_response<PICO_HUB_OPCODE_WIFI_AP_CLOSE>({});
	}

	return false;
}
bool             pico_hub_packet_handler_wifi_station_connect(const void* buffer, size_t size)
{
	if (auto request = pico_hub_io_get_request<PICO_HUB_OPCODE_WIFI_STATION_CONNECT>(buffer, size))
		return pico_hub_io_send_response<PICO_HUB_OPCODE_WIFI_STATION_CONNECT>({
			.success = pico_hub_wifi_station_connect(request->ssid, request->passwd, request->auth)
		});

	return false;
}
bool             pico_hub_packet_handler_wifi_station_disconnect(const void* buffer, size_t size)
{
	if (auto request = pico_hub_io_get_request<PICO_HUB_OPCODE_WIFI_STATION_DISCONNECT>(buffer, size))
	{
		pico_hub_wifi_station_disconnect();

		return pico_hub_io_send_response<PICO_HUB_OPCODE_WIFI_STATION_DISCONNECT>({});
	}

	return false;
}
