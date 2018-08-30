#ifndef _SIRIUS_VIDEO_H_
#define _SIRIUS_VIDEO_H_

#include <sirius.h>

#if defined(EXPORT_VIDEO_LIB)
#define EXP_VIDEO_CLASS __declspec(dllexport)
#else
#define EXP_VIDEO_CLASS __declspec(dllimport)
#endif

namespace sirius
{
	namespace library
	{
		namespace video
		{
			class EXP_VIDEO_CLASS base
				: public sirius::base
			{
			public:
				static const int32_t MAX_VIDEO_SIZE = 1024 * 1024 * 2;
				typedef struct _entity_t
				{
					long long	timestamp;
					void *		data;
					int32_t		data_size;
					int32_t		data_capacity;
					int16_t		x;
					int16_t		y;
					int16_t		width;
					int16_t		height;
					int16_t		stride;
					_entity_t(void)
						: timestamp(0)
						, data(nullptr)
						, data_size(0)
						, data_capacity(0)
						, x(0)
						, y(0)
						, width(0)
						, height(0)
						, stride(0)
					{}

					_entity_t(const _entity_t & clone)
					{
						timestamp = clone.timestamp;
						data = clone.data;
						data_size = clone.data_size;
						data_capacity = clone.data_capacity;
						x = clone.x;
						y = clone.y;
						width = clone.width;
						height = clone.height;
						stride = clone.stride;
					}

					_entity_t operator=(const _entity_t & clone)
					{
						timestamp = clone.timestamp;
						data = clone.data;
						data_size = clone.data_size;
						data_capacity = clone.data_capacity;
						x = clone.x;
						y = clone.y;
						width = clone.width;
						height = clone.height;
						stride = clone.stride;
						return (*this);
					}

					~_entity_t(void)
					{}

				} entity_t;

				base(void);
				virtual ~base(void);
			};
		};
	};
};







#endif