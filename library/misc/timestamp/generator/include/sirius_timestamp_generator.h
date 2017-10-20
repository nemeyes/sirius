#ifndef _SIRIUS_TIMESTAMP_GENERATOR_H_
#define _SIRIUS_TIMESTAMP_GENERATOR_H_

#include <sirius.h>
#if defined(EXPORT_TIMESTAMP_GENERATOR_LIB)
#define EXP_TIMESTAMP_GENERATOR_CLASS __declspec(dllexport)
#else
#define EXP_TIMESTAMP_GENERATOR_CLASS __declspec(dllimport)
#endif

namespace sirius
{
	namespace library
	{
		namespace misc
		{
			namespace timestamp
			{
				class EXP_TIMESTAMP_GENERATOR_CLASS generator
				{
				public:
					class core;
				public:
					static sirius::library::misc::timestamp::generator & instance(void);

					void		begin_elapsed_time(void);
					long long	elapsed_microseconds(void);
					long long	elapsed_milliseconds(void);

				private:
					generator(void);
					generator(const sirius::library::misc::timestamp::generator & clone);
					~generator(void);

				private:
					sirius::library::misc::timestamp::generator::core * _generator;

				};
			};
		};
	};
};

#endif
