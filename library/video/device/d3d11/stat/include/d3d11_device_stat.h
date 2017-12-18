#ifndef _D3D11_DEVICE_STAT_H_
#define _D3D11_DEVICE_STAT_H_

#include "sirius_d3d11_device_stat.h"
#include <dxgi1_3.h>
#include <d3d11.h>
#include <dxgi1_2.h>
#include <rpcdce.h>

namespace sirius
{
	namespace library
	{
		namespace video
		{
			namespace device
			{
				namespace d3d11
				{
					class stat::core
					{
					public:
						static void retreieve(sirius::library::video::device::d3d11::stat::desc_t * adapters, int32_t capacity, int32_t & count, int32_t option);
					};
				};
			};
		};
	};
};








#endif
