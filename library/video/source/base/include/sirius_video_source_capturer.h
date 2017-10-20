#ifndef _SIRIUS_VIDEO_SOURCE_CAPTURER_H_
#define _SIRIUS_VIDEO_SOURCE_CAPTURER_H_

#if defined(WIN32)
#if defined(EXPORT_VIDEO_SOURCE_CAPTURER_LIB)
#define EXP_VIDEO_SOURCE_CAPTURER_CLASS __declspec(dllexport)
#else
#define EXP_VIDEO_SOURCE_CAPTURER_CLASS __declspec(dllimport)
#endif
#else
#include <pthreads.h>
#define EXP_VIDEO_SOURCE_CAPTURER_CLASS
#endif

#include <sirius_video.h>

namespace sirius
{
	namespace library
	{
		namespace video
		{
			namespace source
			{
				class EXP_VIDEO_SOURCE_CAPTURER_CLASS capturer
					: public sirius::library::video::base
				{
				public:
					class EXP_VIDEO_SOURCE_CAPTURER_CLASS handler
					{
					public:
						virtual void on_initialize(void * device, int32_t hwnd, int32_t smt, int32_t width, int32_t height) = 0;
						virtual void on_process(sirius::library::video::source::capturer::entity_t * input) = 0;
						virtual void on_release(void) = 0;
					};

					typedef struct EXP_VIDEO_SOURCE_CAPTURER_CLASS _gpu_description_t
					{
						wchar_t description[128];
						int32_t adaptorIndex;
						uint32_t vendorId;
						uint32_t deviceId;
						uint32_t subsysId;
						uint32_t revision;

						_gpu_description_t(void)
							: adaptorIndex(-1)
							, vendorId(0)
							, deviceId(0)
							, subsysId(0)
							, revision(0)
						{
							memset(description, 0x00, sizeof(description));
						}

						_gpu_description_t(const _gpu_description_t & clone)
						{
							wcsncpy_s(description, clone.description, sizeof(description));
							adaptorIndex = clone.adaptorIndex;
							vendorId = clone.vendorId;
							deviceId = clone.deviceId;
							subsysId = clone.subsysId;
							revision = clone.revision;
						}

						_gpu_description_t operator=(const _gpu_description_t & clone)
						{
							wcsncpy_s(description, clone.description, sizeof(description));
							adaptorIndex = clone.adaptorIndex;
							vendorId = clone.vendorId;
							deviceId = clone.deviceId;
							subsysId = clone.subsysId;
							revision = clone.revision;
							return (*this);
						}
					} gpu_descriton_t;

					typedef struct EXP_VIDEO_SOURCE_CAPTURER_CLASS _context_t
					{
						int32_t width;
						int32_t height;
						int32_t fps;
						int32_t gpuindex;
						int32_t present;
						HWND	hwnd;
						sirius::library::video::source::capturer::handler * handler;
						_context_t(void)
							: width(0)
							, height(0)
							, fps(0)
							, gpuindex(0)
							, present(false)
							, hwnd(NULL)
							, handler(NULL)
						{
						}
					} context_t;

					static int32_t retrieve_gpu_adapters(sirius::library::video::source::capturer::gpu_descriton_t * adapters, int32_t capacity, int32_t & count);
					static int32_t retrieve_gpu_outputs(sirius::library::video::source::capturer::gpu_descriton_t * adapter, int32_t & output);

					capturer(void);
					virtual ~capturer(void);

					virtual int32_t initialize(sirius::library::video::source::capturer::context_t * context);
					virtual int32_t release(void);
					virtual int32_t start(void);
					virtual int32_t stop(void);
					virtual int32_t pause(void);

				private:
					capturer(const sirius::library::video::source::capturer & clone);

				protected:
					sirius::library::video::source::capturer::handler * _handler;
				};
			};
		};
	};
};






#endif
