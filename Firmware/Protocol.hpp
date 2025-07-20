#pragma once
#include <cstddef>
#include <cstdint>
#include <type_traits>

enum PICO_HUB_ADC : uint8_t
{
	PICO_HUB_ADC_GPIO_26            = 0x1, // ADC0/GP26
	PICO_HUB_ADC_GPIO_27            = 0x2, // ADC1/GP27
	PICO_HUB_ADC_GPIO_28            = 0x4, // ADC2/GP28
	PICO_HUB_ADC_SYSTEM_VOLTAGE     = 0x8, // ADC3/GP29/VSYS
	PICO_HUB_ADC_SYSTEM_TEMPERATURE = 0x10 // ADC4/TSYS
};

enum PICO_HUB_I2C : uint8_t
{
	PICO_HUB_I2C_BUS_0, // i2c0
	PICO_HUB_I2C_BUS_1, // i2c1
	PICO_HUB_I2C_BUS_COUNT
};

enum PICO_HUB_PWM : uint8_t
{
	PICO_HUB_PWM_SLICE_1,
	PICO_HUB_PWM_SLICE_2,
	PICO_HUB_PWM_SLICE_3,
	PICO_HUB_PWM_SLICE_4,
	PICO_HUB_PWM_SLICE_5,
	PICO_HUB_PWM_SLICE_6,
	PICO_HUB_PWM_SLICE_7,
	PICO_HUB_PWM_SLICE_8,
	PICO_HUB_PWM_SLICES_COUNT,

	PICO_HUB_PWM_CHANNEL_A = 0,
	PICO_HUB_PWM_CHANNEL_B = 1,
	PICO_HUB_PWM_CHANNELS_COUNT
};

enum PICO_HUB_SPI : uint8_t
{
	PICO_HUB_SPI_BUS_0, // spi0
	PICO_HUB_SPI_BUS_1, // spi1
	PICO_HUB_SPI_BUS_COUNT
};

enum PICO_HUB_GPIO : uint8_t
{
	PICO_HUB_GPIO_0,
	PICO_HUB_GPIO_1,
	PICO_HUB_GPIO_2,
	PICO_HUB_GPIO_3,
	PICO_HUB_GPIO_4,
	PICO_HUB_GPIO_5,
	PICO_HUB_GPIO_6,
	PICO_HUB_GPIO_7,
	PICO_HUB_GPIO_8,
	PICO_HUB_GPIO_9,
	PICO_HUB_GPIO_10,
	PICO_HUB_GPIO_11,
	PICO_HUB_GPIO_12,
	PICO_HUB_GPIO_13,
	PICO_HUB_GPIO_14,
	PICO_HUB_GPIO_15,
	PICO_HUB_GPIO_16,
	PICO_HUB_GPIO_17,
	PICO_HUB_GPIO_18,
	PICO_HUB_GPIO_19,
	PICO_HUB_GPIO_20,
	PICO_HUB_GPIO_21,
	PICO_HUB_GPIO_22,
	PICO_HUB_GPIO_23,
	PICO_HUB_GPIO_24,
	PICO_HUB_GPIO_25,
	PICO_HUB_GPIO_26,
	PICO_HUB_GPIO_27,
	PICO_HUB_GPIO_28,
	PICO_HUB_GPIO_29,
	PICO_HUB_GPIO_COUNT,

	PICO_HUB_GPIO_PULL_UP   = 0x1,
	PICO_HUB_GPIO_PULL_DOWN = 0x2,

	PICO_HUB_GPIO_SLEW_SLOW = 0,
	PICO_HUB_GPIO_SLEW_FAST = 1,

	PICO_HUB_GPIO_VALUE_LOW  = 0,
	PICO_HUB_GPIO_VALUE_HIGH = 1,

	PICO_HUB_GPIO_FUNCTION_NONE = 0,
	PICO_HUB_GPIO_FUNCTION_ADC,
	PICO_HUB_GPIO_FUNCTION_I2C,
	PICO_HUB_GPIO_FUNCTION_PWM,
	PICO_HUB_GPIO_FUNCTION_SPI,
	PICO_HUB_GPIO_FUNCTION_GPIO,
	PICO_HUB_GPIO_FUNCTION_UART,

	PICO_HUB_GPIO_DIRECTION_IN  = 0,
	PICO_HUB_GPIO_DIRECTION_OUT = 1,

	PICO_HUB_GPIO_DRIVE_STRENGTH_2MA  = 0,
	PICO_HUB_GPIO_DRIVE_STRENGTH_4MA  = 1,
	PICO_HUB_GPIO_DRIVE_STRENGTH_8MA  = 2,
	PICO_HUB_GPIO_DRIVE_STRENGTH_12MA = 3
};

enum PICO_HUB_GPIO_FLAGS : uint32_t
{
	PICO_HUB_GPIO_FLAG_PULL_UP             = 0x1,
	PICO_HUB_GPIO_FLAG_PULL_DOWN           = 0x2,

	PICO_HUB_GPIO_FLAG_SLEW_SLOW           = 0x4,
	PICO_HUB_GPIO_FLAG_SLEW_FAST           = 0x8,

	PICO_HUB_GPIO_FLAG_VALUE_LOW           = 0x10,
	PICO_HUB_GPIO_FLAG_VALUE_HIGH          = 0x20,

	PICO_HUB_GPIO_FLAG_FUNCTION_NONE       = 0x40,
	PICO_HUB_GPIO_FLAG_FUNCTION_ADC        = 0x80,
	PICO_HUB_GPIO_FLAG_FUNCTION_I2C        = 0x100,
	PICO_HUB_GPIO_FLAG_FUNCTION_PWM        = 0x200,
	PICO_HUB_GPIO_FLAG_FUNCTION_SPI        = 0x400,
	PICO_HUB_GPIO_FLAG_FUNCTION_GPIO       = 0x800,
	PICO_HUB_GPIO_FLAG_FUNCTION_UART       = 0x1000,

	PICO_HUB_GPIO_FLAG_DIRECTION_IN        = 0x2000,
	PICO_HUB_GPIO_FLAG_DIRECTION_OUT       = 0x4000,

	PICO_HUB_GPIO_FLAG_DRIVE_STRENGTH_2MA  = 0x8000,
	PICO_HUB_GPIO_FLAG_DRIVE_STRENGTH_4MA  = 0x10000,
	PICO_HUB_GPIO_FLAG_DRIVE_STRENGTH_8MA  = 0x20000,
	PICO_HUB_GPIO_FLAG_DRIVE_STRENGTH_12MA = 0x40000
};

enum PICO_HUB_UART : uint8_t
{
	PICO_HUB_UART_BUS_0, // uart0
	PICO_HUB_UART_BUS_1, // uart1
	PICO_HUB_UART_BUS_COUNT
};

enum PICO_HUB_WIFI_AUTH : uint8_t
{
	PICO_HUB_WIFI_AUTH_OPEN,
	PICO_HUB_WIFI_AUTH_WPA_TKIP_PSK,
	PICO_HUB_WIFI_AUTH_WPA2_AES_PSK,
	PICO_HUB_WIFI_AUTH_WPA2_MIXED_PSK,
	PICO_HUB_WIFI_AUTH_WPA3_SAE_AES_PSK,
	PICO_HUB_WIFI_AUTH_WPA3_WPA2_AES_PSK,
};

enum PICO_HUB_VOLTAGE : uint8_t
{
	PICO_HUB_VOLTAGE_0_85 = 0b00110,
	PICO_HUB_VOLTAGE_0_90 = 0b00111,
	PICO_HUB_VOLTAGE_0_95 = 0b01000,
	PICO_HUB_VOLTAGE_1_00 = 0b01001,
	PICO_HUB_VOLTAGE_1_05 = 0b01010,
	PICO_HUB_VOLTAGE_1_10 = 0b01011,
	PICO_HUB_VOLTAGE_1_15 = 0b01100,
	PICO_HUB_VOLTAGE_1_20 = 0b01101,
	PICO_HUB_VOLTAGE_1_25 = 0b01110,
	PICO_HUB_VOLTAGE_1_30 = 0b01111,

	PICO_HUB_VOLTAGE_MIN     = PICO_HUB_VOLTAGE_0_85,
	PICO_HUB_VOLTAGE_MAX     = PICO_HUB_VOLTAGE_1_30,
	PICO_HUB_VOLTAGE_DEFAULT = PICO_HUB_VOLTAGE_1_10
};

enum PICO_HUB_OPCODES : uint8_t
{
	PICO_HUB_OPCODE_GET_ID,
	PICO_HUB_OPCODE_GET_LED,
	PICO_HUB_OPCODE_SET_LED,
	PICO_HUB_OPCODE_GET_CLOCK,
	PICO_HUB_OPCODE_SET_CLOCK,
	PICO_HUB_OPCODE_GET_PINOUT,
	PICO_HUB_OPCODE_GET_LATENCY,
	PICO_HUB_OPCODE_GET_VOLTAGE,
	PICO_HUB_OPCODE_SET_VOLTAGE,
	PICO_HUB_OPCODE_RESTART,
	PICO_HUB_OPCODE_SHUTDOWN,

	PICO_HUB_OPCODE_ADC_INIT,
	PICO_HUB_OPCODE_ADC_DEINIT,
	PICO_HUB_OPCODE_ADC_GET_CHANNEL,
	PICO_HUB_OPCODE_ADC_SET_CHANNEL,
	PICO_HUB_OPCODE_ADC_READ,

	PICO_HUB_OPCODE_I2C_INIT,
	PICO_HUB_OPCODE_I2C_DEINIT,
	PICO_HUB_OPCODE_I2C_READ,
	PICO_HUB_OPCODE_I2C_WRITE,
	PICO_HUB_OPCODE_I2C_WRITE_READ,

	PICO_HUB_OPCODE_PWM_GET_SLICE_AND_CHANNEL,
	PICO_HUB_OPCODE_PWM_INIT,
	PICO_HUB_OPCODE_PWM_DEINIT,
	PICO_HUB_OPCODE_PWM_GET_WRAP,
	PICO_HUB_OPCODE_PWM_GET_LEVEL,
	PICO_HUB_OPCODE_PWM_GET_CLKDIV,
	PICO_HUB_OPCODE_PWM_GET_ENABLED,
	PICO_HUB_OPCODE_PWM_SET_WRAP,
	PICO_HUB_OPCODE_PWM_SET_LEVEL,
	PICO_HUB_OPCODE_PWM_SET_CLKDIV,
	PICO_HUB_OPCODE_PWM_SET_ENABLED,

	PICO_HUB_OPCODE_SPI_INIT,
	PICO_HUB_OPCODE_SPI_DEINIT,
	PICO_HUB_OPCODE_SPI_READ,
	PICO_HUB_OPCODE_SPI_WRITE,
	PICO_HUB_OPCODE_SPI_WRITE_READ,

	PICO_HUB_OPCODE_GPIO_INIT,
	PICO_HUB_OPCODE_GPIO_DEINIT,
	PICO_HUB_OPCODE_GPIO_GET_PULL_UP,
	PICO_HUB_OPCODE_GPIO_SET_PULL_UP,
	PICO_HUB_OPCODE_GPIO_GET_PULL_DOWN,
	PICO_HUB_OPCODE_GPIO_SET_PULL_DOWN,
	PICO_HUB_OPCODE_GPIO_GET_DIRECTION,
	PICO_HUB_OPCODE_GPIO_SET_DIRECTION,
	PICO_HUB_OPCODE_GPIO_GET_DRIVE_STRENGTH,
	PICO_HUB_OPCODE_GPIO_SET_DRIVE_STRENGTH,
	PICO_HUB_OPCODE_GPIO_READ,
	PICO_HUB_OPCODE_GPIO_WRITE,

	PICO_HUB_OPCODE_UART_INIT,
	PICO_HUB_OPCODE_UART_DEINIT,
	PICO_HUB_OPCODE_UART_READ,
	PICO_HUB_OPCODE_UART_WRITE,

	PICO_HUB_OPCODE_WIFI_SCAN,
	PICO_HUB_OPCODE_WIFI_AP_OPEN,
	PICO_HUB_OPCODE_WIFI_AP_CLOSE,
	PICO_HUB_OPCODE_WIFI_STATION_CONNECT,
	PICO_HUB_OPCODE_WIFI_STATION_DISCONNECT,

	PICO_HUB_OPCODES_COUNT
};

enum PICO_HUB_PACKET_TYPES : uint8_t
{
	PICO_HUB_PACKET_TYPE_REQUEST,
	PICO_HUB_PACKET_TYPE_RESPONSE
};

#define PICO_HUB_PACKET(opcode, type, ...) \
	template<> struct pico_hub_packet<opcode, type> __VA_ARGS__; \
	static_assert(std::is_trivial<pico_hub_packet<opcode, type>>::value && std::is_standard_layout<pico_hub_packet<opcode, type>>::value)

struct pico_hub_wifi_network
{
	PICO_HUB_WIFI_AUTH auth;
	const char*        ssid;
	uint8_t            bssid[6];
	uint8_t            channel;
};

#pragma pack(push, 1)
struct pico_hub_pinout_adc
{
	PICO_HUB_ADC channels;
};
struct pico_hub_pinout_i2c
{
	bool     is_open;

	uint32_t baud;
	bool     slave;
	uint8_t  address;
	uint8_t  pin_scl;
	uint8_t  pin_sda;
};
struct pico_hub_pinout_pwm_channel
{
	uint8_t  pin;
	uint16_t level;
};
struct pico_hub_pinout_pwm
{
	bool                        is_open;
	bool                        is_enabled;

	uint16_t                    wrap;
	float                       clkdiv;
	pico_hub_pinout_pwm_channel channels[PICO_HUB_PWM_CHANNELS_COUNT];
};
struct pico_hub_pinout_spi
{
	bool     is_open;

	uint32_t baud;
	bool     slave;
	uint8_t  pin_cs;
	uint8_t  pin_miso;
	uint8_t  pin_mosi;
	uint8_t  pin_clock;
};
struct pico_hub_pinout_gpio
{
	bool                is_open;

	PICO_HUB_GPIO_FLAGS flags;
};
struct pico_hub_pinout_uart
{
	bool     is_open;

	uint32_t baud;
	uint8_t  pin_rx;
	uint8_t  pin_tx;
};
struct pico_hub_pinout
{
	pico_hub_pinout_adc  adc;
	pico_hub_pinout_i2c  i2c[PICO_HUB_I2C_BUS_COUNT];
	pico_hub_pinout_pwm  pwm[PICO_HUB_PWM_SLICES_COUNT];
	pico_hub_pinout_spi  spi[PICO_HUB_SPI_BUS_COUNT];
	pico_hub_pinout_gpio gpio[PICO_HUB_GPIO_COUNT];
	pico_hub_pinout_uart uart[PICO_HUB_UART_BUS_COUNT];
};

template<PICO_HUB_OPCODES OPCODE, PICO_HUB_PACKET_TYPES TYPE>
struct pico_hub_packet;

struct pico_hub_packet_header
{
	PICO_HUB_OPCODES opcode;
	uint16_t         length;
};

template<PICO_HUB_OPCODES OPCODE, PICO_HUB_PACKET_TYPES TYPE, typename = void>
struct pico_hub_packet_is_defined
{
	static constexpr bool value = false;
};
template<PICO_HUB_OPCODES OPCODE, PICO_HUB_PACKET_TYPES TYPE>
struct pico_hub_packet_is_defined<OPCODE, TYPE, std::void_t<decltype(sizeof(pico_hub_packet<OPCODE, TYPE>))>>
{
	static constexpr bool value = true;
};

template<PICO_HUB_OPCODES OPCODE>
using pico_hub_packet_request = pico_hub_packet<OPCODE, PICO_HUB_PACKET_TYPE_REQUEST>;
template<PICO_HUB_OPCODES OPCODE>
using pico_hub_packet_response = pico_hub_packet<OPCODE, PICO_HUB_PACKET_TYPE_RESPONSE>;

template<PICO_HUB_OPCODES OPCODE, bool IS_REQUEST_DEFINED, bool IS_RESPONSE_DEFINED>
struct _pico_hub_packet
{
};
template<PICO_HUB_OPCODES OPCODE>
struct _pico_hub_packet<OPCODE, true, true>
{
	pico_hub_packet_request<OPCODE>  request;
	pico_hub_packet_response<OPCODE> response;
};
template<PICO_HUB_OPCODES OPCODE>
struct _pico_hub_packet<OPCODE, true, false>
{
	pico_hub_packet_request<OPCODE> request;
};
template<PICO_HUB_OPCODES OPCODE>
struct _pico_hub_packet<OPCODE, false, true>
{
	pico_hub_packet_response<OPCODE> response;
};

template<PICO_HUB_OPCODES OPCODE>
using _pico_hub_packets = _pico_hub_packet<OPCODE, pico_hub_packet_is_defined<OPCODE, PICO_HUB_PACKET_TYPE_REQUEST>::value, pico_hub_packet_is_defined<OPCODE, PICO_HUB_PACKET_TYPE_RESPONSE>::value>;

PICO_HUB_PACKET(PICO_HUB_OPCODE_GET_ID,                    PICO_HUB_PACKET_TYPE_REQUEST,  { });
PICO_HUB_PACKET(PICO_HUB_OPCODE_GET_ID,                    PICO_HUB_PACKET_TYPE_RESPONSE, { uint64_t value; });
PICO_HUB_PACKET(PICO_HUB_OPCODE_GET_LED,                   PICO_HUB_PACKET_TYPE_REQUEST,  { });
PICO_HUB_PACKET(PICO_HUB_OPCODE_GET_LED,                   PICO_HUB_PACKET_TYPE_RESPONSE, { bool success; bool value; });
PICO_HUB_PACKET(PICO_HUB_OPCODE_SET_LED,                   PICO_HUB_PACKET_TYPE_REQUEST,  { bool value; });
PICO_HUB_PACKET(PICO_HUB_OPCODE_SET_LED,                   PICO_HUB_PACKET_TYPE_RESPONSE, { bool success; });
PICO_HUB_PACKET(PICO_HUB_OPCODE_GET_CLOCK,                 PICO_HUB_PACKET_TYPE_REQUEST,  { });
PICO_HUB_PACKET(PICO_HUB_OPCODE_GET_CLOCK,                 PICO_HUB_PACKET_TYPE_RESPONSE, { uint32_t value; });
PICO_HUB_PACKET(PICO_HUB_OPCODE_SET_CLOCK,                 PICO_HUB_PACKET_TYPE_REQUEST,  { uint32_t value; });
PICO_HUB_PACKET(PICO_HUB_OPCODE_SET_CLOCK,                 PICO_HUB_PACKET_TYPE_RESPONSE, { bool success; });
PICO_HUB_PACKET(PICO_HUB_OPCODE_GET_PINOUT,                PICO_HUB_PACKET_TYPE_REQUEST,  { });
PICO_HUB_PACKET(PICO_HUB_OPCODE_GET_PINOUT,                PICO_HUB_PACKET_TYPE_RESPONSE, { pico_hub_pinout value; });
PICO_HUB_PACKET(PICO_HUB_OPCODE_GET_LATENCY,               PICO_HUB_PACKET_TYPE_REQUEST,  { });
PICO_HUB_PACKET(PICO_HUB_OPCODE_GET_LATENCY,               PICO_HUB_PACKET_TYPE_RESPONSE, { });
PICO_HUB_PACKET(PICO_HUB_OPCODE_GET_VOLTAGE,               PICO_HUB_PACKET_TYPE_REQUEST,  { });
PICO_HUB_PACKET(PICO_HUB_OPCODE_GET_VOLTAGE,               PICO_HUB_PACKET_TYPE_RESPONSE, { PICO_HUB_VOLTAGE value; });
PICO_HUB_PACKET(PICO_HUB_OPCODE_SET_VOLTAGE,               PICO_HUB_PACKET_TYPE_REQUEST,  { PICO_HUB_VOLTAGE value; });
PICO_HUB_PACKET(PICO_HUB_OPCODE_SET_VOLTAGE,               PICO_HUB_PACKET_TYPE_RESPONSE, { bool success; });
PICO_HUB_PACKET(PICO_HUB_OPCODE_RESTART,                   PICO_HUB_PACKET_TYPE_REQUEST,  { bool restart_to_mass_storage; });
PICO_HUB_PACKET(PICO_HUB_OPCODE_RESTART,                   PICO_HUB_PACKET_TYPE_RESPONSE, { });
PICO_HUB_PACKET(PICO_HUB_OPCODE_SHUTDOWN,                  PICO_HUB_PACKET_TYPE_REQUEST,  { });
PICO_HUB_PACKET(PICO_HUB_OPCODE_SHUTDOWN,                  PICO_HUB_PACKET_TYPE_RESPONSE, { });

PICO_HUB_PACKET(PICO_HUB_OPCODE_ADC_INIT,                  PICO_HUB_PACKET_TYPE_REQUEST,  { PICO_HUB_ADC channels; });
PICO_HUB_PACKET(PICO_HUB_OPCODE_ADC_INIT,                  PICO_HUB_PACKET_TYPE_RESPONSE, { bool success; });
PICO_HUB_PACKET(PICO_HUB_OPCODE_ADC_DEINIT,                PICO_HUB_PACKET_TYPE_REQUEST,  { });
PICO_HUB_PACKET(PICO_HUB_OPCODE_ADC_DEINIT,                PICO_HUB_PACKET_TYPE_RESPONSE, { });
PICO_HUB_PACKET(PICO_HUB_OPCODE_ADC_GET_CHANNEL,           PICO_HUB_PACKET_TYPE_REQUEST,  { });
PICO_HUB_PACKET(PICO_HUB_OPCODE_ADC_GET_CHANNEL,           PICO_HUB_PACKET_TYPE_RESPONSE, { bool success; PICO_HUB_ADC value; });
PICO_HUB_PACKET(PICO_HUB_OPCODE_ADC_SET_CHANNEL,           PICO_HUB_PACKET_TYPE_REQUEST,  { PICO_HUB_ADC value; });
PICO_HUB_PACKET(PICO_HUB_OPCODE_ADC_SET_CHANNEL,           PICO_HUB_PACKET_TYPE_RESPONSE, { bool success; });
PICO_HUB_PACKET(PICO_HUB_OPCODE_ADC_READ,                  PICO_HUB_PACKET_TYPE_REQUEST,  { });
PICO_HUB_PACKET(PICO_HUB_OPCODE_ADC_READ,                  PICO_HUB_PACKET_TYPE_RESPONSE, { bool success; uint16_t value; PICO_HUB_ADC channel; });

PICO_HUB_PACKET(PICO_HUB_OPCODE_I2C_INIT,                  PICO_HUB_PACKET_TYPE_REQUEST,  { PICO_HUB_I2C bus; uint8_t scl; uint8_t sda; uint32_t baud; uint8_t address; bool slave; });
PICO_HUB_PACKET(PICO_HUB_OPCODE_I2C_INIT,                  PICO_HUB_PACKET_TYPE_RESPONSE, { bool success; });
PICO_HUB_PACKET(PICO_HUB_OPCODE_I2C_DEINIT,                PICO_HUB_PACKET_TYPE_REQUEST,  { PICO_HUB_I2C bus; });
PICO_HUB_PACKET(PICO_HUB_OPCODE_I2C_DEINIT,                PICO_HUB_PACKET_TYPE_RESPONSE, { });
PICO_HUB_PACKET(PICO_HUB_OPCODE_I2C_READ,                  PICO_HUB_PACKET_TYPE_REQUEST,  { PICO_HUB_I2C bus; uint8_t address; uint32_t size; bool stop; });
PICO_HUB_PACKET(PICO_HUB_OPCODE_I2C_READ,                  PICO_HUB_PACKET_TYPE_RESPONSE, { bool success; });
PICO_HUB_PACKET(PICO_HUB_OPCODE_I2C_WRITE,                 PICO_HUB_PACKET_TYPE_REQUEST,  { PICO_HUB_I2C bus; uint8_t address; uint32_t size; bool stop; });
PICO_HUB_PACKET(PICO_HUB_OPCODE_I2C_WRITE,                 PICO_HUB_PACKET_TYPE_RESPONSE, { bool success; });
PICO_HUB_PACKET(PICO_HUB_OPCODE_I2C_WRITE_READ,            PICO_HUB_PACKET_TYPE_REQUEST,  { PICO_HUB_I2C bus; uint8_t address; uint32_t size[2]; });
PICO_HUB_PACKET(PICO_HUB_OPCODE_I2C_WRITE_READ,            PICO_HUB_PACKET_TYPE_RESPONSE, { bool success; });

PICO_HUB_PACKET(PICO_HUB_OPCODE_PWM_GET_SLICE_AND_CHANNEL, PICO_HUB_PACKET_TYPE_REQUEST,  { uint8_t pin; });
PICO_HUB_PACKET(PICO_HUB_OPCODE_PWM_GET_SLICE_AND_CHANNEL, PICO_HUB_PACKET_TYPE_RESPONSE, { bool success; uint8_t slice; uint8_t channel; });
PICO_HUB_PACKET(PICO_HUB_OPCODE_PWM_INIT,                  PICO_HUB_PACKET_TYPE_REQUEST,  { uint8_t slice; uint16_t wrap; uint16_t level; float clkdiv; });
PICO_HUB_PACKET(PICO_HUB_OPCODE_PWM_INIT,                  PICO_HUB_PACKET_TYPE_RESPONSE, { bool success; });
PICO_HUB_PACKET(PICO_HUB_OPCODE_PWM_DEINIT,                PICO_HUB_PACKET_TYPE_REQUEST,  { uint8_t slice; });
PICO_HUB_PACKET(PICO_HUB_OPCODE_PWM_DEINIT,                PICO_HUB_PACKET_TYPE_RESPONSE, { });
PICO_HUB_PACKET(PICO_HUB_OPCODE_PWM_GET_WRAP,              PICO_HUB_PACKET_TYPE_REQUEST,  { uint8_t slice; });
PICO_HUB_PACKET(PICO_HUB_OPCODE_PWM_GET_WRAP,              PICO_HUB_PACKET_TYPE_RESPONSE, { bool success; uint16_t value; });
PICO_HUB_PACKET(PICO_HUB_OPCODE_PWM_GET_LEVEL,             PICO_HUB_PACKET_TYPE_REQUEST,  { uint8_t slice; uint8_t channel; });
PICO_HUB_PACKET(PICO_HUB_OPCODE_PWM_GET_LEVEL,             PICO_HUB_PACKET_TYPE_RESPONSE, { bool success; uint16_t value; });
PICO_HUB_PACKET(PICO_HUB_OPCODE_PWM_GET_CLKDIV,            PICO_HUB_PACKET_TYPE_REQUEST,  { uint8_t slice; });
PICO_HUB_PACKET(PICO_HUB_OPCODE_PWM_GET_CLKDIV,            PICO_HUB_PACKET_TYPE_RESPONSE, { bool success; float value; });
PICO_HUB_PACKET(PICO_HUB_OPCODE_PWM_GET_ENABLED,           PICO_HUB_PACKET_TYPE_REQUEST,  { uint8_t slice; });
PICO_HUB_PACKET(PICO_HUB_OPCODE_PWM_GET_ENABLED,           PICO_HUB_PACKET_TYPE_RESPONSE, { bool success; bool value; });
PICO_HUB_PACKET(PICO_HUB_OPCODE_PWM_SET_WRAP,              PICO_HUB_PACKET_TYPE_REQUEST,  { uint8_t slice; uint16_t value; });
PICO_HUB_PACKET(PICO_HUB_OPCODE_PWM_SET_WRAP,              PICO_HUB_PACKET_TYPE_RESPONSE, { bool success; });
PICO_HUB_PACKET(PICO_HUB_OPCODE_PWM_SET_LEVEL,             PICO_HUB_PACKET_TYPE_REQUEST,  { uint8_t slice; uint8_t channel; uint16_t value; });
PICO_HUB_PACKET(PICO_HUB_OPCODE_PWM_SET_LEVEL,             PICO_HUB_PACKET_TYPE_RESPONSE, { bool success; });
PICO_HUB_PACKET(PICO_HUB_OPCODE_PWM_SET_CLKDIV,            PICO_HUB_PACKET_TYPE_REQUEST,  { uint8_t slice; float value; });
PICO_HUB_PACKET(PICO_HUB_OPCODE_PWM_SET_CLKDIV,            PICO_HUB_PACKET_TYPE_RESPONSE, { bool success; });
PICO_HUB_PACKET(PICO_HUB_OPCODE_PWM_SET_ENABLED,           PICO_HUB_PACKET_TYPE_REQUEST,  { uint8_t slice; bool value; });
PICO_HUB_PACKET(PICO_HUB_OPCODE_PWM_SET_ENABLED,           PICO_HUB_PACKET_TYPE_RESPONSE, { bool success; });

PICO_HUB_PACKET(PICO_HUB_OPCODE_SPI_INIT,                  PICO_HUB_PACKET_TYPE_REQUEST,  { PICO_HUB_SPI bus; uint8_t miso; uint8_t mosi; uint8_t clock; uint8_t cs; uint32_t baud; bool slave; });
PICO_HUB_PACKET(PICO_HUB_OPCODE_SPI_INIT,                  PICO_HUB_PACKET_TYPE_RESPONSE, { bool success; });
PICO_HUB_PACKET(PICO_HUB_OPCODE_SPI_DEINIT,                PICO_HUB_PACKET_TYPE_REQUEST,  { PICO_HUB_SPI bus; });
PICO_HUB_PACKET(PICO_HUB_OPCODE_SPI_DEINIT,                PICO_HUB_PACKET_TYPE_RESPONSE, { });
PICO_HUB_PACKET(PICO_HUB_OPCODE_SPI_READ,                  PICO_HUB_PACKET_TYPE_REQUEST,  { PICO_HUB_SPI bus; uint32_t size; });
PICO_HUB_PACKET(PICO_HUB_OPCODE_SPI_READ,                  PICO_HUB_PACKET_TYPE_RESPONSE, { bool success; });
PICO_HUB_PACKET(PICO_HUB_OPCODE_SPI_WRITE,                 PICO_HUB_PACKET_TYPE_REQUEST,  { PICO_HUB_SPI bus; uint32_t size; });
PICO_HUB_PACKET(PICO_HUB_OPCODE_SPI_WRITE,                 PICO_HUB_PACKET_TYPE_RESPONSE, { bool success; });
PICO_HUB_PACKET(PICO_HUB_OPCODE_SPI_WRITE_READ,            PICO_HUB_PACKET_TYPE_REQUEST,  { PICO_HUB_SPI bus; uint32_t size; });
PICO_HUB_PACKET(PICO_HUB_OPCODE_SPI_WRITE_READ,            PICO_HUB_PACKET_TYPE_RESPONSE, { bool success; });

PICO_HUB_PACKET(PICO_HUB_OPCODE_GPIO_INIT,                 PICO_HUB_PACKET_TYPE_REQUEST,  { uint8_t pin; bool direction; bool value; });
PICO_HUB_PACKET(PICO_HUB_OPCODE_GPIO_INIT,                 PICO_HUB_PACKET_TYPE_RESPONSE, { bool success; });
PICO_HUB_PACKET(PICO_HUB_OPCODE_GPIO_DEINIT,               PICO_HUB_PACKET_TYPE_REQUEST,  { uint8_t pin; });
PICO_HUB_PACKET(PICO_HUB_OPCODE_GPIO_DEINIT,               PICO_HUB_PACKET_TYPE_RESPONSE, { });
PICO_HUB_PACKET(PICO_HUB_OPCODE_GPIO_GET_PULL_UP,          PICO_HUB_PACKET_TYPE_REQUEST,  { uint8_t pin; });
PICO_HUB_PACKET(PICO_HUB_OPCODE_GPIO_GET_PULL_UP,          PICO_HUB_PACKET_TYPE_RESPONSE, { bool success; bool value; });
PICO_HUB_PACKET(PICO_HUB_OPCODE_GPIO_SET_PULL_UP,          PICO_HUB_PACKET_TYPE_REQUEST,  { uint8_t pin; bool value; });
PICO_HUB_PACKET(PICO_HUB_OPCODE_GPIO_SET_PULL_UP,          PICO_HUB_PACKET_TYPE_RESPONSE, { bool success; });
PICO_HUB_PACKET(PICO_HUB_OPCODE_GPIO_GET_PULL_DOWN,        PICO_HUB_PACKET_TYPE_REQUEST,  { uint8_t pin; });
PICO_HUB_PACKET(PICO_HUB_OPCODE_GPIO_GET_PULL_DOWN,        PICO_HUB_PACKET_TYPE_RESPONSE, { bool success; bool value; });
PICO_HUB_PACKET(PICO_HUB_OPCODE_GPIO_SET_PULL_DOWN,        PICO_HUB_PACKET_TYPE_REQUEST,  { uint8_t pin; bool value; });
PICO_HUB_PACKET(PICO_HUB_OPCODE_GPIO_SET_PULL_DOWN,        PICO_HUB_PACKET_TYPE_RESPONSE, { bool success; });
PICO_HUB_PACKET(PICO_HUB_OPCODE_GPIO_GET_DIRECTION,        PICO_HUB_PACKET_TYPE_REQUEST,  { uint8_t pin; });
PICO_HUB_PACKET(PICO_HUB_OPCODE_GPIO_GET_DIRECTION,        PICO_HUB_PACKET_TYPE_RESPONSE, { bool success; bool value; });
PICO_HUB_PACKET(PICO_HUB_OPCODE_GPIO_SET_DIRECTION,        PICO_HUB_PACKET_TYPE_REQUEST,  { uint8_t pin; bool value; });
PICO_HUB_PACKET(PICO_HUB_OPCODE_GPIO_SET_DIRECTION,        PICO_HUB_PACKET_TYPE_RESPONSE, { bool success; });
PICO_HUB_PACKET(PICO_HUB_OPCODE_GPIO_GET_DRIVE_STRENGTH,   PICO_HUB_PACKET_TYPE_REQUEST,  { uint8_t pin; });
PICO_HUB_PACKET(PICO_HUB_OPCODE_GPIO_GET_DRIVE_STRENGTH,   PICO_HUB_PACKET_TYPE_RESPONSE, { bool success; uint8_t value; });
PICO_HUB_PACKET(PICO_HUB_OPCODE_GPIO_SET_DRIVE_STRENGTH,   PICO_HUB_PACKET_TYPE_REQUEST,  { uint8_t pin; uint8_t value; });
PICO_HUB_PACKET(PICO_HUB_OPCODE_GPIO_SET_DRIVE_STRENGTH,   PICO_HUB_PACKET_TYPE_RESPONSE, { bool success; });
PICO_HUB_PACKET(PICO_HUB_OPCODE_GPIO_READ,                 PICO_HUB_PACKET_TYPE_REQUEST,  { uint8_t pin; });
PICO_HUB_PACKET(PICO_HUB_OPCODE_GPIO_READ,                 PICO_HUB_PACKET_TYPE_RESPONSE, { bool success; bool value; });
PICO_HUB_PACKET(PICO_HUB_OPCODE_GPIO_WRITE,                PICO_HUB_PACKET_TYPE_REQUEST,  { uint8_t pin; bool value; });
PICO_HUB_PACKET(PICO_HUB_OPCODE_GPIO_WRITE,                PICO_HUB_PACKET_TYPE_RESPONSE, { bool success; });

PICO_HUB_PACKET(PICO_HUB_OPCODE_UART_INIT,                 PICO_HUB_PACKET_TYPE_REQUEST,  { PICO_HUB_UART bus; uint8_t rx; uint8_t tx; uint32_t baud; });
PICO_HUB_PACKET(PICO_HUB_OPCODE_UART_INIT,                 PICO_HUB_PACKET_TYPE_RESPONSE, { bool success; });
PICO_HUB_PACKET(PICO_HUB_OPCODE_UART_DEINIT,               PICO_HUB_PACKET_TYPE_REQUEST,  { PICO_HUB_UART bus; });
PICO_HUB_PACKET(PICO_HUB_OPCODE_UART_DEINIT,               PICO_HUB_PACKET_TYPE_RESPONSE, { });
PICO_HUB_PACKET(PICO_HUB_OPCODE_UART_READ,                 PICO_HUB_PACKET_TYPE_REQUEST,  { PICO_HUB_UART bus; uint32_t size; });
PICO_HUB_PACKET(PICO_HUB_OPCODE_UART_READ,                 PICO_HUB_PACKET_TYPE_RESPONSE, { bool success; });
PICO_HUB_PACKET(PICO_HUB_OPCODE_UART_WRITE,                PICO_HUB_PACKET_TYPE_REQUEST,  { PICO_HUB_UART bus; uint32_t size; });
PICO_HUB_PACKET(PICO_HUB_OPCODE_UART_WRITE,                PICO_HUB_PACKET_TYPE_RESPONSE, { bool success; });

PICO_HUB_PACKET(PICO_HUB_OPCODE_WIFI_SCAN,                 PICO_HUB_PACKET_TYPE_REQUEST,  { });
PICO_HUB_PACKET(PICO_HUB_OPCODE_WIFI_SCAN,                 PICO_HUB_PACKET_TYPE_RESPONSE, { bool success; bool end; char ssid[33]; uint8_t bssid[6]; PICO_HUB_WIFI_AUTH auth; uint8_t channel; });
PICO_HUB_PACKET(PICO_HUB_OPCODE_WIFI_AP_OPEN,              PICO_HUB_PACKET_TYPE_REQUEST,  { char ssid[33]; char passwd[40]; PICO_HUB_WIFI_AUTH auth; uint8_t channel; });
PICO_HUB_PACKET(PICO_HUB_OPCODE_WIFI_AP_OPEN,              PICO_HUB_PACKET_TYPE_RESPONSE, { bool success; });
PICO_HUB_PACKET(PICO_HUB_OPCODE_WIFI_AP_CLOSE,             PICO_HUB_PACKET_TYPE_REQUEST,  { });
PICO_HUB_PACKET(PICO_HUB_OPCODE_WIFI_AP_CLOSE,             PICO_HUB_PACKET_TYPE_RESPONSE, { });
PICO_HUB_PACKET(PICO_HUB_OPCODE_WIFI_STATION_CONNECT,      PICO_HUB_PACKET_TYPE_REQUEST,  { char ssid[33]; char passwd[40]; PICO_HUB_WIFI_AUTH auth; });
PICO_HUB_PACKET(PICO_HUB_OPCODE_WIFI_STATION_CONNECT,      PICO_HUB_PACKET_TYPE_RESPONSE, { bool success; });
PICO_HUB_PACKET(PICO_HUB_OPCODE_WIFI_STATION_DISCONNECT,   PICO_HUB_PACKET_TYPE_REQUEST,  { });
PICO_HUB_PACKET(PICO_HUB_OPCODE_WIFI_STATION_DISCONNECT,   PICO_HUB_PACKET_TYPE_RESPONSE, { });

union pico_hub_packets
{
	_pico_hub_packets<PICO_HUB_OPCODE_GET_ID>                    get_id;

	_pico_hub_packets<PICO_HUB_OPCODE_GET_LED>                   get_led;
	_pico_hub_packets<PICO_HUB_OPCODE_SET_LED>                   set_led;

	_pico_hub_packets<PICO_HUB_OPCODE_GET_CLOCK>                 get_clock;
	_pico_hub_packets<PICO_HUB_OPCODE_SET_CLOCK>                 set_clock;

	_pico_hub_packets<PICO_HUB_OPCODE_GET_PINOUT>                get_pinout;

	_pico_hub_packets<PICO_HUB_OPCODE_GET_LATENCY>               get_latency;

	_pico_hub_packets<PICO_HUB_OPCODE_GET_VOLTAGE>               get_voltage;
	_pico_hub_packets<PICO_HUB_OPCODE_SET_VOLTAGE>               set_voltage;

	_pico_hub_packets<PICO_HUB_OPCODE_RESTART>                   restart;
	_pico_hub_packets<PICO_HUB_OPCODE_SHUTDOWN>                  shutdown;

	_pico_hub_packets<PICO_HUB_OPCODE_ADC_INIT>                  adc_init;
	_pico_hub_packets<PICO_HUB_OPCODE_ADC_DEINIT>                adc_deinit;
	_pico_hub_packets<PICO_HUB_OPCODE_ADC_GET_CHANNEL>           adc_get_channel;
	_pico_hub_packets<PICO_HUB_OPCODE_ADC_SET_CHANNEL>           adc_set_channel;
	_pico_hub_packets<PICO_HUB_OPCODE_ADC_READ>                  adc_read;

	_pico_hub_packets<PICO_HUB_OPCODE_I2C_INIT>                  i2c_init;
	_pico_hub_packets<PICO_HUB_OPCODE_I2C_DEINIT>                i2c_deinit;
	_pico_hub_packets<PICO_HUB_OPCODE_I2C_READ>                  i2c_read;
	_pico_hub_packets<PICO_HUB_OPCODE_I2C_WRITE>                 i2c_write;
	_pico_hub_packets<PICO_HUB_OPCODE_I2C_WRITE_READ>            i2c_write_read;

	_pico_hub_packets<PICO_HUB_OPCODE_PWM_GET_SLICE_AND_CHANNEL> pwm_get_slice_and_channel;
	_pico_hub_packets<PICO_HUB_OPCODE_PWM_INIT>                  pwm_init;
	_pico_hub_packets<PICO_HUB_OPCODE_PWM_DEINIT>                pwm_deinit;
	_pico_hub_packets<PICO_HUB_OPCODE_PWM_GET_WRAP>              pwm_get_wrap;
	_pico_hub_packets<PICO_HUB_OPCODE_PWM_GET_LEVEL>             pwm_get_level;
	_pico_hub_packets<PICO_HUB_OPCODE_PWM_GET_CLKDIV>            pwm_get_clkdiv;
	_pico_hub_packets<PICO_HUB_OPCODE_PWM_GET_ENABLED>           pwm_get_enabled;
	_pico_hub_packets<PICO_HUB_OPCODE_PWM_SET_WRAP>              pwm_set_wrap;
	_pico_hub_packets<PICO_HUB_OPCODE_PWM_SET_LEVEL>             pwm_set_level;
	_pico_hub_packets<PICO_HUB_OPCODE_PWM_SET_CLKDIV>            pwm_set_clkdiv;
	_pico_hub_packets<PICO_HUB_OPCODE_PWM_SET_ENABLED>           pwm_set_enabled;

	_pico_hub_packets<PICO_HUB_OPCODE_SPI_INIT>                  spi_init;
	_pico_hub_packets<PICO_HUB_OPCODE_SPI_DEINIT>                spi_deinit;
	_pico_hub_packets<PICO_HUB_OPCODE_SPI_READ>                  spi_read;
	_pico_hub_packets<PICO_HUB_OPCODE_SPI_WRITE>                 spi_write;
	_pico_hub_packets<PICO_HUB_OPCODE_SPI_WRITE_READ>            spi_write_read;

	_pico_hub_packets<PICO_HUB_OPCODE_GPIO_INIT>                 gpio_init;
	_pico_hub_packets<PICO_HUB_OPCODE_GPIO_DEINIT>               gpio_deinit;
	_pico_hub_packets<PICO_HUB_OPCODE_GPIO_GET_PULL_UP>          gpio_get_pull_up;
	_pico_hub_packets<PICO_HUB_OPCODE_GPIO_SET_PULL_UP>          gpio_set_pull_up;
	_pico_hub_packets<PICO_HUB_OPCODE_GPIO_GET_PULL_DOWN>        gpio_get_pull_down;
	_pico_hub_packets<PICO_HUB_OPCODE_GPIO_SET_PULL_DOWN>        gpio_set_pull_down;
	_pico_hub_packets<PICO_HUB_OPCODE_GPIO_GET_DIRECTION>        gpio_get_direction;
	_pico_hub_packets<PICO_HUB_OPCODE_GPIO_SET_DIRECTION>        gpio_set_direction;
	_pico_hub_packets<PICO_HUB_OPCODE_GPIO_GET_DRIVE_STRENGTH>   gpio_get_drive_strength;
	_pico_hub_packets<PICO_HUB_OPCODE_GPIO_SET_DRIVE_STRENGTH>   gpio_set_drive_strength;
	_pico_hub_packets<PICO_HUB_OPCODE_GPIO_READ>                 gpio_read;
	_pico_hub_packets<PICO_HUB_OPCODE_GPIO_WRITE>                gpio_write;

	_pico_hub_packets<PICO_HUB_OPCODE_UART_INIT>                 uart_init;
	_pico_hub_packets<PICO_HUB_OPCODE_UART_DEINIT>               uart_deinit;
	_pico_hub_packets<PICO_HUB_OPCODE_UART_READ>                 uart_read;
	_pico_hub_packets<PICO_HUB_OPCODE_UART_WRITE>                uart_write;

	_pico_hub_packets<PICO_HUB_OPCODE_WIFI_SCAN>                 wifi_scan;
	_pico_hub_packets<PICO_HUB_OPCODE_WIFI_AP_OPEN>              wifi_ap_open;
	_pico_hub_packets<PICO_HUB_OPCODE_WIFI_AP_CLOSE>             wifi_ap_close;
	_pico_hub_packets<PICO_HUB_OPCODE_WIFI_STATION_CONNECT>      wifi_station_connect;
	_pico_hub_packets<PICO_HUB_OPCODE_WIFI_STATION_DISCONNECT>   wifi_station_disconnect;
};
#pragma pack(pop)
