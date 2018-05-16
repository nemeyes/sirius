#include "arbitrator_session.h"

#define UNDEFINED_UUID	"FFFFFFFF-FFFF-FFFF-FFFF-FFFFFFFFFFFE"
#define UNDEFINED_CLIENT_ID "FF:FF:FF:FF"

sirius::app::server::arbitrator::session::session(int32_t id)
	: _id(id)
	, _pid(0)
	, _total_bandwidth_byte(0)
	, _state(0)
{		
	strncpy_s(_client_uuid, UNDEFINED_UUID, sizeof(UNDEFINED_UUID));
	strncpy_s(_attendant_uuid, UNDEFINED_UUID, sizeof(UNDEFINED_UUID));
	strncpy_s(_client_id, UNDEFINED_CLIENT_ID, sizeof(UNDEFINED_CLIENT_ID));
	::InitializeCriticalSection(&_lock);
}

sirius::app::server::arbitrator::session::~session(void)
{
	::DeleteCriticalSection(&_lock);
}

int32_t sirius::app::server::arbitrator::session::id(void)
{
	return _id;
}

int32_t sirius::app::server::arbitrator::session::pid(void)
{
	return _pid;
}

int32_t sirius::app::server::arbitrator::session::state(void)
{
	return _state;
}

uint32_t sirius::app::server::arbitrator::session::total_bandwidth_byte(void)
{
	return _total_bandwidth_byte;
}

uint64_t sirius::app::server::arbitrator::session::connect_timestamp(void)
{
	return _connect_timestamp;
}

uint64_t sirius::app::server::arbitrator::session::disconnect_timestamp(void)
{
	return _disconnect_timestamp;
}

const char * sirius::app::server::arbitrator::session::attendant_uuid(void)
{
	return _attendant_uuid;
}

const char * sirius::app::server::arbitrator::session::client_uuid(void)
{
	return _client_uuid;
}

const char * sirius::app::server::arbitrator::session::client_id(void)
{
	return _client_id;
}

void sirius::app::server::arbitrator::session::pid(int32_t pid)
{
	_pid = pid;
}

void sirius::app::server::arbitrator::session::attendant_uuid(const char * uuid)
{
	strncpy_s(_attendant_uuid, uuid, sizeof(_attendant_uuid));
}

void sirius::app::server::arbitrator::session::client_uuid(const char * uuid)
{
	strncpy_s(_client_uuid, uuid, sizeof(_client_uuid));
}

void sirius::app::server::arbitrator::session::client_id(const char * client_id)
{
	strncpy_s(_client_id, client_id, sizeof(_client_uuid));
}

void sirius::app::server::arbitrator::session::state(int32_t state)
{
	_state = state;
}

void sirius::app::server::arbitrator::session::total_bandwidth_byte(uint32_t total_bandwidth_byte)
{
	_total_bandwidth_byte = total_bandwidth_byte;
}

void sirius::app::server::arbitrator::session::connect_timestamp(uint64_t connect_timestamp)
{
	_connect_timestamp = connect_timestamp;
}

void sirius::app::server::arbitrator::session::disconnect_timestamp(uint64_t disconnect_timestamp)
{
	_disconnect_timestamp = disconnect_timestamp;
}




