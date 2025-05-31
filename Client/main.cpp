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

		if (!pico_hub_i2c_init(hub, PICO_HUB_I2C_BUS_0, 1, 0, 1000 * 1000))
		{
			std::cout << "Scanning i2c0" << std::endl;

			pico_hub_i2c_scan(hub, PICO_HUB_I2C_BUS_0, [](pico_hub* hub, uint8_t address, void* param) {
				std::cout << "Found " << std::hex << address << std::dec << std::endl;
			}, nullptr);

			pico_hub_i2c_deinit(hub, PICO_HUB_I2C_BUS_0);
		}

		pico_hub_close(hub);
	}

	return 0;
}
