#include <configuration_dao.h>
#include <configuration_entity.h>
#include <attendant_dao.h>
#include <attendant_entity.h>
#include <process_controller.h>
#include <sirius_stringhelper.h>
#include <map>

#define ARGUMENT_SIZE	2048
#define UNDEFINED_UUID	"FFFFFFFF-FFFF-FFFF-FFFF-FFFFFFFFFFFE"
#define MAX_GPU_COUNT	20

#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")

typedef struct _attendant_state_t
{
	static const int32_t idle = 0;
	static const int32_t available = 1;
	static const int32_t starting = 2;
	static const int32_t running = 3;
	static const int32_t stopping = 4;
} attendant_state_t;

typedef struct _context_t
{
	int32_t id;
	wchar_t uuid[64];

	_context_t(void) : id(0)
	{
		memset(uuid, 0x00, sizeof(uuid));
	}
	~_context_t(void)
	{

	}
} context_t;

bool parse_argument(int32_t argc, wchar_t * argv[], context_t * context)
{
	wchar_t * pargv;
	std::map<std::wstring, std::wstring> param;
	for (int32_t i = 1; i < argc; i++)
	{
		pargv = argv[i];
		if (wcsncmp(pargv, L"--", 2) == 0)
		{
			const wchar_t *p = wcschr(pargv + 2, L'=');
			if (p)
			{
				const wchar_t *f = pargv + 2;
				std::wstring name(f, p);
				std::wstring val(p + 1);
				param.insert(std::make_pair(name, val));
			}
			else
			{
				std::wstring name(pargv + 2);
				std::wstring val;
				val.clear();
				param.insert(std::make_pair(name, val));
			}
		}
		else
		{
			continue;
		}
	}

	std::map<std::wstring, std::wstring>::iterator iter;
	std::wstring value;
	if (param.end() != (iter = param.find(L"uuid")))
	{
		value = iter->second;
		wcscpy_s(context->uuid, value.c_str());
	}
	if (param.end() != (iter = param.find(L"id")))
	{
		value = iter->second;
		context->id = _wtoi(value.c_str());
	}
	return true;
}


void retrieve_db_path(char * path)
{
	char module_path[MAX_PATH] = { 0 };
	char * module_name = module_path;
	module_name += GetModuleFileNameA(NULL, module_name, (sizeof(module_path) / sizeof(*module_path)) - (module_name - module_path));
	if (module_name != module_path)
	{
		CHAR * slash = strrchr(module_path, '\\');
		if (slash != NULL)
		{
			module_name = slash + 1;
			_strset_s(module_name, strlen(module_name) + 1, 0);
		}
		else
		{
			_strset_s(module_path, strlen(module_path) + 1, 0);
		}
	}

	_snprintf_s(path, MAX_PATH, MAX_PATH, "%sdb\\sirius.db", module_path);
};

int main()
{
	HWND hwnd = GetConsoleWindow();
	ShowWindow(hwnd, 0);

	context_t context;
	wchar_t * command = GetCommandLine();
	const wchar_t * option = wcschr(command, L'--');
	if (option)
	{
		int32_t argc = 0;
		LPWSTR * argv = ::CommandLineToArgvW(command, &argc);		
		parse_argument(argc, argv, &context);
	}

	char db_path[MAX_PATH];
	retrieve_db_path(db_path);

	int32_t status = sirius::base::err_code_t::fail;
	sirius::app::server::arbitrator::entity::configuration_t confentity;
	{
		sirius::app::server::arbitrator::db::configuration_dao confdao(db_path);
		status = confdao.retrieve(&confentity);
	}

	if (status == sirius::base::err_code_t::success)
	{
		sirius::app::server::arbitrator::process::controller proc_ctrl;

		if(!option && confentity.clean_attendant)
			proc_ctrl.kill("sirius_web_attendant.exe");

		for (int32_t index = 0; index < confentity.max_attendant_instance; index++)
		{
			unsigned long pid = 0;
			char arguments[ARGUMENT_SIZE] = { 0 };
		
			sirius::app::server::arbitrator::entity::attendant_t contenity;
			
			sirius::uuid uuidgen;
			uuidgen.create();
			memmove(contenity.uuid, uuidgen.c_str(), strlen(uuidgen.c_str()) + 1);
						
			if (option)							
				contenity.id = context.id;			
			else			
				contenity.id = index;
			
			memmove(contenity.client_uuid, UNDEFINED_UUID, strlen(UNDEFINED_UUID) + 1);				
			contenity.state = attendant_state_t::idle;
			
			proc_ctrl.set_cmdline(arguments, "--reconnect=true");
			proc_ctrl.set_cmdline(arguments, "--uuid=\"%s\"", contenity.uuid);
			proc_ctrl.set_cmdline(arguments, "--attendant_type=\"web\"");
			proc_ctrl.set_cmdline(arguments, "--url=\"%s\"", confentity.url);
			switch (confentity.video_codec)
			{
				case sirius::base::video_submedia_type_t::png:
				{
					proc_ctrl.set_cmdline(arguments, "--video_codec=\"png\"");
					break;
				}
				case sirius::base::video_submedia_type_t::webp:
				{
					proc_ctrl.set_cmdline(arguments, "--video_codec=\"webp\"");
					break;
				}
			}
			proc_ctrl.set_cmdline(arguments, "--video_width=%d", confentity.video_width);
			proc_ctrl.set_cmdline(arguments, "--video_height=%d", confentity.video_height);
			proc_ctrl.set_cmdline(arguments, "--video_buffer_count=%d", confentity.video_buffer_count);
			proc_ctrl.set_cmdline(arguments, "--video_block_width=%d", confentity.video_block_width);
			proc_ctrl.set_cmdline(arguments, "--video_block_height=%d", confentity.video_block_height);
			
			if (confentity.video_codec == sirius::base::video_submedia_type_t::png)
			{
				proc_ctrl.set_cmdline(arguments, "--video_png_compression_level=%d", confentity.png.video_compression_level);
				if (confentity.png.video_quantization_posterization)
					proc_ctrl.set_cmdline(arguments, "--video_png_quantization_posterization=true");
				else
					proc_ctrl.set_cmdline(arguments, "--video_png_quantization_posterization=false");

				if (confentity.png.video_quantization_dither_map)
					proc_ctrl.set_cmdline(arguments, "--video_png_quantization_dither_map=true");
				else
					proc_ctrl.set_cmdline(arguments, "--video_png_quantization_dither_map=false");

				if (confentity.png.video_quantization_contrast_maps)
					proc_ctrl.set_cmdline(arguments, "--video_png_quantization_contrast_maps=true");
				else
					proc_ctrl.set_cmdline(arguments, "--video_png_quantization_contrast_maps=false");

				proc_ctrl.set_cmdline(arguments, "--video_png_quantization_colors=%d", confentity.png.video_quantization_colors);
			}
			else if (confentity.video_codec == sirius::base::video_submedia_type_t::webp)
			{
				proc_ctrl.set_cmdline(arguments, "--video_webp_quality=%f", confentity.webp.video_quality);
				proc_ctrl.set_cmdline(arguments, "--video_webp_method=%d", confentity.webp.video_method);
			}

			proc_ctrl.set_cmdline(arguments, "--control_server_portnumber=%d", confentity.controller_portnumber);
			proc_ctrl.set_cmdline(arguments, "--streaming_server_portnumber=%d", confentity.streamer_portnumber);
			proc_ctrl.set_cmdline(arguments, "--id=%d", contenity.id);
			proc_ctrl.set_cmdline(arguments, "--play_after_connect=true");

			if(confentity.invalidate4client)
				proc_ctrl.set_cmdline(arguments, "--enable_invalidate4client=true");
			else
				proc_ctrl.set_cmdline(arguments, "--enable_invalidate4client=false");

			if (confentity.indexed_mode)
				proc_ctrl.set_cmdline(arguments, "--enable_indexed_mode=true");
			else
				proc_ctrl.set_cmdline(arguments, "--enable_indexed_mode=false");

			proc_ctrl.set_cmdline(arguments, "--nthread=%d", confentity.nthread);

			if (confentity.double_reloading_on_creating)
			{
				proc_ctrl.set_cmdline(arguments, "--double_reloading_on_creating=true");
			}
			else
			{
				proc_ctrl.set_cmdline(arguments, "--double_reloading_on_creating=false");
			}

			if (confentity.reloading_on_disconnecting)
			{
				proc_ctrl.set_cmdline(arguments, "--reloading_on_disconnecting=true");
			}
			else
			{
				proc_ctrl.set_cmdline(arguments, "--reloading_on_disconnecting=false");
			}
			/*
			if (confentity.enable_keepalive)
				proc_ctrl.set_cmdline(arguments, "--enable_keepalive=true");
			else
				proc_ctrl.set_cmdline(arguments, "--enable_keepalive=false");
			*/
			proc_ctrl.set_cmdline(arguments, "--enable_keepalive=false");

			proc_ctrl.set_cmdline(arguments, "--keepalive_timeout=%d", confentity.keepalive_timeout);

			if (confentity.enable_tls)
				proc_ctrl.set_cmdline(arguments, "--enable_tls=true");
			else
				proc_ctrl.set_cmdline(arguments, "--enable_tls=false");

			if (confentity.enable_present)
				proc_ctrl.set_cmdline(arguments, "--enable_present=true");
			else
				proc_ctrl.set_cmdline(arguments, "--enable_present=false");

			proc_ctrl.set_cmdline(arguments, "--disable-gpu");
			proc_ctrl.set_cmdline(arguments, "--disable-gpu-compositing");
			//proc_ctrl.set_cmdline(arguments, "--disable-d3d11");
			//proc_ctrl.set_cmdline(arguments, "--disable-gpu-vsync");
			//proc_ctrl.set_cmdline(arguments, "--disable-surfaces");
			proc_ctrl.set_cmdline(arguments, "--off-screen-rendering-enabled");
			proc_ctrl.set_cmdline(arguments, "--off-screen-frame-rate=%d", confentity.video_fps);
			proc_ctrl.set_cmdline(arguments, "--enable-begin-frame-scheduling");
			proc_ctrl.set_cmdline(arguments, "--disable-extensions");
			proc_ctrl.set_cmdline(arguments, "--disable-pdf-extension");
			proc_ctrl.set_cmdline(arguments, "--enable-video-hole");
			proc_ctrl.set_cmdline(arguments, "--disable-web-security");
			proc_ctrl.set_cmdline(arguments, "--ignore-certificate-errors");
			proc_ctrl.set_cmdline(arguments, "--transparent-painting-enabled");
			proc_ctrl.set_cmdline(arguments, "--allow-file-access-from-files");
			//proc_ctrl.set_cmdline(arguments, "--multi-threaded-message-loop");
			proc_ctrl.set_cmdline(arguments, "--disable-plugins-discovery");
			proc_ctrl.set_cmdline(arguments, "--process-per-site");
			proc_ctrl.set_cmdline(arguments, "--js-flags=\"--expose-gc\"");
			//proc_ctrl.set_cmdline(arguments, "--headless");
			//proc_ctrl.set_cmdline(arguments, "--disable-backing-store-limit");
			//proc_ctrl.set_cmdline(arguments, "--show-update-rect");

			if (strlen(confentity.app_session_app) > 0)
			{
				proc_ctrl.set_cmdline(arguments, confentity.app_session_app);
			}

			status = proc_ctrl.fork("..\\attendants\\web\\sirius_web_attendant.exe", "..\\attendants\\web", arguments, &pid);

			if (option)	
				break;
				
			::Sleep(confentity.attendant_creation_delay);
		}		
	}
	return status;
}