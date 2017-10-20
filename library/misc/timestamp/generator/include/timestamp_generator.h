#ifndef _TIMESTAMP_GENERATOR_H_
#define _TIMESTAMP_GENERATOR_H_

#include "sirius_timestamp_generator.h"
#include <windows.h>

namespace sirius
{
	namespace library
	{
		namespace misc
		{
			namespace timestamp
			{
				class generator::core
				{
				public:
					core(void);
					virtual ~core(void);

					void		begin_elapsed_time(void);
					long long	elapsed_microseconds(void);
					long long	elapsed_milliseconds(void);

				private:
					LARGE_INTEGER	_frequency;
					LARGE_INTEGER	_begin_elapsed_microseconds;
					bool			_begin;
				};
			};
		};
	};
};

#endif
