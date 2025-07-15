#include "PicoHub.hpp"

int main()
{
	bool restart = false;

	do
	{
		if (pico_hub_init())
		{
			restart = pico_hub_run();

			pico_hub_deinit();
		}
	} while (restart);

	return 0;
}
