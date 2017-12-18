
#include "data_access_object.h"
#include <stdlib.h>
#include <stdio.h>

#include <errno.h>
#include <string>

sirius::app::server::arbitrator::db::data_access_object::data_access_object(const char * path)
{
	sqlite3_enable_shared_cache( true );
	if( sqlite3_open(path, &_db)!=SQLITE_OK )
	{
		printf( "db connection error\n" );
	}
	else
	{
		printf( "db is opened\n" );
	}
}

sirius::app::server::arbitrator::db::data_access_object::~data_access_object( void )
{
	sqlite3_close( _db );
	//printf( "db is closed\n" );
}

void sirius::app::server::arbitrator::db::data_access_object::begin( void )
{
	sqlite3_exec( _db, "BEGIN TRANSACTION;", 0, 0, 0 );
}

void sirius::app::server::arbitrator::db::data_access_object::commit( void )
{
	sqlite3_exec( _db, "COMMIT TRANSACTION;", 0, 0, 0 );
}

void sirius::app::server::arbitrator::db::data_access_object::rollback( void )
{
	sqlite3_exec( _db, "ROLLBACK TRANSACTION;", 0, 0, 0 );
}

void sirius::app::server::arbitrator::db::data_access_object::begin( sqlite3 *connection )
{
	sqlite3_exec( connection, "BEGIN TRANSACTION;", 0, 0, 0 );
}

void sirius::app::server::arbitrator::db::data_access_object::commit( sqlite3 *connection )
{
	sqlite3_exec( connection, "COMMIT TRANSACTION;", 0, 0, 0 );
}

void sirius::app::server::arbitrator::db::data_access_object::rollback( sqlite3 *connection )
{
	sqlite3_exec( connection, "ROLLBACK TRANSACTION;", 0, 0, 0 );
}

sqlite3* sirius::app::server::arbitrator::db::data_access_object::get_connection(void)
{
	return _db;
}

/*
int sirius::app::server::arbitrator::data_access_object::is_tbl_exist(const char *tbl)
{
	char cmd[256];	
	sqlite3_stmt *stmt;
	int ret = 0;

	snprintf(cmd, sizeof(cmd), "SELECT name FROM sqlite_master WHERE type='table' AND name='%s'", tbl);
	sqlite3_prepare(_db, cmd, -1, &stmt, NULL);
	ret = sqlite3_step(stmt);
	if (ret == SQLITE_ROW)
		ret = 1;
	else
		ret = 0;
	
 	sqlite3_finalize(stmt);
	return ret;	
}

int sirius::app::server::arbitrator::data_access_object::is_col_exist(const char *col, const char *tbl)
{
	char cmd[256];	
	sqlite3_stmt *stmt;
	int ret = 0;

	snprintf(cmd, sizeof(cmd), "select %s from %s", col, tbl);
	if (sqlite3_prepare(_db, cmd, -1, &stmt, NULL) == SQLITE_OK)
		ret = 1;

	sqlite3_finalize(stmt);
	return ret;
}

int sirius::app::server::arbitrator::data_access_object::insert_col(const char *col, const char *tbl, int type, void *val)
{
	char cmd[256];
	sqlite3_stmt *stmt;
	int ret;

	if (type == SQLITE_INTEGER)
	{
		snprintf(cmd, sizeof(cmd), "alter table %s add column %s INTEGER DEFAULT %d", tbl, col, *(int *)val);
	}
	else if (type == SQLITE_TEXT)
	{
		snprintf(cmd, sizeof(cmd), "alter table %s add column %s VARCHAR DEFAULT \"%s\"", tbl, col, (char *)val);
	}
	else
	{
		velog("Not yet supported type: %d", type);
		return -1;
	}

	ret = sqlite3_prepare(_db, cmd, -1, &stmt, NULL);
	if (ret != SQLITE_OK)
	{
		velog("Can't compile command: %s reason: %d", cmd, ret);
		return -1;
	}

	ret = sqlite3_step(stmt);
	if (ret != SQLITE_DONE)
		velog("Can't excute command: %s reason: %d", cmd, ret);

	sqlite3_finalize(stmt);
	vilog("New column: %s at %s (%d)", col, tbl, ret)
	return ret == SQLITE_DONE ? 0 : -1;
}
void elastics::data_access_object::export_table(const char *tbl)
{
	int ret;
	char cmd[256];

	snprintf(cmd, sizeof(cmd), "DELETE FROM EXP_DB.%s;", tbl);
	ret = sqlite3_exec(_db, cmd, 0, 0, 0);
	if (ret != SQLITE_OK)
		vdlog("%s failed with %d", cmd, ret);

	snprintf(cmd, sizeof(cmd), "INSERT INTO EXP_DB.%s SELECT * FROM %s;", tbl, tbl);
	ret = sqlite3_exec(_db, cmd, 0, 0, 0);
	if (ret != SQLITE_OK)
		vdlog("%s failed with %d", cmd, ret);
}

#define MAX_TBLS 20
const char tbls[MAX_TBLS][256] =
{
	"tb_media_source",
	"tb_media_profile",
	"tb_media_url",
	"tb_ntp",
	"tb_stream_setup",
	"tb_system_status",
	"tb_transcoder_combination",
	"tb_users",
#ifdef SUPPORT_RECORDER
	"tb_media_record",
	"tb_media_record_device",
	"tb_media_record_schedule",
	"tb_special_day",
#endif
	"__end__"
};
#include <string.h>
void elastics::data_access_object::export_db( void )
{
	int i;
	
	if (SQLITE_OK != sqlite3_exec( _db, "ATTACH DATABASE 'db/export/ONVIF.sqlite' as 'EXP_DB';", 0, 0, 0))
	{
		velog("DB export fail: can't attach DB");
		return;
	}

	//tb_special_day is newbie. it's possible that export db doesn't contain this table //
#ifdef SUPPORT_RECORDER
	std::string sql;
	sql = "CREATE TABLE EXP_DB.tb_special_day (pk INTEGER PRIMARY KEY AUTOINCREMENT,";
	sql += " gr_id VARCHAR NOT NULL DEFAULT (''),";
	sql += " year INTEGER DEFAULT (0) NOT NULL,";
	sql += " month INTEGER DEFAULT (0) NOT NULL,";
	sql += " day INTEGER NOT NULL DEFAULT (0),";
	sql += " hour INTEGER NOT NULL DEFAULT (0));";
	
	sqlite3_exec(_db, sql.c_str(), 0, 0, 0);
#endif
	for (i=0; i<MAX_TBLS; i++)
	{
		if (0 == strcmp(tbls[i], "__end__"))
			break;

		export_table(tbls[i]);
	}

	if (SQLITE_OK != sqlite3_exec( _db, "DETACH DATABASE 'EXP_DB';", 0, 0, 0))
		velog("DB export fail: can't detach DB");
}

void elastics::data_access_object::default_db( void )
{
	int db_status = sqlite3_exec( _db, "ATTACH DATABASE 'db/default/ONVIF.sqlite' as 'DEF_DB';", 0, 0, 0);
	//printf( "db_status for attach %d \n", db_status );

	//copy media source
	db_status = sqlite3_exec( _db, "DELETE FROM tb_media_source;", 0, 0, 0);
	//printf( "db_status for delete %d \n", db_status );
	db_status = sqlite3_exec( _db, "INSERT INTO tb_media_source SELECT * FROM DEF_DB.tb_media_source;", 0, 0, 0);
	//printf( "db_status for insert %d \n", db_status );

	//copy media profile
	db_status = sqlite3_exec( _db, "DELETE FROM tb_media_profile;", 0, 0, 0);
	//printf( "db_status for delete %d \n", db_status );
	db_status = sqlite3_exec( _db, "INSERT INTO tb_media_profile SELECT * FROM DEF_DB.tb_media_profile;", 0, 0, 0);
	//printf( "db_status for insert %d \n", db_status );

	//copy media url
	db_status = sqlite3_exec( _db, "DELETE FROM tb_media_url;", 0, 0, 0);
	//printf( "db_status for delete %d \n", db_status );
	db_status = sqlite3_exec( _db, "INSERT INTO tb_media_url SELECT * FROM DEF_DB.tb_media_url;", 0, 0, 0);
	//printf( "db_status for insert %d \n", db_status );

	//copy ntp
	db_status = sqlite3_exec( _db, "DELETE FROM tb_ntp;", 0, 0, 0);
	//printf( "db_status for delete %d \n", db_status );
	db_status = sqlite3_exec( _db, "INSERT INTO tb_ntp SELECT * FROM DEF_DB.tb_ntp;", 0, 0, 0);
	//printf( "db_status for insert %d \n", db_status );

	//copy stream setup
	db_status = sqlite3_exec( _db, "DELETE FROM tb_stream_setup;", 0, 0, 0);
	//printf( "db_status for delete %d \n", db_status );
	db_status = sqlite3_exec( _db, "INSERT INTO tb_stream_setup SELECT * FROM DEF_DB.tb_stream_setup;", 0, 0, 0);
	//printf( "db_status for insert %d \n", db_status );

	//copy system status
	db_status = sqlite3_exec( _db, "DELETE FROM tb_system_status;", 0, 0, 0);
	//printf( "db_status for delete %d \n", db_status );
	db_status = sqlite3_exec( _db, "INSERT INTO tb_system_status SELECT * FROM DEF_DB.tb_system_status;", 0, 0, 0);
	//printf( "db_status for insert %d \n", db_status );

	//copy transcoder combination
	db_status = sqlite3_exec( _db, "DELETE FROM tb_transcoder_combination;", 0, 0, 0);
	//printf( "db_status for delete %d \n", db_status );
	db_status = sqlite3_exec( _db, "INSERT INTO tb_transcoder_combination SELECT * FROM DEF_DB.tb_transcoder_combination;", 0, 0, 0);
	//printf( "db_status for insert %d \n", db_status );

	//copy users
	db_status = sqlite3_exec( _db, "DELETE FROM tb_users;", 0, 0, 0);
	//printf( "db_status for delete %d \n", db_status );
	db_status = sqlite3_exec( _db, "INSERT INTO tb_users SELECT * FROM DEF_DB.tb_users;", 0, 0, 0);
	//printf( "db_status for insert %d \n", db_status );

#if defined(LINUX) && defined(WORK_AS_DISTRIBUTOR) && defined(SUPPORT_RECORDER)
	//copy record group info
	db_status = sqlite3_exec( _db, "DELETE FROM tb_media_record;", 0, 0, 0);
	db_status = sqlite3_exec( _db, "INSERT INTO tb_media_record SELECT * FROM DEF_DB.tb_media_record;", 0, 0, 0);
	//opcy record device info
	db_status = sqlite3_exec( _db, "DELETE FROM tb_media_record_device;", 0, 0, 0);
	db_status = sqlite3_exec( _db, "INSERT INTO tb_media_record_device SELECT * FROM DEF_DB.tb_media_record_device;", 0, 0, 0);
	//opcy record device info
	db_status = sqlite3_exec( _db, "DELETE FROM tb_media_record_schedule;", 0, 0, 0);
	db_status = sqlite3_exec( _db, "INSERT INTO tb_media_record_schedule SELECT * FROM DEF_DB.tb_media_record_schedule;", 0, 0, 0);

	db_status = sqlite3_exec( _db, "DELETE FROM tb_special_day;", 0, 0, 0);
	db_status = sqlite3_exec( _db, "INSERT INTO tb_special_day SELECT * FROM DEF_DB.tb_special_day;", 0, 0, 0);
#endif

	db_status = sqlite3_exec( _db, "DETACH DATABASE 'DEF_DB';", 0, 0, 0);
}

void elastics::data_access_object::import_db( void )
{
	int db_status = sqlite3_exec( _db, "ATTACH DATABASE 'db/import/ONVIF.sqlite' as 'IMP_DB';", 0, 0, 0);
	//printf( "db_status for attach %d \n", db_status );

	//copy media source
	db_status = sqlite3_exec( _db, "DELETE FROM tb_media_source;", 0, 0, 0);
	//printf( "db_status for delete %d \n", db_status );
	db_status = sqlite3_exec( _db, "INSERT INTO tb_media_source SELECT * FROM IMP_DB.tb_media_source;", 0, 0, 0);
	//printf( "db_status for insert %d \n", db_status );

	//copy media profile
	db_status = sqlite3_exec( _db, "DELETE FROM tb_media_profile;", 0, 0, 0);
	//printf( "db_status for delete %d \n", db_status );
	db_status = sqlite3_exec( _db, "INSERT INTO tb_media_profile SELECT * FROM IMP_DB.tb_media_profile;", 0, 0, 0);
	//printf( "db_status for insert %d \n", db_status );

	//copy media url
	db_status = sqlite3_exec( _db, "DELETE FROM tb_media_url;", 0, 0, 0);
	//printf( "db_status for delete %d \n", db_status );
	db_status = sqlite3_exec( _db, "INSERT INTO tb_media_url SELECT * FROM IMP_DB.tb_media_url;", 0, 0, 0);
	//printf( "db_status for insert %d \n", db_status );

	//copy ntp
	db_status = sqlite3_exec( _db, "DELETE FROM tb_ntp;", 0, 0, 0);
	//printf( "db_status for delete %d \n", db_status );
	db_status = sqlite3_exec( _db, "INSERT INTO tb_ntp SELECT * FROM IMP_DB.tb_ntp;", 0, 0, 0);
	//printf( "db_status for insert %d \n", db_status );

	//copy stream setup
	db_status = sqlite3_exec( _db, "DELETE FROM tb_stream_setup;", 0, 0, 0);
	//printf( "db_status for delete %d \n", db_status );
	db_status = sqlite3_exec( _db, "INSERT INTO tb_stream_setup SELECT * FROM IMP_DB.tb_stream_setup;", 0, 0, 0);
	//printf( "db_status for insert %d \n", db_status );

	//copy system status
	db_status = sqlite3_exec( _db, "DELETE FROM tb_system_status;", 0, 0, 0);
	//printf( "db_status for delete %d \n", db_status );
	db_status = sqlite3_exec( _db, "INSERT INTO tb_system_status SELECT * FROM IMP_DB.tb_system_status;", 0, 0, 0);
	//printf( "db_status for insert %d \n", db_status );

	//copy transcoder combination
	db_status = sqlite3_exec( _db, "DELETE FROM tb_transcoder_combination;", 0, 0, 0);
	//printf( "db_status for delete %d \n", db_status );
	db_status = sqlite3_exec( _db, "INSERT INTO tb_transcoder_combination SELECT * FROM IMP_DB.tb_transcoder_combination;", 0, 0, 0);
	//printf( "db_status for insert %d \n", db_status );

	//copy users
	db_status = sqlite3_exec( _db, "DELETE FROM tb_users;", 0, 0, 0);
	//printf( "db_status for delete %d \n", db_status );
	db_status = sqlite3_exec( _db, "INSERT INTO tb_users SELECT * FROM IMP_DB.tb_users;", 0, 0, 0);
	//printf( "db_status for insert %d \n", db_status );

#if defined(LINUX) && defined(WORK_AS_DISTRIBUTOR) && defined(SUPPORT_RECORDER)
	//copy record group info
	db_status = sqlite3_exec( _db, "DELETE FROM tb_media_record;", 0, 0, 0);
	db_status = sqlite3_exec( _db, "INSERT INTO tb_media_record SELECT * FROM IMP_DB.tb_media_record;", 0, 0, 0);
	//opcy record device info
	db_status = sqlite3_exec( _db, "DELETE FROM tb_media_record_device;", 0, 0, 0);
	db_status = sqlite3_exec( _db, "INSERT INTO tb_media_record_device SELECT * FROM IMP_DB.tb_media_record_device;", 0, 0, 0);
	//opcy record device info
	db_status = sqlite3_exec( _db, "DELETE FROM tb_media_record_schedule;", 0, 0, 0);
	db_status = sqlite3_exec( _db, "INSERT INTO tb_media_record_schedule SELECT * FROM IMP_DB.tb_media_record_schedule;", 0, 0, 0);
	
	db_status = sqlite3_exec( _db, "DELETE FROM tb_special_day;", 0, 0, 0);
	db_status = sqlite3_exec( _db, "INSERT INTO tb_special_day SELECT * FROM IMP_DB.tb_special_day;", 0, 0, 0);
#endif

	db_status = sqlite3_exec( _db, "DETACH DATABASE 'IMP_DB';", 0, 0, 0);
}

void elastics::data_access_object::update_cluster_uuid( const char * uuid )
{
	//update tb_media_source's cluster uuid
	std::string sql = " UPDATE tb_media_source SET system_id='";
	sql				+= uuid;
	sql				+= "' WHERE ";
	sql				+= " system_id!='intellivms-distributor-pro-subsystem1' and ";
	sql				+= " system_id!='intellivms-distributor-pro-subsystem2' and ";
	sql				+= " system_id!='intellivms-distributor-pro-subsystem3' and ";
	sql				+= " system_id!='intellivms-distributor-pro-subsystem4' and ";
	sql				+= " system_id!='intellivms-distributor-pro-subsystem5' and ";
	sql				+= " system_id!='intellivms-distributor-pro-subsystem6' and ";
	sql				+= " system_id!='intellivms-distributor-pro-subsystem7' and ";
	sql				+= " system_id!='intellivms-distributor-pro-subsystem8';";
	sqlite3_exec( _db, sql.c_str(), 0, 0, 0);

	//update tb_ntp's cluster uuid
	sql = " UPDATE tb_ntp SET system_id='";
	sql	+= uuid;
	sql	+= "' WHERE ";
	sql	+= " system_id!='intellivms-distributor-pro-subsystem1' and ";
	sql	+= " system_id!='intellivms-distributor-pro-subsystem2' and ";
	sql	+= " system_id!='intellivms-distributor-pro-subsystem3' and ";
	sql	+= " system_id!='intellivms-distributor-pro-subsystem4' and ";
	sql	+= " system_id!='intellivms-distributor-pro-subsystem5' and ";
	sql	+= " system_id!='intellivms-distributor-pro-subsystem6' and ";
	sql	+= " system_id!='intellivms-distributor-pro-subsystem7' and ";
	sql	+= " system_id!='intellivms-distributor-pro-subsystem8';";
	sqlite3_exec( _db, sql.c_str(), 0, 0, 0);

	//update tb_stream_setup's cluster uuid
	sql = " UPDATE tb_stream_setup SET system_id='";
	sql	+= uuid;
	sql	+= "' WHERE ";
	sql	+= " system_id!='intellivms-distributor-pro-subsystem1' and ";
	sql	+= " system_id!='intellivms-distributor-pro-subsystem2' and ";
	sql	+= " system_id!='intellivms-distributor-pro-subsystem3' and ";
	sql	+= " system_id!='intellivms-distributor-pro-subsystem4' and ";
	sql	+= " system_id!='intellivms-distributor-pro-subsystem5' and ";
	sql	+= " system_id!='intellivms-distributor-pro-subsystem6' and ";
	sql	+= " system_id!='intellivms-distributor-pro-subsystem7' and ";
	sql	+= " system_id!='intellivms-distributor-pro-subsystem8';";
	sqlite3_exec( _db, sql.c_str(), 0, 0, 0);

	//update tb_transcoder_combination's cluster uuid
	sql = " UPDATE tb_transcoder_combination SET system_id='";
	sql	+= uuid;
	sql	+= "' WHERE ";
	sql	+= " system_id!='intellivms-distributor-pro-subsystem1' and ";
	sql	+= " system_id!='intellivms-distributor-pro-subsystem2' and ";
	sql	+= " system_id!='intellivms-distributor-pro-subsystem3' and ";
	sql	+= " system_id!='intellivms-distributor-pro-subsystem4' and ";
	sql	+= " system_id!='intellivms-distributor-pro-subsystem5' and ";
	sql	+= " system_id!='intellivms-distributor-pro-subsystem6' and ";
	sql	+= " system_id!='intellivms-distributor-pro-subsystem7' and ";
	sql	+= " system_id!='intellivms-distributor-pro-subsystem8';";
	sqlite3_exec( _db, sql.c_str(), 0, 0, 0);

}
*/