#include "sirius_performance_monitor.h"
#include "performance_monitor.h"

sirius::library::misc::performance::monitor::monitor(void)
{
	_core = new sirius::library::misc::performance::monitor::core(this);
}

sirius::library::misc::performance::monitor::~monitor(void)
{
	if (_core)
	{
		delete _core;
		_core = nullptr;
	}
}

int32_t sirius::library::misc::performance::monitor::initialize(void)
{
	return _core->initialize();
}

int32_t sirius::library::misc::performance::monitor::release(void)
{
	return _core->release();
}

char * sirius::library::misc::performance::monitor::cpu_info(void)
{
	return _core->cpu_info();
}

char * sirius::library::misc::performance::monitor::mem_info(void)
{
	return _core->mem_info();
}

double sirius::library::misc::performance::monitor::total_cpu_usage(void)
{
	return _core->total_cpu_usage();
}

double sirius::library::misc::performance::monitor::process_cpu_usage(int32_t container_number)
{
	return _core->process_cpu_usage(container_number);
}

double sirius::library::misc::performance::monitor::total_mem_usage(void)
{
	return _core->total_mem_usage();
}