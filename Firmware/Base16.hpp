#pragma once
#include <cstddef>
#include <cstdint>

struct pico_hub_base16_context
{
	const char    table[16];
	const uint8_t table_reverse[71];
};

constexpr const pico_hub_base16_context pico_hub_base16 =
{
	.table         = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' },
	.table_reverse = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 10, 11, 12, 13, 14, 15 }
};

inline void pico_hub_base16_encode(char& c1, char& c2, uint8_t value)
{
	c1 = pico_hub_base16.table[value >> 4];
	c2 = pico_hub_base16.table[value & 0x0F];
}

static bool pico_hub_base16_decode(char c1, char c2, uint8_t& value)
{
	if (c1 >= sizeof(pico_hub_base16_context::table_reverse))
		return false;

	if (c2 >= sizeof(pico_hub_base16_context::table_reverse))
		return false;

	uint8_t v1 = pico_hub_base16.table_reverse[c1];
	uint8_t v2 = pico_hub_base16.table_reverse[c2];

	if ((v1 == 0xFF) || (v2 == 0xFF))
		return false;

	value = (v1 << 4) | v2;

	return true;
}
