#include "sirius_timestamp_generator.h"
#include "timestamp_generator.h"

sirius::library::misc::timestamp::generator::generator(void)
{
	_generator = new sirius::library::misc::timestamp::generator::core();
}

sirius::library::misc::timestamp::generator::~generator(void)
{
	if (_generator)
		delete _generator;
	_generator = nullptr;
}

sirius::library::misc::timestamp::generator & sirius::library::misc::timestamp::generator::instance(void)
{
	static sirius::library::misc::timestamp::generator _instance;
	return _instance;
}

void sirius::library::misc::timestamp::generator::begin_elapsed_time(void)
{
	if (_generator)
		_generator->begin_elapsed_time();
}

long long sirius::library::misc::timestamp::generator::elapsed_microseconds(void)
{
	if (_generator)
		return _generator->elapsed_microseconds();
	return 0;
}

long long sirius::library::misc::timestamp::generator::elapsed_milliseconds(void)
{
	if (_generator)
		return _generator->elapsed_milliseconds();
	return 0;
}

