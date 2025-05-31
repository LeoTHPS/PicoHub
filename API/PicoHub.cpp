#include "PicoHub.hpp"

#include <Firmware/Base16.hpp>

#include <chrono>
#include <utility>
#include <iostream>
#include <unordered_map>

#if defined(PICO_HUB_API_UNIX)
	#include <fcntl.h>
	#include <unistd.h>
	#include <termios.h> // speed_t

	#include <sys/ioctl.h>
#elif defined(PICO_HUB_API_WIN32)
	#include <windows.h>
#endif

struct pico_hub
{
	pico_hub_io*     io;
	pico_hub_packets packets;
};

struct pico_hub_io
{
	std::string device;
#if defined(PICO_HUB_API_UNIX)
	int         handle;
#elif defined(PICO_HUB_API_WIN32)
	HANDLE      handle;
#endif
};

pico_hub_io* pico_hub_io_open(const char* device)
{
	auto io = new pico_hub_io
	{
		.device = device
	};

#if defined(PICO_HUB_API_UNIX)
	if ((io->handle = open(device, O_RDWR | O_NOCTTY | O_NDELAY)) == -1)
	{
		auto error = errno;

		std::cerr << "Error calling open: " << error << std::endl;

		delete io;

		return nullptr;
	}

	fcntl(io->handle, F_SETFL, O_RDWR);

	termios options;
	tcgetattr(io->handle, &options);
	cfmakeraw(&options);
	cfsetspeed(&options, 115200);

	options.c_cc[VMIN]  = 1;
	options.c_cc[VTIME] = 0;
	options.c_cflag    |= CS8;
	options.c_cflag    |= CLOCAL | CREAD;
	options.c_cflag    &= ~(PARENB | CSTOPB | CSIZE);
	options.c_cflag    &= ~(ICANON | ECHO | ECHOE | ISIG | OPOST);
	tcsetattr(io->handle, TCSANOW, &options);

	int status;
	ioctl(io->handle, TIOCMGET, &status);
	status |= TIOCM_DTR | TIOCM_RTS;
	ioctl(io->handle, TIOCMSET, &status);
#elif defined(PICO_HUB_API_WIN32)
	if ((io->handle = CreateFileA(device, GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, 0, 0)) == INVALID_HANDLE_VALUE)
	{
		auto error = GetLastError();

		std::cerr << "Error calling CreateFileA: " << error << std::endl;

		delete io;

		return nullptr;
	}

	DCB dcb = { .DCBlength = sizeof(DCB) };

	if (!GetCommState(io->handle, &dcb))
	{
		auto error = GetLastError();

		std::cerr << "Error calling GetCommState: " << error << std::endl;

		CloseHandle(io->handle);

		delete io;

		return nullptr;
	}

	dcb.Parity        = NOPARITY;
	dcb.fBinary       = TRUE;
	dcb.BaudRate      = CBR_115200;
	dcb.ByteSize      = 8;
	dcb.StopBits      = ONESTOPBIT;
	dcb.fDtrControl   = DTR_CONTROL_ENABLE;
	dcb.fRtsControl   = RTS_CONTROL_DISABLE;
	dcb.fAbortOnError = TRUE;

	if (!SetCommState(io->handle, &dcb))
	{
		auto error = GetLastError();

		std::cerr << "Error calling SetCommState: " << error << std::endl;

		CloseHandle(io->handle);

		delete io;

		return nullptr;
	}

	COMMTIMEOUTS timeouts =
	{
		.ReadIntervalTimeout         = MAXWORD,
		.ReadTotalTimeoutMultiplier  = 0,
		.ReadTotalTimeoutConstant    = 0,
		.WriteTotalTimeoutMultiplier = 10,
		.WriteTotalTimeoutConstant   = 50
	};

	if (!SetCommTimeouts(io->handle, &timeouts))
	{
		auto error = GetLastError();

		std::cerr << "Error calling SetCommTimeouts: " << error << std::endl;

		CloseHandle(io->handle);

		delete io;

		return nullptr;
	}
#endif

	return io;
}
void         pico_hub_io_close(pico_hub_io* io)
{
#if defined(PICO_HUB_API_UNIX)
	close(io->handle);
#elif defined(PICO_HUB_API_WIN32)
	CloseHandle(io->handle);
#endif

	delete io;
}
bool         pico_hub_io_send(pico_hub_io* io, const void* buffer, size_t size)
{
#if defined(PICO_HUB_API_WIN32)
	// if (!SetCommMask(io->handle, EV_TXEMPTY))
	// {
	// 	auto error = GetLastError();

	// 	std::cerr << "Error calling SetCommMask: " << error << std::endl;

	// 	return false;
	// }
#endif

	std::string base16(size * 2, '\0');

	{
		auto value  = (const uint8_t*)buffer;
		auto string = &base16[0];

		for (size_t i = 0; i < size; ++i, string += 2, ++value)
			pico_hub_base16_encode(string[0], string[1], *value);
	}

	size_t total_bytes_sent = 0;

	while (total_bytes_sent < base16.length())
	{
#if defined(PICO_HUB_API_UNIX)
		int bytes_sent;

		if ((bytes_sent = write(io->handle, &base16[total_bytes_sent], base16.length() - total_bytes_sent)) == -1)
		{
			auto error = errno;

			std::cerr << "Error calling write: " << error << std::endl;

			return false;
		}

		total_bytes_sent += bytes_sent;
#elif defined(PICO_HUB_API_WIN32)
		DWORD bytes_sent;

		if (!WriteFile(io->handle, &base16[total_bytes_sent], static_cast<DWORD>(base16.length() - total_bytes_sent), &bytes_sent, nullptr))
		{
			auto error = GetLastError();

			std::cerr << "Error calling WriteFile: " << error << std::endl;

			return false;
		}

		total_bytes_sent += bytes_sent;
#endif
	}

#if defined(PICO_HUB_API_WIN32)
	// DWORD event;

	// if (!WaitCommEvent(io->handle, &event, NULL))
	// {
	// 	auto error = GetLastError();

	// 	std::cerr << "Error calling WaitCommEvent: " << error << std::endl;

	// 	return false;
	// }
#endif

	return true;
}
bool         pico_hub_io_receive(pico_hub_io* io, void* buffer, size_t size)
{
#if defined(PICO_HUB_API_WIN32)
	// DWORD event;

	// if (!SetCommMask(io->handle, EV_RXCHAR))
	// {
	// 	auto error = GetLastError();

	// 	std::cerr << "Error calling SetCommMask: " << error << std::endl;

	// 	return false;
	// }

	// if (!WaitCommEvent(io->handle, &event, NULL))
	// {
	// 	auto error = GetLastError();

	// 	std::cerr << "Error calling WaitCommEvent: " << error << std::endl;

	// 	return false;
	// }
#endif

	std::string base16(size * 2, '\0');

	size_t total_bytes_received = 0;

	while (total_bytes_received < base16.length())
	{
#if defined(PICO_HUB_API_UNIX)
		int bytes_received;

		if ((bytes_received = read(io->handle, &base16[total_bytes_received], base16.length() - total_bytes_received)) == -1)
		{
			auto error = errno;

			std::cerr << "Error calling read: " << error << std::endl;

			return false;
		}

		total_bytes_received += bytes_received;
#elif defined(PICO_HUB_API_WIN32)
		DWORD bytes_received;

		if (!ReadFile(io->handle, &base16[total_bytes_received], static_cast<DWORD>(base16.length() - total_bytes_received), &bytes_received, nullptr))
		{
			auto error = GetLastError();

			std::cerr << "Error calling ReadFile: " << error << std::endl;

			return false;
		}

		total_bytes_received += bytes_received;
#endif
	}

	auto value  = (uint8_t*)buffer;
	auto string = base16.c_str();

	for (size_t i = 0; i < size; ++i, string += 2, ++value)
		if (!pico_hub_base16_decode(string[0], string[1], *value))
			return false;

	return true;
}
template<PICO_HUB_OPCODES OPCODE>
bool         pico_hub_io_send_request(pico_hub_io* io, const pico_hub_packet_request<OPCODE>& packet)
{
	pico_hub_packet_header header =
	{
		.opcode = (PICO_HUB_OPCODES)((PICO_HUB_PACKET_TYPE_REQUEST << 6) | (OPCODE & 0x3F)),
		.length = sizeof(pico_hub_packet_request<OPCODE>)
	};

	if (!pico_hub_io_send(io, &header, sizeof(pico_hub_packet_header)))
	{
		std::cerr << "Error sending request header" << std::endl;

		return false;
	}

	if (!pico_hub_io_send(io, &packet, header.length))
	{
		std::cerr << "Error sending request payload" << std::endl;

		return false;
	}

	return true;
}
template<PICO_HUB_OPCODES OPCODE>
bool         pico_hub_io_receive_response(pico_hub_io* io, pico_hub_packet_response<OPCODE>& packet)
{
	pico_hub_packet_header header;

	if (!pico_hub_io_receive(io, &header, sizeof(pico_hub_packet_header)))
	{
		std::cerr << "Error receiving response header" << std::endl;

		return false;
	}

	if ((((header.opcode & 0xC0) >> 6) != PICO_HUB_PACKET_TYPE_RESPONSE) || ((header.opcode & 0x3F) != OPCODE) || (header.length != sizeof(pico_hub_packet_response<OPCODE>)))
	{
		std::cerr << "Received invalid response header" << std::endl;

		return false;
	}

	if (!pico_hub_io_receive(io, &packet, header.length))
	{
		std::cerr << "Error receiving response payload" << std::endl;

		return false;
	}

	return true;
}
template<PICO_HUB_OPCODES OPCODE>
bool         pico_hub_io_send_request_receive_response(pico_hub_io* io, const pico_hub_packet_request<OPCODE>& request, pico_hub_packet_response<OPCODE>& response)
{
	if (!pico_hub_io_send_request(io, request))
	{
		std::cerr << "Error sending request [OPCode: " << (int)OPCODE << "]" << std::endl;

		return false;
	}

	if (!pico_hub_io_receive_response(io, response))
	{
		std::cerr << "Error receiving response [OPCode: " << (int)OPCODE << "]" << std::endl;

		return false;
	}

	return true;
}

pico_hub* pico_hub_open(const char* device)
{
	auto hub = new pico_hub
	{
	};

	if (!(hub->io = pico_hub_io_open(device)))
	{
		std::cerr << "Error opening io" << std::endl;

		delete hub;

		return nullptr;
	}

	return hub;
}
void      pico_hub_close(pico_hub* hub)
{
	pico_hub_io_close(hub->io);

	delete hub;
}

int       pico_hub_get_id(pico_hub* hub, uint64_t* value)
{
	pico_hub_packet_request<PICO_HUB_OPCODE_GET_ID>  request;
	pico_hub_packet_response<PICO_HUB_OPCODE_GET_ID> response;

	if (!pico_hub_io_send_request_receive_response(hub->io, request, response))
		return PICO_HUB_ERROR_IO_ERROR;

	*value = response.value;

	return PICO_HUB_ERROR_NONE;
}

int       pico_hub_get_led(pico_hub* hub, bool* value)
{
	pico_hub_packet_request<PICO_HUB_OPCODE_GET_LED>  request;
	pico_hub_packet_response<PICO_HUB_OPCODE_GET_LED> response;

	if (!pico_hub_io_send_request_receive_response(hub->io, request, response))
		return PICO_HUB_ERROR_IO_ERROR;

	if (!response.success)
		return PICO_HUB_ERROR_REQUEST_FAILED;

	*value = response.value;

	return PICO_HUB_ERROR_NONE;
}
int       pico_hub_set_led(pico_hub* hub, bool value)
{
	pico_hub_packet_request<PICO_HUB_OPCODE_SET_LED> request =
	{
		.value = value
	};

	pico_hub_packet_response<PICO_HUB_OPCODE_SET_LED> response;

	if (!pico_hub_io_send_request_receive_response(hub->io, request, response))
		return PICO_HUB_ERROR_IO_ERROR;
		
	if (!response.success)
		return PICO_HUB_ERROR_REQUEST_FAILED;

	return PICO_HUB_ERROR_NONE;
}

int       pico_hub_get_clock(pico_hub* hub, std::uint32_t* value)
{
	pico_hub_packet_request<PICO_HUB_OPCODE_GET_CLOCK>  request;
	pico_hub_packet_response<PICO_HUB_OPCODE_GET_CLOCK> response;

	if (!pico_hub_io_send_request_receive_response(hub->io, request, response))
		return PICO_HUB_ERROR_IO_ERROR;

	*value = response.value;

	return PICO_HUB_ERROR_NONE;
}
int       pico_hub_set_clock(pico_hub* hub, std::uint32_t value)
{
	pico_hub_packet_request<PICO_HUB_OPCODE_SET_CLOCK> request =
	{
		.value = value
	};

	pico_hub_packet_response<PICO_HUB_OPCODE_SET_CLOCK> response;

	if (!pico_hub_io_send_request_receive_response(hub->io, request, response))
		return PICO_HUB_ERROR_IO_ERROR;
		
	if (!response.success)
		return PICO_HUB_ERROR_REQUEST_FAILED;

	return PICO_HUB_ERROR_NONE;
}

template<typename DURATION>
int       pico_hub_get_latency(pico_hub* hub, uint32_t* value)
{
	auto time = std::chrono::steady_clock::now();

	pico_hub_packet_request<PICO_HUB_OPCODE_GET_LATENCY>  request;
	pico_hub_packet_response<PICO_HUB_OPCODE_GET_LATENCY> response;

	if (!pico_hub_io_send_request_receive_response(hub->io, request, response))
		return PICO_HUB_ERROR_IO_ERROR;

	*value = std::chrono::duration_cast<DURATION>(std::chrono::steady_clock::now() - time).count();

	return PICO_HUB_ERROR_NONE;
}
int       pico_hub_get_latency_ms(pico_hub* hub, uint32_t* value)
{
	return pico_hub_get_latency<std::chrono::milliseconds>(hub, value);
}
int       pico_hub_get_latency_us(pico_hub* hub, uint32_t* value)
{
	return pico_hub_get_latency<std::chrono::microseconds>(hub, value);
}

int       pico_hub_get_voltage(pico_hub* hub, PICO_HUB_VOLTAGE* value)
{
	pico_hub_packet_request<PICO_HUB_OPCODE_GET_VOLTAGE>  request;
	pico_hub_packet_response<PICO_HUB_OPCODE_GET_VOLTAGE> response;

	if (!pico_hub_io_send_request_receive_response(hub->io, request, response))
		return PICO_HUB_ERROR_IO_ERROR;

	*value = response.value;

	return PICO_HUB_ERROR_NONE;
}
int       pico_hub_set_voltage(pico_hub* hub, PICO_HUB_VOLTAGE value)
{
	pico_hub_packet_request<PICO_HUB_OPCODE_SET_VOLTAGE> request =
	{
		.value = value
	};

	pico_hub_packet_response<PICO_HUB_OPCODE_SET_VOLTAGE> response;

	if (!pico_hub_io_send_request_receive_response(hub->io, request, response))
		return PICO_HUB_ERROR_IO_ERROR;

	if (!response.success)
		return PICO_HUB_ERROR_REQUEST_FAILED;

	return PICO_HUB_ERROR_NONE;
}

int       pico_hub_restart(pico_hub* hub, bool restart_to_mass_storage)
{
	pico_hub_packet_request<PICO_HUB_OPCODE_RESTART> request =
	{
		.restart_to_mass_storage = restart_to_mass_storage
	};

	pico_hub_packet_response<PICO_HUB_OPCODE_RESTART> response;

	if (!pico_hub_io_send_request_receive_response(hub->io, request, response))
		return PICO_HUB_ERROR_IO_ERROR;

	return PICO_HUB_ERROR_NONE;
}
int       pico_hub_shutdown(pico_hub* hub)
{
	pico_hub_packet_request<PICO_HUB_OPCODE_SHUTDOWN>  request;
	pico_hub_packet_response<PICO_HUB_OPCODE_SHUTDOWN> response;

	if (!pico_hub_io_send_request_receive_response(hub->io, request, response))
		return PICO_HUB_ERROR_IO_ERROR;

	return PICO_HUB_ERROR_NONE;
}

int       pico_hub_adc_init(pico_hub* hub, PICO_HUB_ADC channels)
{
	pico_hub_packet_request<PICO_HUB_OPCODE_ADC_INIT> request =
	{
		.channels = channels
	};

	pico_hub_packet_response<PICO_HUB_OPCODE_ADC_INIT> response;

	if (!pico_hub_io_send_request_receive_response(hub->io, request, response))
		return PICO_HUB_ERROR_IO_ERROR;

	if (!response.success)
		return PICO_HUB_ERROR_REQUEST_FAILED;

	return PICO_HUB_ERROR_NONE;
}
int       pico_hub_adc_deinit(pico_hub* hub)
{
	pico_hub_packet_request<PICO_HUB_OPCODE_ADC_DEINIT>  request;
	pico_hub_packet_response<PICO_HUB_OPCODE_ADC_DEINIT> response;

	if (!pico_hub_io_send_request_receive_response(hub->io, request, response))
		return PICO_HUB_ERROR_IO_ERROR;

	return PICO_HUB_ERROR_NONE;
}
int       pico_hub_adc_get_channel(pico_hub* hub, PICO_HUB_ADC* value)
{
	pico_hub_packet_request<PICO_HUB_OPCODE_ADC_GET_CHANNEL>  request;
	pico_hub_packet_response<PICO_HUB_OPCODE_ADC_GET_CHANNEL> response;

	if (!pico_hub_io_send_request_receive_response(hub->io, request, response))
		return PICO_HUB_ERROR_IO_ERROR;

	if (!response.success)
		return PICO_HUB_ERROR_REQUEST_FAILED;

	*value = response.value;

	return PICO_HUB_ERROR_NONE;
}
int       pico_hub_adc_set_channel(pico_hub* hub, PICO_HUB_ADC value)
{
	pico_hub_packet_request<PICO_HUB_OPCODE_ADC_SET_CHANNEL> request =
	{
		.value = value
	};

	pico_hub_packet_response<PICO_HUB_OPCODE_ADC_SET_CHANNEL> response;

	if (!pico_hub_io_send_request_receive_response(hub->io, request, response))
		return PICO_HUB_ERROR_IO_ERROR;

	if (!response.success)
		return PICO_HUB_ERROR_REQUEST_FAILED;

	return PICO_HUB_ERROR_NONE;
}
int       pico_hub_adc_read(pico_hub* hub, uint16_t* value)
{
	pico_hub_packet_request<PICO_HUB_OPCODE_ADC_READ>  request;
	pico_hub_packet_response<PICO_HUB_OPCODE_ADC_READ> response;

	if (!pico_hub_io_send_request_receive_response(hub->io, request, response))
		return PICO_HUB_ERROR_IO_ERROR;

	if (!response.success)
		return PICO_HUB_ERROR_REQUEST_FAILED;

	*value = response.value;

	return PICO_HUB_ERROR_NONE;
}

int       pico_hub_i2c_init(pico_hub* hub, PICO_HUB_I2C bus, uint8_t scl, uint8_t sda, uint32_t baud)
{
	pico_hub_packet_request<PICO_HUB_OPCODE_I2C_INIT> request =
	{
		.bus  = bus,
		.scl  = scl,
		.sda  = sda,
		.baud = baud
	};
	pico_hub_packet_response<PICO_HUB_OPCODE_I2C_INIT> response;

	if (!pico_hub_io_send_request_receive_response(hub->io, request, response))
		return PICO_HUB_ERROR_IO_ERROR;

	if (!response.success)
		return PICO_HUB_ERROR_REQUEST_FAILED;

	return PICO_HUB_ERROR_NONE;
}
int       pico_hub_i2c_deinit(pico_hub* hub, PICO_HUB_I2C bus)
{
	pico_hub_packet_request<PICO_HUB_OPCODE_I2C_DEINIT> request =
	{
		.bus = bus
	};
	pico_hub_packet_response<PICO_HUB_OPCODE_I2C_DEINIT> response;

	if (!pico_hub_io_send_request_receive_response(hub->io, request, response))
		return PICO_HUB_ERROR_IO_ERROR;

	return PICO_HUB_ERROR_NONE;
}
int       pico_hub_i2c_scan(pico_hub* hub, PICO_HUB_I2C bus, pico_hub_i2c_scan_callback callback, void* param)
{
	auto is_address_reserved = [](uint8_t address)
	{
		return !(address & 0x78) || ((address & 0x78) == 0x78);
	};

	uint8_t data;

	for (uint8_t address = 0; address < 0x80; ++address)
	{
		if (is_address_reserved(address))
			continue;

		switch (auto error = pico_hub_i2c_read(hub, bus, address, &data, 1, true))
		{
			case PICO_HUB_ERROR_NONE:
				callback(hub, address, param);
				break;

			case PICO_HUB_ERROR_REQUEST_FAILED:
				break;

			default:
				return error;
		}
	}

	return PICO_HUB_ERROR_NONE;
}
int       pico_hub_i2c_read(pico_hub* hub, PICO_HUB_I2C bus, uint8_t address, void* buffer, size_t size, bool stop)
{
	pico_hub_packet_request<PICO_HUB_OPCODE_I2C_READ> request =
	{
		.bus     = bus,
		.address = address,
		.size    = (uint32_t)size,
		.stop    = stop
	};
	pico_hub_packet_response<PICO_HUB_OPCODE_I2C_READ> response;

	if (!pico_hub_io_send_request_receive_response(hub->io, request, response))
		return PICO_HUB_ERROR_IO_ERROR;

	if (!response.success)
		return PICO_HUB_ERROR_REQUEST_FAILED;

	if (!pico_hub_io_receive(hub->io, buffer, size))
		return PICO_HUB_ERROR_IO_ERROR;

	return PICO_HUB_ERROR_NONE;
}
int       pico_hub_i2c_write(pico_hub* hub, PICO_HUB_I2C bus, uint8_t address, const void* buffer, size_t size, bool stop)
{
	pico_hub_packet_request<PICO_HUB_OPCODE_I2C_WRITE> request =
	{
		.bus     = bus,
		.address = address,
		.size    = (uint32_t)size,
		.stop    = stop
	};
	pico_hub_packet_response<PICO_HUB_OPCODE_I2C_WRITE> response;

	if (!pico_hub_io_send_request(hub->io, request))
		return PICO_HUB_ERROR_IO_ERROR;

	if (!pico_hub_io_send(hub->io, buffer, size))
		return PICO_HUB_ERROR_IO_ERROR;

	if (!pico_hub_io_receive_response(hub->io, response))
		return PICO_HUB_ERROR_IO_ERROR;

	if (!response.success)
		return PICO_HUB_ERROR_REQUEST_FAILED;

	return PICO_HUB_ERROR_NONE;
}
int       pico_hub_i2c_write_read(pico_hub* hub, PICO_HUB_I2C bus, uint8_t address, const void* tx, size_t tx_size, void* rx, size_t rx_size)
{
	pico_hub_packet_request<PICO_HUB_OPCODE_I2C_WRITE_READ> request =
	{
		.bus     = bus,
		.address = address,
		.size    = { (uint32_t)tx_size, (uint32_t)rx_size }
	};
	pico_hub_packet_response<PICO_HUB_OPCODE_I2C_WRITE_READ> response;

	if (!pico_hub_io_send_request(hub->io, request))
		return PICO_HUB_ERROR_IO_ERROR;

	if (!pico_hub_io_send(hub->io, tx, tx_size))
		return PICO_HUB_ERROR_IO_ERROR;

	if (!pico_hub_io_receive_response(hub->io, response))
		return PICO_HUB_ERROR_IO_ERROR;

	if (!response.success)
		return PICO_HUB_ERROR_REQUEST_FAILED;

	if (!pico_hub_io_receive(hub->io, rx, rx_size))
		return PICO_HUB_ERROR_IO_ERROR;

	return PICO_HUB_ERROR_NONE;
}

int       pico_hub_spi_init(pico_hub* hub, PICO_HUB_SPI bus, uint8_t miso, uint8_t mosi, uint8_t clock, uint8_t cs, uint32_t baud, bool slave)
{
	pico_hub_packet_request<PICO_HUB_OPCODE_SPI_INIT> request =
	{
		.bus   = bus,
		.miso  = miso,
		.mosi  = mosi,
		.clock = clock,
		.cs    = cs,
		.baud  = baud,
		.slave = slave
	};
	pico_hub_packet_response<PICO_HUB_OPCODE_SPI_INIT> response;

	if (!pico_hub_io_send_request_receive_response(hub->io, request, response))
		return PICO_HUB_ERROR_IO_ERROR;

	if (!response.success)
		return PICO_HUB_ERROR_REQUEST_FAILED;

	return PICO_HUB_ERROR_NONE;
}
int       pico_hub_spi_deinit(pico_hub* hub, PICO_HUB_SPI bus)
{
	pico_hub_packet_request<PICO_HUB_OPCODE_SPI_DEINIT> request =
	{
		.bus = bus
	};
	pico_hub_packet_response<PICO_HUB_OPCODE_SPI_DEINIT> response;

	if (!pico_hub_io_send_request_receive_response(hub->io, request, response))
		return PICO_HUB_ERROR_IO_ERROR;

	return PICO_HUB_ERROR_NONE;
}
int       pico_hub_spi_read(pico_hub* hub, PICO_HUB_SPI bus, void* buffer, size_t size)
{
	pico_hub_packet_request<PICO_HUB_OPCODE_SPI_READ> request =
	{
		.bus  = bus,
		.size = (uint32_t)size
	};
	pico_hub_packet_response<PICO_HUB_OPCODE_SPI_READ> response;

	if (!pico_hub_io_send_request_receive_response(hub->io, request, response))
		return PICO_HUB_ERROR_IO_ERROR;

	if (!response.success)
		return PICO_HUB_ERROR_REQUEST_FAILED;

	if (!pico_hub_io_receive(hub->io, buffer, size))
		return PICO_HUB_ERROR_IO_ERROR;

	return PICO_HUB_ERROR_NONE;
}
int       pico_hub_spi_write(pico_hub* hub, PICO_HUB_SPI bus, const void* buffer, size_t size)
{
	pico_hub_packet_request<PICO_HUB_OPCODE_SPI_WRITE> request =
	{
		.bus  = bus,
		.size = (uint32_t)size
	};
	pico_hub_packet_response<PICO_HUB_OPCODE_SPI_WRITE> response;

	if (!pico_hub_io_send_request(hub->io, request))
		return PICO_HUB_ERROR_IO_ERROR;

	if (!pico_hub_io_send(hub->io, buffer, size))
		return PICO_HUB_ERROR_IO_ERROR;

	if (!pico_hub_io_receive_response(hub->io, response))
		return PICO_HUB_ERROR_IO_ERROR;

	if (!response.success)
		return PICO_HUB_ERROR_REQUEST_FAILED;

	return PICO_HUB_ERROR_NONE;
}
int       pico_hub_spi_write_read(pico_hub* hub, PICO_HUB_SPI bus, const void* tx, void* rx, size_t size)
{
	pico_hub_packet_request<PICO_HUB_OPCODE_SPI_WRITE_READ> request =
	{
		.bus  = bus,
		.size = (uint32_t)size
	};
	pico_hub_packet_response<PICO_HUB_OPCODE_SPI_WRITE_READ> response;

	if (!pico_hub_io_send_request(hub->io, request))
		return PICO_HUB_ERROR_IO_ERROR;

	if (!pico_hub_io_send(hub->io, tx, size))
		return PICO_HUB_ERROR_IO_ERROR;

	if (!pico_hub_io_receive_response(hub->io, response))
		return PICO_HUB_ERROR_IO_ERROR;

	if (!response.success)
		return PICO_HUB_ERROR_REQUEST_FAILED;

	if (!pico_hub_io_receive(hub->io, rx, size))
		return PICO_HUB_ERROR_IO_ERROR;

	return PICO_HUB_ERROR_NONE;
}

int       pico_hub_gpio_init(pico_hub* hub, uint8_t pin, bool direction, bool value)
{
	pico_hub_packet_request<PICO_HUB_OPCODE_GPIO_INIT> request =
	{
		.pin       = pin,
		.direction = direction,
		.value     = value
	};
	pico_hub_packet_response<PICO_HUB_OPCODE_GPIO_INIT> response;

	if (!pico_hub_io_send_request_receive_response(hub->io, request, response))
		return PICO_HUB_ERROR_IO_ERROR;

	if (!response.success)
		return PICO_HUB_ERROR_REQUEST_FAILED;

	return PICO_HUB_ERROR_NONE;
}
int       pico_hub_gpio_deinit(pico_hub* hub, uint8_t pin)
{
	pico_hub_packet_request<PICO_HUB_OPCODE_GPIO_DEINIT> request =
	{
		.pin = pin
	};
	pico_hub_packet_response<PICO_HUB_OPCODE_GPIO_DEINIT> response;

	if (!pico_hub_io_send_request_receive_response(hub->io, request, response))
		return PICO_HUB_ERROR_IO_ERROR;

	return PICO_HUB_ERROR_NONE;
}
int       pico_hub_gpio_get_pull_up(pico_hub* hub, uint8_t pin, bool* value)
{
	pico_hub_packet_request<PICO_HUB_OPCODE_GPIO_GET_PULL_UP> request =
	{
		.pin = pin
	};
	pico_hub_packet_response<PICO_HUB_OPCODE_GPIO_GET_PULL_UP> response;

	if (!pico_hub_io_send_request_receive_response(hub->io, request, response))
		return PICO_HUB_ERROR_IO_ERROR;

	if (!response.success)
		return PICO_HUB_ERROR_REQUEST_FAILED;

	*value = response.value;

	return PICO_HUB_ERROR_NONE;
}
int       pico_hub_gpio_set_pull_up(pico_hub* hub, uint8_t pin, bool value)
{
	pico_hub_packet_request<PICO_HUB_OPCODE_GPIO_SET_PULL_UP> request =
	{
		.pin   = pin,
		.value = value
	};
	pico_hub_packet_response<PICO_HUB_OPCODE_GPIO_SET_PULL_UP> response;

	if (!pico_hub_io_send_request_receive_response(hub->io, request, response))
		return PICO_HUB_ERROR_IO_ERROR;

	if (!response.success)
		return PICO_HUB_ERROR_REQUEST_FAILED;

	return PICO_HUB_ERROR_NONE;
}
int       pico_hub_gpio_get_pull_down(pico_hub* hub, uint8_t pin, bool* value)
{
	pico_hub_packet_request<PICO_HUB_OPCODE_GPIO_GET_PULL_DOWN> request =
	{
		.pin = pin
	};
	pico_hub_packet_response<PICO_HUB_OPCODE_GPIO_GET_PULL_DOWN> response;

	if (!pico_hub_io_send_request_receive_response(hub->io, request, response))
		return PICO_HUB_ERROR_IO_ERROR;

	if (!response.success)
		return PICO_HUB_ERROR_REQUEST_FAILED;

	*value = response.value;

	return PICO_HUB_ERROR_NONE;
}
int       pico_hub_gpio_set_pull_down(pico_hub* hub, uint8_t pin, bool value)
{
	pico_hub_packet_request<PICO_HUB_OPCODE_GPIO_SET_PULL_DOWN> request =
	{
		.pin   = pin,
		.value = value
	};
	pico_hub_packet_response<PICO_HUB_OPCODE_GPIO_SET_PULL_DOWN> response;

	if (!pico_hub_io_send_request_receive_response(hub->io, request, response))
		return PICO_HUB_ERROR_IO_ERROR;

	if (!response.success)
		return PICO_HUB_ERROR_REQUEST_FAILED;

	return PICO_HUB_ERROR_NONE;
}
int       pico_hub_gpio_get_direction(pico_hub* hub, uint8_t pin, bool* value)
{
	pico_hub_packet_request<PICO_HUB_OPCODE_GPIO_GET_DIRECTION> request =
	{
		.pin = pin
	};
	pico_hub_packet_response<PICO_HUB_OPCODE_GPIO_GET_DIRECTION> response;

	if (!pico_hub_io_send_request_receive_response(hub->io, request, response))
		return PICO_HUB_ERROR_IO_ERROR;

	if (!response.success)
		return PICO_HUB_ERROR_REQUEST_FAILED;

	*value = response.value;

	return PICO_HUB_ERROR_NONE;
}
int       pico_hub_gpio_set_direction(pico_hub* hub, uint8_t pin, bool value)
{
	pico_hub_packet_request<PICO_HUB_OPCODE_GPIO_SET_DIRECTION> request =
	{
		.pin   = pin,
		.value = value
	};
	pico_hub_packet_response<PICO_HUB_OPCODE_GPIO_SET_DIRECTION> response;

	if (!pico_hub_io_send_request_receive_response(hub->io, request, response))
		return PICO_HUB_ERROR_IO_ERROR;

	if (!response.success)
		return PICO_HUB_ERROR_REQUEST_FAILED;

	return PICO_HUB_ERROR_NONE;
}
int       pico_hub_gpio_get_drive_strength(pico_hub* hub, uint8_t pin, int* value)
{
	pico_hub_packet_request<PICO_HUB_OPCODE_GPIO_GET_DRIVE_STRENGTH> request =
	{
		.pin = pin
	};
	pico_hub_packet_response<PICO_HUB_OPCODE_GPIO_GET_DRIVE_STRENGTH> response;

	if (!pico_hub_io_send_request_receive_response(hub->io, request, response))
		return PICO_HUB_ERROR_IO_ERROR;

	if (!response.success)
		return PICO_HUB_ERROR_REQUEST_FAILED;

	*value = response.value;

	return PICO_HUB_ERROR_NONE;
}
int       pico_hub_gpio_set_drive_strength(pico_hub* hub, uint8_t pin, int value)
{
	pico_hub_packet_request<PICO_HUB_OPCODE_GPIO_SET_DRIVE_STRENGTH> request =
	{
		.pin   = pin,
		.value = (uint8_t)value
	};
	pico_hub_packet_response<PICO_HUB_OPCODE_GPIO_SET_DRIVE_STRENGTH> response;

	if (!pico_hub_io_send_request_receive_response(hub->io, request, response))
		return PICO_HUB_ERROR_IO_ERROR;

	if (!response.success)
		return PICO_HUB_ERROR_REQUEST_FAILED;

	return PICO_HUB_ERROR_NONE;
}
int       pico_hub_gpio_read(pico_hub* hub, uint8_t pin, bool* value)
{
	pico_hub_packet_request<PICO_HUB_OPCODE_GPIO_READ> request =
	{
		.pin = pin
	};
	pico_hub_packet_response<PICO_HUB_OPCODE_GPIO_READ> response;

	if (!pico_hub_io_send_request_receive_response(hub->io, request, response))
		return PICO_HUB_ERROR_IO_ERROR;

	if (!response.success)
		return PICO_HUB_ERROR_REQUEST_FAILED;

	*value = response.value;

	return PICO_HUB_ERROR_NONE;
}
int       pico_hub_gpio_write(pico_hub* hub, uint8_t pin, bool value)
{
	pico_hub_packet_request<PICO_HUB_OPCODE_GPIO_WRITE> request =
	{
		.pin   = pin,
		.value = value
	};
	pico_hub_packet_response<PICO_HUB_OPCODE_GPIO_WRITE> response;

	if (!pico_hub_io_send_request_receive_response(hub->io, request, response))
		return PICO_HUB_ERROR_IO_ERROR;

	if (!response.success)
		return PICO_HUB_ERROR_REQUEST_FAILED;

	return PICO_HUB_ERROR_NONE;
}

int       pico_hub_uart_init(pico_hub* hub, PICO_HUB_UART bus, uint8_t rx, uint8_t tx, uint32_t baud)
{
	pico_hub_packet_request<PICO_HUB_OPCODE_UART_INIT> request =
	{
		.bus  = bus,
		.rx   = rx,
		.tx   = tx,
		.baud = baud
	};
	pico_hub_packet_response<PICO_HUB_OPCODE_UART_INIT> response;

	if (!pico_hub_io_send_request_receive_response(hub->io, request, response))
		return PICO_HUB_ERROR_IO_ERROR;

	if (!response.success)
		return PICO_HUB_ERROR_REQUEST_FAILED;

	return PICO_HUB_ERROR_NONE;
}
int       pico_hub_uart_deinit(pico_hub* hub, PICO_HUB_UART bus)
{
	pico_hub_packet_request<PICO_HUB_OPCODE_UART_DEINIT> request =
	{
		.bus = bus
	};
	pico_hub_packet_response<PICO_HUB_OPCODE_UART_DEINIT> response;

	if (!pico_hub_io_send_request_receive_response(hub->io, request, response))
		return PICO_HUB_ERROR_IO_ERROR;

	return PICO_HUB_ERROR_NONE;
}
int       pico_hub_uart_read(pico_hub* hub, PICO_HUB_UART bus, void* buffer, size_t size)
{
	pico_hub_packet_request<PICO_HUB_OPCODE_UART_READ> request =
	{
		.bus  = bus,
		.size = (uint32_t)size
	};
	pico_hub_packet_response<PICO_HUB_OPCODE_UART_READ> response;

	if (!pico_hub_io_send_request_receive_response(hub->io, request, response))
		return PICO_HUB_ERROR_IO_ERROR;

	if (!response.success)
		return PICO_HUB_ERROR_REQUEST_FAILED;

	if (!pico_hub_io_receive(hub->io, buffer, size))
		return PICO_HUB_ERROR_IO_ERROR;

	return PICO_HUB_ERROR_NONE;
}
int       pico_hub_uart_write(pico_hub* hub, PICO_HUB_UART bus, const void* buffer, size_t size)
{
	pico_hub_packet_request<PICO_HUB_OPCODE_UART_WRITE> request =
	{
		.bus  = bus,
		.size = (uint32_t)size
	};
	pico_hub_packet_response<PICO_HUB_OPCODE_UART_WRITE> response;

	if (!pico_hub_io_send_request(hub->io, request))
		return PICO_HUB_ERROR_IO_ERROR;

	if (!pico_hub_io_send(hub->io, buffer, size))
		return PICO_HUB_ERROR_IO_ERROR;

	if (!pico_hub_io_receive_response(hub->io, response))
		return PICO_HUB_ERROR_IO_ERROR;

	if (!response.success)
		return PICO_HUB_ERROR_REQUEST_FAILED;

	return PICO_HUB_ERROR_NONE;
}
