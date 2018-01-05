#ifndef _CLUSTER_TYPE_H_
#define _CLUSTER_TYPE_H_

#include <stdint.h>

namespace sirius
{
	namespace library
	{
		namespace net
		{
			namespace backend
			{
				class type
				{
				public:
					typedef struct _msg_t {
						char*		data;
						int			length;
					}msg_t;
				};
			};
		};
	};
};

#endif //_CLUSTER_TYPE_H_