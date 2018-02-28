#include "configuration_dao.h"
#include <string>

sirius::app::server::arbitrator::db::configuration_dao::configuration_dao(const char * path)
	: sirius::app::server::arbitrator::db::data_access_object(path)
{

}

sirius::app::server::arbitrator::db::configuration_dao::~configuration_dao(void)
{

}

int32_t sirius::app::server::arbitrator::db::configuration_dao::update(sirius::app::server::arbitrator::entity::configuration_t * entity, sqlite3 * connection)
{
	int32_t status = sirius::app::server::arbitrator::db::configuration_dao::err_code_t::fail;
	if (!entity)
		return status;

	sqlite3_stmt * stmt;
	sqlite3 * conn;

	if (connection == 0)
		conn = _db;
	else
		conn = connection;

	std::string sql = "UPDATE tb_configuration SET ";
	sql += "uuid=?, url=?, max_attendant_instance=?, attendant_creation_delay=?, controller_portnumber=?, streamer_portnumber=?, ";
	sql += "video_codec=?, video_width=?, video_height=?, video_fps=?, ";
	sql += "video_block_width=?, video_block_height=?, video_compression_level=?, video_quantization_colors=?, ";
	sql += "enable_tls=?, enable_keepalive=?, enable_present=?, enable_auto_start=?, enable_caching=?, ";
	sql += "app_session_app=?";

	if (sqlite3_prepare(conn, sql.c_str(), -1, &stmt, 0) == SQLITE_OK)
	{
		int32_t index = 0;
		sqlite3_bind_text(stmt, ++index, entity->uuid, -1, 0);
		sqlite3_bind_text(stmt, ++index, entity->url, -1, 0);
		sqlite3_bind_int(stmt, ++index, entity->max_attendant_instance);
		sqlite3_bind_int(stmt, ++index, entity->attendant_creation_delay);
		sqlite3_bind_int(stmt, ++index, entity->controller_portnumber);
		sqlite3_bind_int(stmt, ++index, entity->streamer_portnumber);
		sqlite3_bind_int(stmt, ++index, entity->video_codec);
		sqlite3_bind_int(stmt, ++index, entity->video_width);
		sqlite3_bind_int(stmt, ++index, entity->video_height);
		sqlite3_bind_int(stmt, ++index, entity->video_fps);
		sqlite3_bind_int(stmt, ++index, entity->video_block_width);
		sqlite3_bind_int(stmt, ++index, entity->video_block_height);
		sqlite3_bind_int(stmt, ++index, entity->video_compression_level);
		sqlite3_bind_int(stmt, ++index, entity->video_quantization_colors);
		sqlite3_bind_int(stmt, ++index, entity->enable_tls?1:0);
		sqlite3_bind_int(stmt, ++index, entity->enable_keepalive ?1:0);
		sqlite3_bind_int(stmt, ++index, entity->enable_present?1:0);
		sqlite3_bind_int(stmt, ++index, entity->enable_auto_start ? 1 : 0);
		sqlite3_bind_int(stmt, ++index, entity->enable_caching ? 1 : 0);
		sqlite3_bind_text(stmt, ++index, entity->app_session_app, -1, 0);

		int32_t result = sqlite3_step(stmt);
		if (result == SQLITE_DONE)
		{
			status = sirius::app::server::arbitrator::db::configuration_dao::err_code_t::success;
		}
	}
	sqlite3_reset(stmt);
	sqlite3_finalize(stmt);
	return status;
}

int32_t sirius::app::server::arbitrator::db::configuration_dao::retrieve(sirius::app::server::arbitrator::entity::configuration_t * entity, sqlite3 * connection)
{
	int32_t status = sirius::app::server::arbitrator::db::configuration_dao::err_code_t::fail;
	if (!entity)
		return status;

	sqlite3_stmt * stmt;
	sqlite3 * conn;
	
	if (connection == 0)
		conn = _db;
	else
		conn = connection;

	int32_t count = retrieve_count(conn);

	if (count < 0)
		return status;

	if (count == 0)
	{
		sirius::uuid uuidgen;
		uuidgen.create();

		sirius::app::server::arbitrator::entity::configuration_t c_entity;
		strncpy_s(c_entity.uuid, uuidgen.c_str(), strlen(uuidgen.c_str()) + 1);
		strncpy_s(c_entity.url, "about:blank", sizeof(c_entity.url));
		c_entity.max_attendant_instance = 1;
		c_entity.attendant_creation_delay = 1000;
		c_entity.controller_portnumber = 5000;
		c_entity.streamer_portnumber = 7000;
		c_entity.video_codec = sirius::app::server::arbitrator::db::configuration_dao::video_submedia_type_t::png;
		c_entity.video_width = 1280;
		c_entity.video_height = 720;
		c_entity.video_fps = 30;
		c_entity.video_block_width = 128;
		c_entity.video_block_height = 72;
		c_entity.video_compression_level = 5;
		c_entity.video_quantization_colors = 36;
		c_entity.enable_tls = false;
		c_entity.enable_keepalive = false;
		c_entity.enable_present = false;
		c_entity.enable_auto_start = false;
		c_entity.enable_caching = false;
		strncpy_s(c_entity.app_session_app, "", sizeof(c_entity.app_session_app));

		status = create(&c_entity, conn);
		if (status != sirius::app::server::arbitrator::db::configuration_dao::err_code_t::success)
			return status;
	}

	std::string sql = "SELECT uuid, url, max_attendant_instance, attendant_creation_delay, controller_portnumber, streamer_portnumber, ";
	sql += "video_codec, video_width, video_height, video_fps, ";
	sql += "video_block_width, video_block_height, video_compression_level, video_quantization_colors, ";
	sql += "enable_tls, enable_keepalive, enable_present, enable_auto_start, enable_caching, ";
	sql += "app_session_app ";
	sql += "FROM tb_configuration";
	if (sqlite3_prepare(conn, sql.c_str(), -1, &stmt, 0) == SQLITE_OK)
	{
		int32_t result = SQLITE_ERROR;
		while (true)
		{
			result = sqlite3_step(stmt);
			if (result == SQLITE_ROW)
			{
				int32_t index = 0;
				char * uuid = (char*)sqlite3_column_text(stmt, index++);
				strncpy_s(entity->uuid, uuid, sizeof(entity->uuid));
				char * url = (char*)sqlite3_column_text(stmt, index++);
				strncpy_s(entity->url, url, sizeof(entity->url));
				entity->max_attendant_instance = sqlite3_column_int(stmt, index++);
				entity->attendant_creation_delay = sqlite3_column_int(stmt, index++);
				entity->controller_portnumber = sqlite3_column_int(stmt, index++);
				entity->streamer_portnumber = sqlite3_column_int(stmt, index++);
				entity->video_codec = sqlite3_column_int(stmt, index++);
				entity->video_width = sqlite3_column_int(stmt, index++);
				entity->video_height = sqlite3_column_int(stmt, index++);
				entity->video_fps = sqlite3_column_int(stmt, index++);
				entity->video_block_width = sqlite3_column_int(stmt, index++);
				entity->video_block_height = sqlite3_column_int(stmt, index++);
				entity->video_compression_level = sqlite3_column_int(stmt, index++);
				entity->video_quantization_colors = sqlite3_column_int(stmt, index++);
				entity->enable_tls = sqlite3_column_int(stmt, index++) ? true : false;
				entity->enable_keepalive = sqlite3_column_int(stmt, index++) ? true : false;
				entity->enable_present = sqlite3_column_int(stmt, index++) ? true : false;
				entity->enable_auto_start = sqlite3_column_int(stmt, index++) ? true : false;
				entity->enable_caching = sqlite3_column_int(stmt, index++) ? true : false;
				char * app_session_app = (char*)sqlite3_column_text(stmt, index++);
				strncpy_s(entity->app_session_app, app_session_app, sizeof(entity->app_session_app));

				status = sirius::app::server::arbitrator::db::configuration_dao::err_code_t::success;
			}
			else
			{
				break;
			}
		}
	}
	sqlite3_reset(stmt);
	sqlite3_finalize(stmt);
	return status;
}

int32_t sirius::app::server::arbitrator::db::configuration_dao::create(sirius::app::server::arbitrator::entity::configuration_t * entity, sqlite3 * connection)
{
	int32_t status = sirius::app::server::arbitrator::db::configuration_dao::err_code_t::fail;
	if (!entity)
		return status;

	sqlite3_stmt * stmt;
	sqlite3 * conn;

	if (connection == 0)
		conn = _db;
	else
		conn = connection;

	std::string sql = "INSERT INTO tb_configuration (uuid, url, max_attendant_instance, attendant_creation_delay, controller_portnumber, streamer_portnumber, video_codec, video_width, video_height, video_fps, video_block_width, video_block_height, video_compression_level, video_quantization_colors, enable_tls, enable_keepalive, enable_present, enable_auto_start, enable_caching, app_session_app) ";
	sql += "values (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)";

	if (sqlite3_prepare(conn, sql.c_str(), -1, &stmt, 0) == SQLITE_OK)
	{
		int32_t index = 0;
		sqlite3_bind_text(stmt, ++index, entity->uuid, -1, 0);
		sqlite3_bind_text(stmt, ++index, entity->url, -1, 0);
		sqlite3_bind_int(stmt, ++index, entity->max_attendant_instance);
		sqlite3_bind_int(stmt, ++index, entity->attendant_creation_delay);
		sqlite3_bind_int(stmt, ++index, entity->controller_portnumber);
		sqlite3_bind_int(stmt, ++index, entity->streamer_portnumber);
		sqlite3_bind_int(stmt, ++index, entity->video_codec);
		sqlite3_bind_int(stmt, ++index, entity->video_width);
		sqlite3_bind_int(stmt, ++index, entity->video_height);
		sqlite3_bind_int(stmt, ++index, entity->video_fps);
		sqlite3_bind_int(stmt, ++index, entity->video_block_width);
		sqlite3_bind_int(stmt, ++index, entity->video_block_height);
		sqlite3_bind_int(stmt, ++index, entity->video_compression_level);
		sqlite3_bind_int(stmt, ++index, entity->video_quantization_colors);
		sqlite3_bind_int(stmt, ++index, entity->enable_tls ? 1 : 0);
		sqlite3_bind_int(stmt, ++index, entity->enable_keepalive ? 1 : 0);
		sqlite3_bind_int(stmt, ++index, entity->enable_present ? 1 : 0);
		sqlite3_bind_int(stmt, ++index, entity->enable_auto_start ? 1 : 0);
		sqlite3_bind_int(stmt, ++index, entity->enable_caching ? 1 : 0);
		sqlite3_bind_text(stmt, ++index, entity->app_session_app, -1, 0);

		int32_t result = SQLITE_ERROR;
		result = sqlite3_step(stmt);
		if (result == SQLITE_DONE)
		{
			status = sirius::app::server::arbitrator::db::configuration_dao::err_code_t::success;
		}
	}
	sqlite3_reset(stmt);
	sqlite3_finalize(stmt);
	return status;
}

int32_t sirius::app::server::arbitrator::db::configuration_dao::retrieve_count(sqlite3 * connection)
{
	int32_t count = -1;
	sqlite3_stmt * stmt;
	sqlite3 * conn;

	if (connection == 0)
		conn = _db;
	else
		conn = connection;

	if (sqlite3_prepare(conn, "SELECT count(*) FROM tb_configuration;", -1, &stmt, 0) == SQLITE_OK)
	{
		int result = SQLITE_ERROR;
		while (true)
		{
			result = sqlite3_step(stmt);
			if (result == SQLITE_ROW)
			{
				count = sqlite3_column_int(stmt, 0);
			}
			else
				break;
		}
	}
	sqlite3_reset(stmt);
	sqlite3_finalize(stmt);
	return count;
}