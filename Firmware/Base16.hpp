#pragma once
#include <cstdint>

constexpr void pico_hub_base16_encode(char& c1, char& c2, uint8_t value)
{
	c1 = 'a' + (value >> 4);
	c2 = 'a' + (value & 0x0F);
}

constexpr bool pico_hub_base16_decode(char c1, char c2, uint8_t& value)
{
	if ((c1 >= 'a') && (c1 <= 'p'))
		if ((c2 >= 'a') && (c2 <= 'p'))
		{
			value = ((c1 - 'a') << 4) | (c2 - 'a');

			return true;
		}

	return false;
}
