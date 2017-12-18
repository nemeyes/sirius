#ifndef _ATTENANT_ENTITY_H_
#define _ATTENANT_ENTITY_H_

#include <sirius.h>

namespace sirius
{
	namespace app
	{
		namespace server
		{
			namespace arbitrator
			{
				namespace entity
				{
					typedef struct _attendant_t
					{
						char		uuid[64];
						int32_t		id;
						char		client_uuid[64];
						char		client_id[MAX_PATH];
						int32_t		pid;
						int32_t		state;
						uint64_t	total_bandwidth_bytes;
						_attendant_t(void)
							: id(0)
							, pid(0)
							, total_bandwidth_bytes(0)
						{
							memset(uuid, 0x00, sizeof(uuid));
							memset(client_id, 0x00, sizeof(client_id));
							memset(client_uuid, 0x00, sizeof(client_uuid));
						}
					} attendant_t;
				};
			};
		};
	};
};

#endif