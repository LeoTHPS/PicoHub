#include <PicoHub.hpp>

#include <iostream>

const char* pico_hub_voltage_to_string(PICO_HUB_VOLTAGE value)
{
	switch (value)
	{
		case PICO_HUB_VOLTAGE_0_85: return "0.85";
		case PICO_HUB_VOLTAGE_0_90: return "0.90";
		case PICO_HUB_VOLTAGE_0_95: return "0.95";
		case PICO_HUB_VOLTAGE_1_00: return "1.00";
		case PICO_HUB_VOLTAGE_1_05: return "1.05";
		case PICO_HUB_VOLTAGE_1_10: return "1.10";
		case PICO_HUB_VOLTAGE_1_15: return "1.15";
		case PICO_HUB_VOLTAGE_1_20: return "1.20";
		case PICO_HUB_VOLTAGE_1_25: return "1.25";
		case PICO_HUB_VOLTAGE_1_30: return "1.30";
	}

	return "";
}

int         pico_hub_get_latency_us(pico_hub* hub, uint32_t* value, uint32_t count)
{
	uint64_t total = 0;

	for (size_t i = 0; i < count; ++i)
	{
		if (auto error = pico_hub_get_latency_us(hub, value))
			return error;

		total += *value;
	}

	*value = total / count;

	return PICO_HUB_ERROR_NONE;
}

int         pico_hub_get_and_display_pinout(pico_hub* hub)
{
	uint32_t        clock;
	pico_hub_pinout pinout;

	if (auto error = pico_hub_get_clock(hub, &clock))
		return error;

	if (auto error = pico_hub_get_pinout(hub, &pinout))
		return error;

	std::cout << "Pinout" << std::endl;

	std::cout << "\tADC" << std::endl;
	std::cout << "\t\tGPIO_26: "            << ((pinout.adc.channels & PICO_HUB_ADC_GPIO_26) ?            "Open" : "") << std::endl;
	std::cout << "\t\tGPIO_27: "            << ((pinout.adc.channels & PICO_HUB_ADC_GPIO_27) ?            "Open" : "") << std::endl;
	std::cout << "\t\tGPIO_28: "            << ((pinout.adc.channels & PICO_HUB_ADC_GPIO_28) ?            "Open" : "") << std::endl;
	std::cout << "\t\tSystem Voltage: "     << ((pinout.adc.channels & PICO_HUB_ADC_SYSTEM_VOLTAGE) ?     "Open" : "") << std::endl;
	std::cout << "\t\tSystem Temperature: " << ((pinout.adc.channels & PICO_HUB_ADC_SYSTEM_TEMPERATURE) ? "Open" : "") << std::endl;

	std::cout << "\tI2C" << std::endl;
	for (int bus = 0; bus < PICO_HUB_I2C_BUS_COUNT; ++bus)
	{
		if (pinout.i2c[bus].is_open)
		{
			std::cout << "\t\tBus: "     << bus << std::endl;
			std::cout << "\t\tBaud: "    << pinout.i2c[bus].baud << std::endl;
			std::cout << "\t\tSlave: "   << (pinout.i2c[bus].slave ? "True" : "False") << std::endl;
			std::cout << "\t\tAddress: " << (int)pinout.i2c[bus].address << std::endl;
			std::cout << "\t\tSCL: "     << (int)pinout.i2c[bus].pin_scl << std::endl;
			std::cout << "\t\tSDA: "     << (int)pinout.i2c[bus].pin_sda << std::endl;
		}
	}

	std::cout << "\tPWM" << std::endl;
	for (int slice = 0; slice < PICO_HUB_PWM_SLICES_COUNT; ++slice)
	{
		if (pinout.pwm[slice].is_open)
		{
			std::cout << "\t\tSlice: "   << slice << std::endl;
			std::cout << "\t\tWrap: "    << pinout.pwm[slice].wrap << std::endl;
			std::cout << "\t\tClock: "   << (clock / pinout.pwm[slice].clkdiv) << std::endl;
			std::cout << "\t\tChannel A" << std::endl;
			std::cout << "\t\t\tPin: "   << (uint8_t)pinout.pwm[slice].channels[PICO_HUB_PWM_CHANNEL_A].pin << std::endl;
			std::cout << "\t\t\tLevel: " << pinout.pwm[slice].channels[PICO_HUB_PWM_CHANNEL_A].level << std::endl;
			std::cout << "\t\tChannel B" << std::endl;
			std::cout << "\t\t\tPin: "   << (uint8_t)pinout.pwm[slice].channels[PICO_HUB_PWM_CHANNEL_B].pin << std::endl;
			std::cout << "\t\t\tLevel: " << pinout.pwm[slice].channels[PICO_HUB_PWM_CHANNEL_B].level << std::endl;
		}
	}

	std::cout << "\tSPI" << std::endl;
	for (int bus = 0; bus < PICO_HUB_SPI_BUS_COUNT; ++bus)
	{
		if (pinout.spi[bus].is_open)
		{
			std::cout << "\t\tBus: "   << bus << std::endl;
			std::cout << "\t\tBaud: "  << pinout.spi[bus].baud << std::endl;
			std::cout << "\t\tSlave: " << (pinout.spi[bus].slave ? "True" : "False") << std::endl;
			std::cout << "\t\tCS: "    << (uint8_t)pinout.spi[bus].pin_cs << std::endl;
			std::cout << "\t\tMISO: "  << (uint8_t)pinout.spi[bus].pin_miso << std::endl;
			std::cout << "\t\tMOSI: "  << (uint8_t)pinout.spi[bus].pin_mosi << std::endl;
			std::cout << "\t\tSCLK: "  << (uint8_t)pinout.spi[bus].pin_clock << std::endl;
		}
	}

	std::cout << "\tGPIO" << std::endl;
	for (int gpio = 0; gpio < PICO_HUB_GPIO_COUNT; ++gpio)
	{
		if (pinout.gpio[gpio].is_open)
		{
			std::cout << "\t\tPin: " << gpio << std::endl;
			std::cout << "\t\tFlags: 0x" << std::hex << pinout.gpio[gpio].flags << std::dec << std::endl;
			if (pinout.gpio[gpio].flags & PICO_HUB_GPIO_FLAG_FUNCTION_NONE) std::cout << "\t\tFunction: None" << std::endl;
			if (pinout.gpio[gpio].flags & PICO_HUB_GPIO_FLAG_FUNCTION_ADC)  std::cout << "\t\tFunction: ADC" << std::endl;
			if (pinout.gpio[gpio].flags & PICO_HUB_GPIO_FLAG_FUNCTION_I2C)  std::cout << "\t\tFunction: I2C" << std::endl;
			if (pinout.gpio[gpio].flags & PICO_HUB_GPIO_FLAG_FUNCTION_PWM)  std::cout << "\t\tFunction: PWM" << std::endl;
			if (pinout.gpio[gpio].flags & PICO_HUB_GPIO_FLAG_FUNCTION_SPI)  std::cout << "\t\tFunction: SPI" << std::endl;
			if (pinout.gpio[gpio].flags & PICO_HUB_GPIO_FLAG_FUNCTION_GPIO) std::cout << "\t\tFunction: GPIO" << std::endl;
			if (pinout.gpio[gpio].flags & PICO_HUB_GPIO_FLAG_FUNCTION_UART) std::cout << "\t\tFunction: UART" << std::endl;
			if (pinout.gpio[gpio].flags & PICO_HUB_GPIO_FLAG_DIRECTION_IN)  std::cout << "\t\tDirection: Input" << std::endl;
			if (pinout.gpio[gpio].flags & PICO_HUB_GPIO_FLAG_DIRECTION_OUT) std::cout << "\t\tDirection: Output" << std::endl;
			if (pinout.gpio[gpio].flags & PICO_HUB_GPIO_FLAG_VALUE_LOW)     std::cout << "\t\tValue: Low" << std::endl;
			if (pinout.gpio[gpio].flags & PICO_HUB_GPIO_FLAG_VALUE_HIGH)    std::cout << "\t\tValue: High" << std::endl;
		}
	}

	std::cout << "\tUART" << std::endl;
	for (int bus = 0; bus < PICO_HUB_UART_BUS_COUNT; ++bus)
	{
		if (pinout.uart[bus].is_open)
		{
			std::cout << "\t\tBus: "  << bus << std::endl;
			std::cout << "\t\tBaud: " << pinout.uart[bus].baud << std::endl;
			std::cout << "\t\tRX: "   << (int)pinout.uart[bus].pin_rx << std::endl;
			std::cout << "\t\tTX: "   << (int)pinout.uart[bus].pin_tx << std::endl;
		}
	}

	return PICO_HUB_ERROR_NONE;
}

int main(int argc, char* argv[])
{
	uint64_t         id;
	bool             led;
	uint32_t         clock;
	uint32_t         latency;
	PICO_HUB_VOLTAGE voltage;
	uint16_t         adc_value;

	if (auto hub = pico_hub_open("COM3"))
	{
		if (!pico_hub_get_id(hub, &id))
			std::cout << "ID = 0x" << std::hex << id << std::dec << std::endl;

		if (!pico_hub_get_led(hub, &led))
			std::cout << "LED = " << (led ? "On" : "Off") << std::endl;

		if (!pico_hub_get_clock(hub, &clock))
			std::cout << "CPU Clock = " << (clock / 1000000.0f) << " MHz" << std::endl;

		if (!pico_hub_get_voltage(hub, &voltage))
			std::cout << "CPU Voltage = " << pico_hub_voltage_to_string(voltage) << std::endl;

		if (!pico_hub_get_latency_us(hub, &latency, 1000))
		{
			std::cout << "Latency Average = " << latency << " us" << std::endl;
			std::cout << "Latency Samples = " << 1000 << std::endl;
		}

		pico_hub_get_and_display_pinout(hub);

		if (!pico_hub_adc_init(hub, (PICO_HUB_ADC)(PICO_HUB_ADC_GPIO_26 | PICO_HUB_ADC_GPIO_27 | PICO_HUB_ADC_GPIO_28 | PICO_HUB_ADC_SYSTEM_VOLTAGE | PICO_HUB_ADC_SYSTEM_TEMPERATURE)))
		{
			if (!pico_hub_adc_set_channel(hub, PICO_HUB_ADC_GPIO_26))
				if (!pico_hub_adc_read(hub, &adc_value))
					std::cout << "ADC0 = " << adc_value << std::endl;

			if (!pico_hub_adc_set_channel(hub, PICO_HUB_ADC_GPIO_27))
				if (!pico_hub_adc_read(hub, &adc_value))
					std::cout << "ADC1 = " << adc_value << std::endl;

			if (!pico_hub_adc_set_channel(hub, PICO_HUB_ADC_GPIO_28))
				if (!pico_hub_adc_read(hub, &adc_value))
					std::cout << "ADC2 = " << adc_value << std::endl;

			if (!pico_hub_adc_set_channel(hub, PICO_HUB_ADC_SYSTEM_VOLTAGE))
				if (!pico_hub_adc_read(hub, &adc_value))
					std::cout << "ADC3 = " << adc_value << std::endl;

			if (!pico_hub_adc_set_channel(hub, PICO_HUB_ADC_SYSTEM_TEMPERATURE))
				if (!pico_hub_adc_read(hub, &adc_value))
					std::cout << "ADC4 = " << adc_value << std::endl;

			pico_hub_adc_deinit(hub);
		}

		if (uint8_t pwm_slice, pwm_channel; !pico_hub_pwm_get_slice_and_channel(hub, PICO_HUB_GPIO_LED, &pwm_slice, &pwm_channel))
			if (!pico_hub_pwm_init(hub, pwm_slice, 0xFFFF, 0x7FFF, 255))
			{
				pico_hub_pwm_set_enabled(hub, pwm_slice, true);

				pico_hub_pwm_deinit(hub, pwm_slice);
			}

		if (!pico_hub_i2c_init(hub, PICO_HUB_I2C_BUS_0, 1, 0, 1000 * 1000, 0, false))
		{
			std::cout << "Scanning i2c0" << std::endl;

			size_t device_count = 0;

			pico_hub_i2c_scan(hub, PICO_HUB_I2C_BUS_0, [](pico_hub* hub, uint8_t address, void* param) {
				std::cout << '[' << ++*((uint8_t*)param) << "] Found " << std::hex << address << std::dec << std::endl;
			}, &device_count);

			pico_hub_i2c_deinit(hub, PICO_HUB_I2C_BUS_0);
		}

		pico_hub_close(hub);
	}

	return 0;
}
