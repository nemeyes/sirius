#include "arbitrator_session.h"

#define UNDEFINED_UUID	"FFFFFFFF-FFFF-FFFF-FFFF-FFFFFFFFFFFE"

sirius::app::server::arbitrator::session::session(int32_t id)
	: _id(id)
{		
	strncpy_s(_client_uuid, UNDEFINED_UUID, sizeof(UNDEFINED_UUID));
	
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




