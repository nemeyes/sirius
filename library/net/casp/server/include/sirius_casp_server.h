#ifndef _CAP_CASP_SERVER_H_
#define _CAP_CASP_SERVER_H_

//#include <abstract_stream_server.h>
////#include <windows.h>
//#include <sirius_sicp_server.h>

#include <abstract_stream_server.h>

#define SERVER_UUID		"00000000-0000-0000-0000-000000000000"
#define MAX_VIDEO_ES_SIZE	1024*1024
//#ifdef _DEBUG
//#define ES_WRITE
//#endif
#if defined(EXPORT_CASP_SERVER_LIB)
#define EXP_CASP_SERVER_CLASS __declspec(dllexport)
#else
#define EXP_CASP_SERVER_CLASS __declspec(dllimport)
#endif
//#define CELT_WRITE

namespace sirius
{
	namespace library
	{
		namespace net
		{
			namespace casp
			{
				class EXP_CASP_SERVER_CLASS server
					: public sirius::library::net::stream::server
				{
				public:
					class core;
				public:
					static const int32_t port_number_base = 3400;
#pragma pack(1)
					typedef struct EXP_CASP_SERVER_CLASS _casp_stream_payload_t
					{
						uint8_t contents_count;
						size_t frame_index;
						long long time_stamp;
						size_t contents_length;
					} casp_stream_payload_t;
#pragma pack()

					typedef struct EXP_CASP_SERVER_CLASS _stream_state_t
					{
						static const int32_t state_stopped = 0;
						static const int32_t state_paused = 1;
						static const int32_t state_subscribing = 2;
						static const int32_t state_begin_publishing = 3;
						static const int32_t state_publishing = 4;
					} stream_state_t;

					server(void);
					virtual ~server(void);

					int32_t publish_begin(int32_t vsmt, int32_t asmt, const char * address, int32_t slot_number, const char * uuid, proxy * sc = nullptr);
					int32_t publish_end(void);

					int32_t publish_video(uint8_t * vps, size_t vps_size, uint8_t * sps, size_t sps_size, uint8_t * pps, size_t pps_size, uint8_t * bitstream, size_t nb, long long timestamp);
					int32_t publish_video(int32_t count, int32_t * index, uint8_t ** compressed, int32_t * size, long long timestamp);

					int32_t set_es_stream(uint8_t * data, int32_t stream_type, int32_t data_pos, uint8_t * bitstream, size_t nb);
					int32_t set_sps_pps(int32_t data_pos, uint8_t * vps, size_t vps_size, uint8_t * sps, size_t sps_size, uint8_t * pps, size_t pps_size);
					int32_t set_casp_payload(uint8_t * data, uint8_t stream_type, int32_t & data_pos, uint8_t contents_count, size_t es_header_size, size_t nb, long long timestamp);

				private:
					sirius::library::net::casp::server::core *						_core;
					int32_t								_frame_index_video;
					uint8_t *							_video_data;
					int32_t nsize;
					size_t								_audio_configstr_size;
					uint8_t								_audio_configstr_data[10];

					HANDLE								_video_file;

					int32_t								_state;

#ifdef ES_WRITE
					HANDLE								_pFile;
#endif
#ifdef CELT_WRITE
					HANDLE								_pFile;
#endif
				};
			};
		};
	};
};
#endif