#include "attendant_dao.h"
#include <string>

sirius::app::server::arbitrator::db::attendant_dao::attendant_dao(const char * path)
	: sirius::app::server::arbitrator::db::data_access_object(path)
{

}

sirius::app::server::arbitrator::db::attendant_dao::~attendant_dao(void)
{

}

int32_t sirius::app::server::arbitrator::db::attendant_dao::add(sirius::app::server::arbitrator::entity::attendant_t * entity, sqlite3 * connection)
{
	int32_t status = sirius::app::server::arbitrator::db::attendant_dao::err_code_t::fail;
	if (!entity)
		return status;

	sqlite3_stmt * stmt;
	sqlite3 * conn;

	if (connection == 0)
		conn = _db;
	else
		conn = connection;

	std::string sql = "INSERT INTO tb_attendant (uuid, id, client_uuid, client_id, pid, state, total_bandwidth_bytes) ";
	sql += "values (?,?,?,?,?,?,?)";

	if (sqlite3_prepare(conn, sql.c_str(), -1, &stmt, 0) == SQLITE_OK)
	{
		int32_t index = 0;
		sqlite3_bind_text(stmt, ++index, entity->uuid, -1, 0);
		sqlite3_bind_int(stmt, ++index, entity->id);
		sqlite3_bind_text(stmt, ++index, entity->client_uuid, -1, 0);
		sqlite3_bind_text(stmt, ++index, entity->client_id, -1, 0);
		sqlite3_bind_int(stmt, ++index, entity->pid);
		sqlite3_bind_int(stmt, ++index, entity->state);
		sqlite3_bind_int64(stmt, ++index, entity->total_bandwidth_bytes);

		int32_t result = SQLITE_ERROR;
		result = sqlite3_step(stmt);
		if (result == SQLITE_DONE)
		{
			status = sirius::app::server::arbitrator::db::attendant_dao::err_code_t::success;
		}
	}
	sqlite3_reset(stmt);
	sqlite3_finalize(stmt);
	return status;
}

int32_t sirius::app::server::arbitrator::db::attendant_dao::remove(sirius::app::server::arbitrator::entity::attendant_t * entity, sqlite3 * connection)
{
	int32_t status = sirius::app::server::arbitrator::db::attendant_dao::err_code_t::fail;
	if (!entity)
		return status;

	sqlite3 *conn;
	if (connection == 0)
		conn = _db;
	else
		conn = connection;

	sqlite3_stmt *stmt;
	if (sqlite3_prepare(conn, "DELETE FROM tb_attendant WHERE id=?;", -1, &stmt, 0) == SQLITE_OK)
	{
		sqlite3_bind_int(stmt, 1, entity->id);
		int result = SQLITE_ERROR;
		result = sqlite3_step(stmt);
		if (result == SQLITE_DONE)
		{
			status = sirius::app::server::arbitrator::db::attendant_dao::err_code_t::success;
		}
	}
	sqlite3_reset(stmt);
	sqlite3_finalize(stmt);
	return status;
}

int32_t sirius::app::server::arbitrator::db::attendant_dao::remove(sqlite3 * connection)
{
	int32_t status = sirius::app::server::arbitrator::db::attendant_dao::err_code_t::fail;

	sqlite3 *conn;
	if (connection == 0)
		conn = _db;
	else
		conn = connection;

	sqlite3_stmt *stmt;
	if (sqlite3_prepare(conn, "DELETE FROM tb_attendant;", -1, &stmt, 0) == SQLITE_OK)
	{
		int result = SQLITE_ERROR;
		result = sqlite3_step(stmt);
		if (result == SQLITE_DONE)
		{
			status = sirius::app::server::arbitrator::db::attendant_dao::err_code_t::success;
		}
	}
	sqlite3_reset(stmt);
	sqlite3_finalize(stmt);
	return status;
}


int32_t sirius::app::server::arbitrator::db::attendant_dao::update(sirius::app::server::arbitrator::entity::attendant_t * entity, sqlite3 * connection)
{
	int32_t status = sirius::app::server::arbitrator::db::attendant_dao::err_code_t::fail;
	if (!entity)
		return status;

	sqlite3_stmt * stmt;
	sqlite3 * conn;

	if (connection == 0)
		conn = _db;
	else
		conn = connection;

	std::string sql = "UPDATE tb_attendant SET ";
	sql += "uuid=?, client_uuid=?, ";
	sql += "pid=?, client_id=?, ";
	sql += "state=?, total_bandwidth_bytes=? ";
	sql += "WHERE id=?";

	if (sqlite3_prepare(conn, sql.c_str(), -1, &stmt, 0) == SQLITE_OK)
	{
		int32_t index = 0;
		sqlite3_bind_text(stmt, ++index, entity->uuid, -1, 0);
		sqlite3_bind_text(stmt, ++index, entity->client_uuid, -1, 0);
		sqlite3_bind_int(stmt, ++index, entity->pid);
		sqlite3_bind_text(stmt, ++index, entity->client_id, -1, 0);
		sqlite3_bind_int(stmt, ++index, entity->state);
		sqlite3_bind_int64(stmt, ++index, entity->total_bandwidth_bytes);
		sqlite3_bind_int(stmt, ++index, entity->id);

		int32_t result = sqlite3_step(stmt);
		if (result == SQLITE_DONE)
		{
			status = sirius::app::server::arbitrator::db::attendant_dao::err_code_t::success;
		}
	}
	sqlite3_reset(stmt);
	sqlite3_finalize(stmt);
	return status;
}

int32_t sirius::app::server::arbitrator::db::attendant_dao::update(int32_t state, int32_t type, const char * uuid, sqlite3 * connection)
{
	int32_t status = sirius::app::server::arbitrator::db::attendant_dao::err_code_t::fail;
	if (!uuid)
		return status;

	sqlite3_stmt * stmt;
	sqlite3 * conn;

	if (connection == 0)
		conn = _db;
	else
		conn = connection;

	
	std::string sql = "UPDATE tb_attendant SET ";
	sql += "state=? ";
	if (type==sirius::app::server::arbitrator::db::attendant_dao::type_t::attendant)
		sql += "WHERE uuid=?";
	else if (type == sirius::app::server::arbitrator::db::attendant_dao::type_t::client)
		sql += "WHERE client_uuid=?";

	if (sqlite3_prepare(conn, sql.c_str(), -1, &stmt, 0) == SQLITE_OK)
	{
		int32_t index = 0;
		sqlite3_bind_int(stmt, ++index, state);
		sqlite3_bind_text(stmt, ++index, uuid, -1, 0);

		int32_t result = sqlite3_step(stmt);
		if (result == SQLITE_DONE)
		{
			status = sirius::app::server::arbitrator::db::attendant_dao::err_code_t::success;
		}
	}
	sqlite3_reset(stmt);
	sqlite3_finalize(stmt);
	return status;
}

int32_t sirius::app::server::arbitrator::db::attendant_dao::retrieve(sirius::app::server::arbitrator::entity::attendant_t * entity, sqlite3 * connection)
{
	int32_t status = sirius::app::server::arbitrator::db::attendant_dao::err_code_t::fail;
	if (!entity)
		return status;

	sqlite3_stmt * stmt;
	sqlite3 * conn;

	if (connection == 0)
		conn = _db;
	else
		conn = connection;

	std::string sql = "SELECT uuid, client_uuid, id, client_id, state, total_bandwidth_bytes FROM tb_attendant WHERE pid=?";
	if (sqlite3_prepare(conn, sql.c_str(), -1, &stmt, 0) == SQLITE_OK)
	{
		sqlite3_bind_int(stmt, 1, entity->pid);
		int32_t result = SQLITE_ERROR;
		while (true)
		{
			result = sqlite3_step(stmt);
			if (result == SQLITE_ROW)
			{
				int32_t index = 0;
				
				char * uuid = (char*)sqlite3_column_text(stmt, index++);
				strncpy_s(entity->uuid, uuid, sizeof(entity->uuid));
				char * client_uuid = (char*)sqlite3_column_text(stmt, index++);
				strncpy_s(entity->client_uuid, client_uuid, sizeof(entity->client_uuid));

				entity->id = sqlite3_column_int(stmt, index++);
				char * client_id = (char*)sqlite3_column_text(stmt, index++);
				strncpy_s(entity->client_id, client_id, sizeof(entity->client_id));

				entity->state = sqlite3_column_int(stmt, index++);
				entity->total_bandwidth_bytes = sqlite3_column_int64(stmt, index++);


				status = sirius::app::server::arbitrator::db::attendant_dao::err_code_t::success;
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

int32_t sirius::app::server::arbitrator::db::attendant_dao::retrieve(sirius::app::server::arbitrator::entity::attendant_t *** entity, int32_t & count, sqlite3 * connection)
{
	int32_t status = sirius::app::server::arbitrator::db::attendant_dao::err_code_t::fail;
	if (!entity)
		return status;

	sqlite3_stmt * stmt;
	sqlite3 * conn;

	if (connection == 0)
		conn = _db;
	else
		conn = connection;

	count = retrieve_count(conn);
	*entity = static_cast<sirius::app::server::arbitrator::entity::attendant_t**>(malloc(sizeof(sirius::app::server::arbitrator::entity::attendant_t*)*count));

	std::string sql = "SELECT uuid, client_uuid, id, client_id, pid, state, total_bandwidth_bytes FROM tb_attendant;";
	if (sqlite3_prepare(conn, sql.c_str(), -1, &stmt, 0) == SQLITE_OK)
	{
		int32_t result = SQLITE_ERROR;
		int32_t index = 0;
		while (true)
		{
			result = sqlite3_step(stmt);
			if (result == SQLITE_ROW)
			{
				int32_t column_index = 0;

				(*entity)[index] = static_cast<sirius::app::server::arbitrator::entity::attendant_t*>(malloc(sizeof(sirius::app::server::arbitrator::entity::attendant_t)));
				char * uuid = (char*)sqlite3_column_text(stmt, column_index++);
				strncpy_s((*entity)[index]->uuid, uuid, sizeof((*entity)[index]->uuid));
				char * client_uuid = (char*)sqlite3_column_text(stmt, column_index++);
				strncpy_s((*entity)[index]->client_uuid, client_uuid, sizeof((*entity)[index]->client_uuid));

				(*entity)[index]->id = sqlite3_column_int(stmt, column_index++);
				char * client_id = (char*)sqlite3_column_text(stmt, column_index++);
				strncpy_s((*entity)[index]->client_id, client_id, sizeof((*entity)[index]->client_id));

				(*entity)[index]->pid = sqlite3_column_int(stmt, column_index++);
				(*entity)[index]->state = sqlite3_column_int(stmt, column_index++);
				(*entity)[index]->total_bandwidth_bytes = sqlite3_column_int64(stmt, column_index++);

				index++;

				status = sirius::app::server::arbitrator::db::attendant_dao::err_code_t::success;
			}
			else
			{
				if (result == SQLITE_DONE)
				{
					status = sirius::app::server::arbitrator::db::attendant_dao::err_code_t::success;
				}
				break;
			}
		}
	}
	sqlite3_reset(stmt);
	sqlite3_finalize(stmt);
	return status;
}

int32_t sirius::app::server::arbitrator::db::attendant_dao::retrieve(int32_t state, sirius::app::server::arbitrator::entity::attendant_t *** entity, int32_t & count, sqlite3 * connection)
{
	int32_t status = sirius::app::server::arbitrator::db::attendant_dao::err_code_t::fail;
	if (!entity)
		return status;

	sqlite3_stmt * stmt;
	sqlite3 * conn;

	if (connection == 0)
		conn = _db;
	else
		conn = connection;

	count = retrieve_count(state, conn);
	*entity = static_cast<sirius::app::server::arbitrator::entity::attendant_t**>(malloc(sizeof(sirius::app::server::arbitrator::entity::attendant_t*)*count));

	std::string sql = "SELECT uuid, client_uuid, id, client_id, pid, state, total_bandwidth_bytes FROM tb_attendant WHERE state=? ORDER BY id ASC;";
	if (sqlite3_prepare(conn, sql.c_str(), -1, &stmt, 0) == SQLITE_OK)
	{
		sqlite3_bind_int(stmt, 1, state);
		int32_t result = SQLITE_ERROR;
		int32_t index = 0;
		while (true)
		{
			result = sqlite3_step(stmt);
			if (result == SQLITE_ROW)
			{
				int32_t column_index = 0;

				(*entity)[index] = static_cast<sirius::app::server::arbitrator::entity::attendant_t*>(malloc(sizeof(sirius::app::server::arbitrator::entity::attendant_t)));
				char * uuid = (char*)sqlite3_column_text(stmt, column_index++);
				strncpy_s((*entity)[index]->uuid, uuid, sizeof((*entity)[index]->uuid));
				char * client_uuid = (char*)sqlite3_column_text(stmt, column_index++);
				strncpy_s((*entity)[index]->client_uuid, client_uuid, sizeof((*entity)[index]->client_uuid));

				(*entity)[index]->id = sqlite3_column_int(stmt, column_index++);
				char * client_id = (char*)sqlite3_column_text(stmt, column_index++);
				strncpy_s((*entity)[index]->client_id, client_id, sizeof((*entity)[index]->client_id));

				(*entity)[index]->pid = sqlite3_column_int(stmt, column_index++);
				(*entity)[index]->state = sqlite3_column_int(stmt, column_index++);
				(*entity)[index]->total_bandwidth_bytes = sqlite3_column_int64(stmt, column_index++);

				index++;

				status = sirius::app::server::arbitrator::db::attendant_dao::err_code_t::success;
			}
			else
			{
				if (result == SQLITE_DONE)
				{
					status = sirius::app::server::arbitrator::db::attendant_dao::err_code_t::success;
				}
				break;
			}
		}
	}
	sqlite3_reset(stmt);
	sqlite3_finalize(stmt);
	return status;
}

int32_t sirius::app::server::arbitrator::db::attendant_dao::retrieve(const char * client_uuid, sirius::app::server::arbitrator::entity::attendant_t * entity, sqlite3 * connection)
{
	int32_t status = sirius::app::server::arbitrator::db::attendant_dao::err_code_t::fail;
	if (!entity)
		return status;

	sqlite3_stmt * stmt;
	sqlite3 * conn;

	if (connection == 0)
		conn = _db;
	else
		conn = connection;

	std::string sql = "SELECT uuid, client_uuid, id, client_id, state, total_bandwidth_bytes FROM tb_attendant WHERE client_uuid=?";
	if (sqlite3_prepare(conn, sql.c_str(), -1, &stmt, 0) == SQLITE_OK)
	{
		sqlite3_bind_text(stmt, 1, client_uuid, -1, 0);
		int32_t result = SQLITE_ERROR;
		while (true)
		{
			result = sqlite3_step(stmt);
			if (result == SQLITE_ROW)
			{
				int32_t index = 0;

				char * uuid = (char*)sqlite3_column_text(stmt, index++);
				strncpy_s(entity->uuid, uuid, sizeof(entity->uuid));
				char * client_uuid = (char*)sqlite3_column_text(stmt, index++);
				strncpy_s(entity->client_uuid, client_uuid, sizeof(entity->client_uuid));

				entity->id = sqlite3_column_int(stmt, index++);
				char * client_id = (char*)sqlite3_column_text(stmt, index++);
				strncpy_s(entity->client_id, client_id, sizeof(entity->client_id));

				entity->state = sqlite3_column_int(stmt, index++);
				entity->total_bandwidth_bytes = sqlite3_column_int64(stmt, index++);


				status = sirius::app::server::arbitrator::db::attendant_dao::err_code_t::success;
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


int32_t sirius::app::server::arbitrator::db::attendant_dao::retrieve(int32_t id, sqlite3 * connection)
{
	int32_t status = sirius::app::server::arbitrator::db::attendant_dao::err_code_t::fail;
	int32_t count = -1;

	sqlite3_stmt * stmt;
	sqlite3 * conn;

	if (connection == 0)
		conn = _db;
	else
		conn = connection;

	if (sqlite3_prepare(conn, "SELECT count(*) FROM tb_attendant WHERE id=?;", -1, &stmt, 0) == SQLITE_OK)
	{
		sqlite3_bind_int(stmt, 1, id);
		int result = SQLITE_ERROR;
		while (true)
		{
			result = sqlite3_step(stmt);
			if (result == SQLITE_ROW)
			{			
				count = sqlite3_column_int(stmt, 0);	
				if (count > 0)
					status = sirius::app::server::arbitrator::db::attendant_dao::err_code_t::success;
			}
			else
				break;
		}
	}	
	sqlite3_reset(stmt);
	sqlite3_finalize(stmt);
	return status;
}

int32_t sirius::app::server::arbitrator::db::attendant_dao::retrieve_count(sqlite3 * connection)
{
	int32_t count = 0;

	sqlite3_stmt * stmt;
	sqlite3 * conn;

	if (connection == 0)
		conn = _db;
	else
		conn = connection;

	if (sqlite3_prepare(conn, "SELECT count(*) FROM tb_attendant;", -1, &stmt, 0) == SQLITE_OK)
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

int32_t sirius::app::server::arbitrator::db::attendant_dao::retrieve_count(int32_t state, sqlite3 * connection)
{
	int32_t count = -1;

	sqlite3_stmt * stmt;
	sqlite3 * conn;

	if (connection == 0)
		conn = _db;
	else
		conn = connection;

	if (sqlite3_prepare(conn, "SELECT count(*) FROM tb_attendant WHERE state=?;", -1, &stmt, 0) == SQLITE_OK)
	{
		sqlite3_bind_int(stmt, 1, state);
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