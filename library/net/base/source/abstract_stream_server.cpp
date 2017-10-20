#include <abstract_stream_server.h>

sirius::library::net::stream::server::server(void)
{
}

sirius::library::net::stream::server::~server(void)
{

}

int32_t sirius::library::net::stream::server::start(sirius::library::net::stream::server::context_t * context)
{
	return sirius::library::net::stream::server::err_code_t::not_implemented;
}

int32_t sirius::library::net::stream::server::stop(void)
{
	return sirius::library::net::stream::server::err_code_t::not_implemented;
}

int32_t sirius::library::net::stream::server::post_video(uint8_t * bytes, size_t nbytes, long long timestamp)
{
	return sirius::library::net::stream::server::err_code_t::not_implemented;
}

sirius::library::net::stream::server::network_usage_t & sirius::library::net::stream::server::get_network_usage(void)
{
	return _network_usage;
}
