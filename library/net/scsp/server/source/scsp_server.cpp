#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <crtdbg.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <conio.h>
#include <intrin.h>
#include <iphlpapi.h>
#include <iptypes.h>
#include <iprtrmib.h>
#include <tlhelp32.h>

#include "scsp_server.h"
#include <commands_server.h>
#include <sirius_log4cplus_logger.h>
#include <json/json.h>
#include <sirius_locks.h>

#include <commands_payload.h>

sirius::library::net::scsp::server::core::core(const char * uuid, sirius::library::net::scsp::server * front)
	: sirius::library::net::sicp::server(uuid, RECV_BUF_SIZE, SEND_BUF_SIZE, RECV_BUF_SIZE, SEND_BUF_SIZE, IO_THREAD_POOL_COUNT, COMMAND_THREAD_POOL_COUNT, FALSE, 5000, FALSE)
	, _context(nullptr)
	, _front(front)
{
	add_command(new sirius::library::net::scsp::play_req_cmd(this));
}

sirius::library::net::scsp::server::core::~core(void)
{

}

int32_t sirius::library::net::scsp::server::core::start(sirius::library::net::scsp::server::context_t * context)
{
	if (!context)
		return sirius::library::net::scsp::server::err_code_t::fail;

	_context = context;
	return sirius::library::net::sicp::server::start(nullptr, _context->portnumber);
}

int32_t sirius::library::net::scsp::server::core::stop(void)
{
	return sirius::library::net::sicp::server::stop();
}

int32_t sirius::library::net::scsp::server::core::post_indexed_video(uint8_t * bytes, size_t nbytes, long long timestamp)
{
	int32_t status = sirius::library::net::scsp::server::err_code_t::success;

	if (state(sirius::library::net::scsp::server::media_type_t::video) != sirius::library::net::scsp::server::state_t::stopped)
	{
		if (state(sirius::library::net::scsp::server::media_type_t::video) == sirius::library::net::scsp::server::state_t::begin_publishing)
			_video_conf.state = sirius::library::net::scsp::server::state_t::publishing;


		if (_video_conf.peers.size() > 0)
		{
			sirius::autolock mutex(&_video_conf.cs);

			std::vector<std::shared_ptr<sirius::library::net::scsp::server::core::stream_session_info_t>>::iterator dst_uuid_iter;
			for (dst_uuid_iter = _video_conf.peers.begin(); dst_uuid_iter != _video_conf.peers.end(); dst_uuid_iter++)
			{
				std::shared_ptr<sirius::library::net::scsp::server::core::stream_session_info_t> peer = *dst_uuid_iter;
				data_request((char*)peer->uuid, CMD_VIDEO_INDEXED_STREAM_DATA, reinterpret_cast<char*>(bytes), nbytes);
			}
		}
		else
		{
			_video_conf.state = sirius::library::net::scsp::server::state_t::stopped;
		}
	}
	else
	{
		status = sirius::library::net::scsp::server::err_code_t::not_implemented;
	}
	return status;
}

int32_t sirius::library::net::scsp::server::core::post_coordinates_video(uint8_t * bytes, size_t nbytes, long long timestamp)
{
	int32_t status = sirius::library::net::scsp::server::err_code_t::success;

	if (state(sirius::library::net::scsp::server::media_type_t::video) != sirius::library::net::scsp::server::state_t::stopped)
	{
		if (state(sirius::library::net::scsp::server::media_type_t::video) == sirius::library::net::scsp::server::state_t::begin_publishing)
			_video_conf.state = sirius::library::net::scsp::server::state_t::publishing;


		if (_video_conf.peers.size() > 0)
		{
			sirius::autolock mutex(&_video_conf.cs);

			std::vector<std::shared_ptr<sirius::library::net::scsp::server::core::stream_session_info_t>>::iterator dst_uuid_iter;
			for (dst_uuid_iter = _video_conf.peers.begin(); dst_uuid_iter != _video_conf.peers.end(); dst_uuid_iter++)
			{
				std::shared_ptr<sirius::library::net::scsp::server::core::stream_session_info_t> peer = *dst_uuid_iter;
				data_request((char*)peer->uuid, CMD_VIDEO_COORDINATES_STREAM_DATA, reinterpret_cast<char*>(bytes), nbytes);
			}
		}
		else
		{
			_video_conf.state = sirius::library::net::scsp::server::state_t::stopped;
		}
	}
	else
	{
		status = sirius::library::net::scsp::server::err_code_t::not_implemented;
	}
	return status;
}

int32_t sirius::library::net::scsp::server::core::play(int32_t flags)
{
	if (_context && _context->controller)
		return _context->controller->play(flags);
	return sirius::library::net::scsp::server::err_code_t::fail;
}

int32_t sirius::library::net::scsp::server::core::pause(int32_t flags)
{
	if (_context && _context->controller)
		return _context->controller->pause(flags);
	return sirius::library::net::scsp::server::err_code_t::fail;
}

int32_t sirius::library::net::scsp::server::core::stop(int32_t flags)
{
	if (_context && _context->controller)
		return _context->controller->stop(flags);
	return sirius::library::net::scsp::server::err_code_t::fail;
}

int32_t sirius::library::net::scsp::server::core::invalidate(void)
{
	if (_context && _context->controller)
		return _context->controller->invalidate();
	return sirius::library::net::scsp::server::err_code_t::fail;
}

void sirius::library::net::scsp::server::core::on_create_session(const char * uuid)
{
	LOGGER::make_trace_log(SLNS, "sirius::library::net::scsp::server", "%s(), connect [src=%s]", __FUNCTION__, uuid);
}

void sirius::library::net::scsp::server::core::on_destroy_session(const char * uuid)
{
	LOGGER::make_trace_log(SLNS, "sirius::library::net::scsp::server", "%s(), disconnect [src=%s]", __FUNCTION__, uuid);

	std::vector<std::shared_ptr<sirius::library::net::scsp::server::core::stream_session_info_t>>::iterator dst_uuid_iter;
	{
		sirius::autolock mutex(&_video_conf.cs);

		for (dst_uuid_iter = _video_conf.peers.begin(); dst_uuid_iter != _video_conf.peers.end(); dst_uuid_iter++)
		{
			if (!strncmp((*dst_uuid_iter)->uuid, uuid, sizeof((*dst_uuid_iter)->uuid)))
				break;
		}
		if (dst_uuid_iter != _video_conf.peers.end())
		{
			_video_conf.peers.erase(dst_uuid_iter);
			if (_video_conf.peers.size() < 1)
			{
				_video_conf.state = sirius::library::net::scsp::server::state_t::stopped;
			}
		}
	}
}

int32_t sirius::library::net::scsp::server::core::play_callback(const char * client_uuid, int32_t type, const char * attendant_uuid)
{
/*
	int32_t res_code = sirius::base::err_code_t::fail;

	if (!_context)
		return res_code;

	LOGGER::make_trace_log(SLNS, "%s(), %d : client_uuid=%s, clientsocket=%d, msg=%s, length=%d", __FUNCTION__, __LINE__, client_uuid, clientsocket, msg, length);

	Json::Value root;
	Json::Reader reader;
	Json::StyledWriter writer;
	std::string control_uuid;

	reader.parse(msg, root);

	int32_t stream_type = 0;
	if (root["type"].isInt())
		stream_type = root.get("type", 0).asInt();

	LOGGER::make_info_log(SLNS, "[play request] - command:%d", CMD_PLAY_REQ);
	LOGGER::make_info_log(SLNS, "%s(),%d, play request packet -> type : %d", __FUNCTION__, __LINE__, stream_type);
	control_uuid = root.get("attendant_uuid", _context->uuid).asString();
	root.clear();
	if ((control_uuid.compare(_context->uuid) == 0) && ((stream_type == sirius::library::net::scsp::server::media_type_t::video) || (stream_type == sirius::library::net::scsp::server::media_type_t::audio)))
	{
		if (stream_type == sirius::library::net::scsp::server::media_type_t::video)
		{
			root["type"] = stream_type;
			root["codec"] = _context->video_codec;
			root["video_width"] = _context->video_width;
			root["video_height"] = _context->video_height;
			root["video_fps"] = _context->video_fps;
			root["video_block_width"] = _context->video_block_width;
			root["video_block_height"] = _context->video_block_height;
		}
		res_code = sirius::base::err_code_t::success;
	}
	else
	{
		if (stream_type != sirius::library::net::scsp::server::media_type_t::video)
		{
			res_code = sirius::base::err_code_t::fail;
			LOGGER::make_error_log(SLNS, "%s(),%d, uuid does not match", __FUNCTION__, __LINE__);
		}
	}

	root["rcode"] = res_code;
	std::string res_json = writer.write(root);

	LOGGER::make_trace_log(SLNS, "%s(), %d : client uuid=%s, client uuid=%s, msg=%s, length=%d", __FUNCTION__, __LINE__, client_uuid, control_uuid.c_str(), res_json.c_str(), res_json.size());

	data_request((char*)client_uuid, CMD_PLAY_RES, (char*)res_json.c_str(), res_json.size());
	LOGGER::make_info_log(SLNS, "[stream response] - command:%d", CMD_PLAY_RES);
	LOGGER::make_info_log(SLNS, "%s(),%d, stream response packet -> type : %d, rcode : %d", __FUNCTION__, __LINE__, stream_type, res_code);
	if (res_code == sirius::base::err_code_t::success)
	{
		if (stream_type == sirius::library::net::scsp::server::media_type_t::video)
		{
			{
				sirius::autolock mutex(&_video_conf.cs);

				std::shared_ptr<sirius::library::net::scsp::server::core::stream_session_info_t> peer(new sirius::library::net::scsp::server::core::stream_session_info_t);
				strncpy_s(peer->uuid, client_uuid, sizeof(peer->uuid));
				_video_conf.peers.push_back(peer);
				if (_video_conf.state != sirius::library::net::scsp::server::state_t::begin_publishing)
				{
					_video_conf.state = sirius::library::net::scsp::server::state_t::begin_publishing;
					play(sirius::library::net::scsp::server::media_type_t::video);
				}
			}

		}
		else
		{
			LOGGER::make_error_log(SLNS, "%s(), %d : client uuid = %s msg=%s, res_msg=%s", __FUNCTION__, __LINE__, client_uuid, msg, res_json.c_str());
			res_code = -1;
		}
	}
	else
	{
		LOGGER::make_error_log(SLNS, "%s(), %d : client_uuid=%s, attendant_uuid=%s, msg=%s, res_msg=%s", __FUNCTION__, __LINE__, client_uuid, _context->uuid, msg, res_json.c_str());
	}
	return res_code;
*/
	int32_t status = sirius::library::net::scsp::server::err_code_t::fail;

	if (!_context)
		return sirius::library::net::scsp::server::err_code_t::fail;

	Json::Value wpacket;
	Json::StyledWriter writer;
	if (type == sirius::library::net::scsp::server::media_type_t::video)
	{
		wpacket["type"] = type;
		wpacket["codec"] = _context->video_codec;
		wpacket["video_width"] = _context->video_width;
		wpacket["video_height"] = _context->video_height;
		wpacket["video_fps"] = _context->video_fps;
		wpacket["video_block_width"] = _context->video_block_width;
		wpacket["video_block_height"] = _context->video_block_height;
		status = sirius::library::net::scsp::server::err_code_t::success;
	}
	else
	{
		status = sirius::library::net::scsp::server::err_code_t::fail;
	}

	wpacket["rcode"] = status;
	std::string response = writer.write(wpacket);
	data_request((char*)client_uuid, CMD_PLAY_RES, (char*)response.c_str(), response.size());

	if (status == sirius::library::net::scsp::server::err_code_t::success)
	{
		if (type == sirius::library::net::scsp::server::media_type_t::video)
		{
			{
				sirius::autolock mutex(&_video_conf.cs);

				std::shared_ptr<sirius::library::net::scsp::server::core::stream_session_info_t> peer = std::shared_ptr<sirius::library::net::scsp::server::core::stream_session_info_t>(new sirius::library::net::scsp::server::core::stream_session_info_t);
				strncpy_s(peer->uuid, client_uuid, sizeof(peer->uuid));
				_video_conf.peers.push_back(peer);
				if (_video_conf.state != sirius::library::net::scsp::server::state_t::begin_publishing)
				{
					_video_conf.state = sirius::library::net::scsp::server::state_t::begin_publishing;
					play(sirius::library::net::scsp::server::media_type_t::video);
				}
				else
				{
					invalidate();
				}
			}

		}
		else
		{
			status = -1;
		}
	}
	else
	{
		//LOGGER::make_error_log("sirius::library::net::scsp::server", "%s(),%d : client_uuid=%s, container_uuid=%s, msg=%s, res_msg=%s", __FUNCTION__, __LINE__, client_uuid, _context->uuid, msg, res_json.c_str());
	}
	return status;
}

int32_t sirius::library::net::scsp::server::core::state(int32_t type)
{
	int32_t state;

	if (type == sirius::library::net::scsp::server::media_type_t::video)
		state = _video_conf.state;

	return state;
}