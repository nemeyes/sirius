#ifndef _DATA_ACCESS_OBJECT_H_
#define _DATA_ACCESS_OBJECT_H_

#include "sqlite3.h"
#include <sirius.h>

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
					class data_access_object
						: public sirius::base
					{
					public:
						data_access_object(const char * path);
						virtual ~data_access_object(void);
						void		begin(void);
						void		commit(void);
						void		rollback(void);

						void		begin(sqlite3 *connection);
						void		commit(sqlite3 *connection);
						void		rollback(sqlite3 *connection);

						sqlite3*	get_connection(void);

						/*
						void		export_db(void);
						void 		default_db(void);
						void		import_db(void);
						void		update_cluster_uuid(const char * uuid);
						int32_t		is_col_exist(const char * col, const char * tbl);
						int32_t		insert_col(const char * col, const char * tbl, int type, void * val);
						int32_t		is_tbl_exist(const char * tbl);
						void		export_table(const char * tbl);
						*/
					protected:
						sqlite3 *_db;
					};
				};
			};
		};
	};
};

#endif