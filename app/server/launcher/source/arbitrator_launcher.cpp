#include <configuration_dao.h>
#include <configuration_entity.h>
#include <attendant_dao.h>
#include <attendant_entity.h>
#include <process_controller.h>
#include <sirius_d3d11_device_stat.h>

#define ARGUMENT_SIZE	1024
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

	char db_path[MAX_PATH];
	retrieve_db_path(db_path);

	int32_t hw_gpu_desc_cnt = 0;
	sirius::library::video::device::d3d11::stat::desc_t hw_gpu_desc[MAX_GPU_COUNT];
	sirius::library::video::device::d3d11::stat::retreieve(hw_gpu_desc, MAX_GPU_COUNT, hw_gpu_desc_cnt, sirius::library::video::device::d3d11::stat::option_t::hw);


	int32_t status = sirius::base::err_code_t::fail;
	sirius::app::server::arbitrator::entity::configuration_t confentity;
	{
		sirius::app::server::arbitrator::db::configuration_dao confdao(db_path);
		status = confdao.retrieve(&confentity);
	}

	if (status == sirius::base::err_code_t::success)
	{

#if !defined(WITH_COORDINATOR_ONLY_DEBUG)
		sirius::app::server::arbitrator::db::attendant_dao contdao(db_path);
		contdao.remove();
		for (int32_t index = 0; index < confentity.max_attendant_instance; index++)
		{
			unsigned long pid = 0;
			char arguments[ARGUMENT_SIZE] = { 0 };
			sirius::uuid uuidgen;
			uuidgen.create();

			sirius::app::server::arbitrator::entity::attendant_t contenity;
			memmove(contenity.uuid, uuidgen.c_str(), strlen(uuidgen.c_str()) + 1);
			memmove(contenity.client_uuid, UNDEFINED_UUID, strlen(UNDEFINED_UUID) + 1);
			contenity.id = index;
			memset(contenity.client_id, 0x00, sizeof(contenity.client_id));
			contenity.state = attendant_state_t::idle;
			contenity.total_bandwidth_bytes = 0;

			sirius::app::server::arbitrator::process::controller proc_ctrl;
			proc_ctrl.set_cmdline(arguments, "--reconnect=false");
			proc_ctrl.set_cmdline(arguments, "--uuid=\"%s\"", contenity.uuid);
			proc_ctrl.set_cmdline(arguments, "--attendant_type=\"web\"");
			proc_ctrl.set_cmdline(arguments, "--url=\"%s\"", confentity.url);
			switch (confentity.video_codec)
			{
			case sirius::base::video_submedia_type_t::png:
				proc_ctrl.set_cmdline(arguments, "--video_codec=\"png\"");
				break;
			}
			proc_ctrl.set_cmdline(arguments, "--video_width=%d", confentity.video_width);
			proc_ctrl.set_cmdline(arguments, "--video_height=%d", confentity.video_height);
			proc_ctrl.set_cmdline(arguments, "--video_fps=%d", confentity.video_fps);
			proc_ctrl.set_cmdline(arguments, "--video_block_width=%d", confentity.video_block_width);
			proc_ctrl.set_cmdline(arguments, "--video_block_height=%d", confentity.video_block_height);
			proc_ctrl.set_cmdline(arguments, "--video_compression_level=%d", confentity.video_compression_level);
			proc_ctrl.set_cmdline(arguments, "--video_quantization_colors=%d", confentity.video_quantization_colors);
			proc_ctrl.set_cmdline(arguments, "--control_server_portnumber=%d", confentity.portnumber);
			proc_ctrl.set_cmdline(arguments, "--id=%d", contenity.id);
			proc_ctrl.set_cmdline(arguments, "--play_after_connect=true");

			if (confentity.enable_present)
				proc_ctrl.set_cmdline(arguments, "--enable_present=true");
			else
				proc_ctrl.set_cmdline(arguments, "--enable_present=false");

			if (confentity.enable_gpu && (hw_gpu_desc_cnt > 0))
			{
				proc_ctrl.set_cmdline(arguments, "--gpu_index=%d", index%hw_gpu_desc_cnt);
				proc_ctrl.set_cmdline(arguments, "--ignore-gpu-blacklist");
				proc_ctrl.set_cmdline(arguments, "--force-compositing-mode");
				proc_ctrl.set_cmdline(arguments, "--enable-gpu");
				proc_ctrl.set_cmdline(arguments, "--multi-gpu");
			}
			else
			{
				proc_ctrl.set_cmdline(arguments, "--gpu_index=%d", 0);
				proc_ctrl.set_cmdline(arguments, "--disable-gpu");
				proc_ctrl.set_cmdline(arguments, "--disable-gpu-compositing");
				proc_ctrl.set_cmdline(arguments, "--disable-d3d11");
				proc_ctrl.set_cmdline(arguments, "--disable-surfaces");
				proc_ctrl.set_cmdline(arguments, "--off-screen-rendering-enabled");
				proc_ctrl.set_cmdline(arguments, "--off-screen-frame-rate=%d", 5);
			}

			proc_ctrl.set_cmdline(arguments, "--enable-begin-frame-scheduling");
			proc_ctrl.set_cmdline(arguments, "--disable-extensions");
			proc_ctrl.set_cmdline(arguments, "--disable-pdf-extension");
			proc_ctrl.set_cmdline(arguments, "--enable-video-hole");

			status = proc_ctrl.fork("..\\attendants\\web\\sirius_web_attendant.exe", "..\\attendants\\web", arguments, &pid);
			if (status == sirius::base::err_code_t::success)
			{
				contenity.pid = pid;
				contdao.add(&contenity);
			}

			::Sleep(confentity.attendant_creation_delay);
		}
#endif
	}
	return status;
}