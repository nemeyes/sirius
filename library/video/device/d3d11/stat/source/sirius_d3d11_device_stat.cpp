#include "sirius_d3d11_device_stat.h"
#include "d3d11_device_stat.h"

void sirius::library::video::device::d3d11::stat::retreieve(sirius::library::video::device::d3d11::stat::desc_t * adapters, int32_t capacity, int32_t & count, int32_t option)
{
	sirius::library::video::device::d3d11::stat::core::retreieve(adapters, capacity, count, option);
}