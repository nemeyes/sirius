#ifndef _SIRIUS_D3D11_DEVICE_STAT_H_
#define _SIRIUS_D3D11_DEVICE_STAT_H_

#if defined(EXPORT_D3D11_DEVICE_STAT_LIB)
#define EXP_D3D11_DEVICE_STAT_CLASS __declspec(dllexport)
#else
#define EXP_D3D11_DEVICE_STAT_CLASS __declspec(dllimport)
#endif

#include <sirius.h>

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
					class EXP_D3D11_DEVICE_STAT_CLASS stat
					{
					public:
						class core;
					public:
						typedef struct _option_t
						{
							static const int32_t hw = 0;
							static const int32_t sw = 1;
							static const int32_t both = 2;
						} option_t;

						typedef struct _desc_t
						{
							char		description[128];
							int32_t		adaptorIndex;
							uint32_t	vendorId;
							uint32_t	deviceId;
							uint32_t	subsysId;
							uint32_t	revision;
							int32_t		coordLeft;
							int32_t		coordTop;
							int32_t		coordRight;
							int32_t		coordBottom;
							char		luid[64];
							_desc_t(void)
								: adaptorIndex(-1)
								, vendorId(0)
								, deviceId(0)
								, subsysId(0)
								, revision(0)
								, coordLeft(0)
								, coordTop(0)
								, coordRight(0)
								, coordBottom(0)
							{
								memset(description, 0x00, sizeof(description));
								memset(luid, 0x00, sizeof(luid));
							}

							_desc_t(const _desc_t & clone)
							{
								strncpy_s(description, clone.description, sizeof(description));
								adaptorIndex = clone.adaptorIndex;
								vendorId = clone.vendorId;
								deviceId = clone.deviceId;
								subsysId = clone.subsysId;
								revision = clone.revision;
								strncpy_s(luid, clone.luid, sizeof(luid));
								coordLeft = clone.coordLeft;
								coordTop = clone.coordTop;
								coordRight = clone.coordRight;
								coordBottom = clone.coordBottom;
							}

							_desc_t operator=(const _desc_t & clone)
							{
								strncpy_s(description, clone.description, sizeof(description));
								adaptorIndex = clone.adaptorIndex;
								vendorId = clone.vendorId;
								deviceId = clone.deviceId;
								subsysId = clone.subsysId;
								revision = clone.revision;
								strncpy_s(luid, clone.luid, sizeof(luid));
								coordLeft = clone.coordLeft;
								coordTop = clone.coordTop;
								coordRight = clone.coordRight;
								coordBottom = clone.coordBottom;
								return (*this);
							}
						} desc_t;

						static void retreieve(sirius::library::video::device::d3d11::stat::desc_t * adapters, int32_t capacity, int32_t & count, int32_t option);
					};
				};
			};
		};
	};
};


#endif
