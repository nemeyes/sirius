#ifndef _ATTENDANT_DAO_H_
#define _ATTENDANT_DAO_H_

#include "data_access_object.h"
#include "attendant_entity.h"

namespace sirius
{
	namespace app
	{
		namespace server
		{
			namespace arbitrator
			{
				namespace db
				{
					class attendant_dao
						: public sirius::app::server::arbitrator::db::data_access_object
					{
					public:
						typedef struct _type_t
						{
							static const int32_t attendant = 0;
							static const int32_t client = 1;
						} type_t;

						attendant_dao(const char * path);
						virtual ~attendant_dao(void);

						int32_t add(sirius::app::server::arbitrator::entity::attendant_t * entity, sqlite3 * connection = nullptr);
						int32_t remove(sirius::app::server::arbitrator::entity::attendant_t * entity, sqlite3 * connection = nullptr);
						int32_t remove(sqlite3 * connection = nullptr);
						int32_t update(sirius::app::server::arbitrator::entity::attendant_t * entity, sqlite3 * connection = nullptr);
						int32_t update(int32_t state, int32_t type, const char * uuid, sqlite3 * connection = nullptr);
						

						int32_t retrieve(sirius::app::server::arbitrator::entity::attendant_t * entity, sqlite3 * connection = nullptr);
						int32_t retrieve(sirius::app::server::arbitrator::entity::attendant_t *** entity, int32_t & count, sqlite3 * connection = nullptr);
						int32_t retrieve(int32_t state, sirius::app::server::arbitrator::entity::attendant_t *** entity, int32_t & count, sqlite3 * connection = nullptr);
						int32_t retrieve(const char * client_uuid, sirius::app::server::arbitrator::entity::attendant_t * entity, sqlite3 * connection = nullptr);

					private:
						int32_t retrieve_count(sqlite3 * connection = nullptr);
						int32_t retrieve_count(int32_t state, sqlite3 * connection = nullptr);
					};
				};
			};
		};
	};
};















#endif
