#ifndef _SIRIUS_RECEIVER_H_
#define _SIRIUS_RECEIVER_H_

#include "sirius_unified_client.h"

namespace sirius
{
	namespace library
	{
		namespace unified
		{
			class receiver
			{
			public:
				receiver(sirius::library::unified::client * front);
				virtual ~receiver(void);

			protected:
				sirius::library::unified::client *	_front;
			};
		};
	};
};

#endif