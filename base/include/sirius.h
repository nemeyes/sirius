#ifndef _SIRIUS_H_
#define _SIRIUS_H_

#include <cstdint>

#if defined(WIN32) || defined(WIN64)
#include <windows.h>
#define SIRIUS_SAFE_DELETE_ARRAY(P)                {if (P) {delete[] P; P = NULL;}}
#define SIRIUS_SAFE_RELEASE(X)                     {if (X) { X->Release(); X = NULL; }}
#define SIRIUS_SAFE_FREE(X)                        {if (X) { free(X); X = NULL; }}
#define SIRIUS_SAFE_DELETE(P)                      {if (P) {delete P; P = NULL;}}

#if defined(WIN64)
typedef unsigned __int64 UPARAM;
#elif defined(WIN32)
typedef unsigned long UPARAM;
#endif
#else

#endif

namespace sirius
{
	class base
	{
	public:
		typedef struct _err_code_t
		{
			static const int32_t success = 0;
			static const int32_t fail = 1;
			
			//pngqaunt
			static const int32_t missing_argument = 2;
			static const int32_t read_error = 3;
			static const int32_t invalid_argument = 4;
			static const int32_t not_overwriting_error = 15;
			static const int32_t cant_write_error = 16;
			static const int32_t out_of_memory_error = 17;
			static const int32_t wrong_architecture = 18;
			static const int32_t png_out_of_memory_error = 24;
			static const int32_t libpng_fatal_error = 25;
			static const int32_t wrong_input_color_type = 26;
			static const int32_t libpng_init_error = 35;
			static const int32_t too_large_file = 98;
			static const int32_t too_low_quality = 99;

			static const int32_t expected = 100;

			static const int32_t slot_full = 1201;
			static const int32_t Json_parser_fail = 1202;
			static const int32_t slot_launcher_fail = 1203;
			static const int32_t invaild_appid = 1204;
			static const int32_t duplicate_uuid = 1205;
			static const int32_t duplicate_device_id = 1206;
			static const int32_t invalid_device_type = 1207;
			static const int32_t invalid_parameter = 1208;
			static const int32_t unsupported_resolution = 1209;
			static const int32_t not_implemented = 1301;
			static const int32_t socket_error = 1401;
			static const int32_t max_es_size_over = 1402;
			static const int32_t slot_count_danger_mn = 1403;
			static const int32_t slot_count_danger_mg = 1404;
			static const int32_t slot_count_danger_ct = 1405;
			static const int32_t gpu_load_danger_mg = 1406;
			static const int32_t gpu_load_danger_ct = 1407;
			static const int32_t gpu_error_detection = 1408;
			static const int32_t invalid_file_path = 1409;
			static const int32_t unsupported_media_file = 1410;
			static const int32_t invalid_media_file = 1411;
			static const int32_t corrupted_media_file = 1412;
			static const int32_t not_found_sirius_ini = 1501;
			static const int32_t not_found_applist_json = 1502;
			static const int32_t not_found_sirius_ip = 1503;
			static const int32_t port_is_already_use = 1504;



		} err_code_t;

		typedef struct _media_type_t
		{
			static const int32_t unknown = 0x00;
			static const int32_t video = 0x01;
			static const int32_t audio = 0x03;
		} media_type_t;

		typedef struct _video_submedia_type_t
		{
			static const int32_t unknown = -1;
			static const int32_t rgb32 = 0;
			static const int32_t rgb24 = 1;
			static const int32_t yuy2 = 2;
			static const int32_t i420 = 3;
			static const int32_t yv12 = 4;
			static const int32_t nv12 = 5;
			static const int32_t jpeg = 6;
			static const int32_t png = 7;
			static const int32_t dxt = 8;
			static const int32_t max_video_submedia_count = dxt;
		} video_submedia_type_t;

		typedef struct _video_memory_type_t
		{
			static const int32_t host = 0;
			static const int32_t d3d9 = 1;
			static const int32_t d3d10 = 2;
			static const int32_t d3d11 = 3;
			static const int32_t d3d12 = 4;
			static const int32_t opengl = 5;
			static const int32_t opencl = 6;
		} video_memory_type_t;

		typedef struct _render_type_t
		{
			static const int32_t stretch = 0;
			static const int32_t original = 1;
		} render_type_t;

		typedef struct _ethernet_type_t
		{
			static const int32_t tcp = 0;
			static const int32_t udp = 1;
		} ethernet_type_t;

		typedef struct _attendant_type_t
		{
			static const int32_t desktop = 0;
			static const int32_t web = 1;
		} attendant_type_t;

		typedef struct _client_device_type_t
		{
			static const int32_t unknown = 0;
			static const int32_t settop = 1;
			static const int32_t mobile = 2;
		} client_device_type_t;

		typedef struct _client_environment_type_t
		{
			static const int32_t unknown = 0;
			static const int32_t android = 1;
			static const int32_t ios = 2;
			static const int32_t native = 3;
		} client_environment_type_t;
	};
};



#endif