#ifndef _SIRIUS_CIRCULAR_BUFFER_H_
#define _SIRIUS_CIRCULAR_BUFFER_H_

#include <cstdint>
#include <cstring>

namespace sirius
{
	namespace circular
	{
		typedef struct _buffer_t
		{
			uint8_t *	buffer;
			int32_t		length;
			int32_t		start;
			int32_t		available;		//ckoh added
			int32_t		end;
		} buffer_t;

#ifdef __cplusplus
		extern "C"
		{
#endif
			sirius::circular::buffer_t * create(int32_t capacity);
			void	destroy(sirius::circular::buffer_t * buffer);
			int32_t read(sirius::circular::buffer_t * buffer, char * target, int32_t amount);
			int32_t write(sirius::circular::buffer_t * buffer, const char * data, int32_t length);
			int32_t isempty(sirius::circular::buffer_t * buffer);
			int32_t isfull(sirius::circular::buffer_t * buffer);
			int32_t available_data(sirius::circular::buffer_t * buffer);
			int32_t available_space(sirius::circular::buffer_t * buffer);
			//std::string gets(cap_circular_buffer_t * buffer, int amount);

#define	available_data(B) (((B)->end + 1) % (B)->length - (B)->start - 1)
#define	available_space(B) ((B)->length - (B)->end - 1)
#define isfull(B) (sirius::circular::available_data((B)) - (B)->length == 0)
#define isempty(B) (sirius::circular::available_data((B)) == 0)
			//#define puts(B, D) write((B), bdata((D)), blength((D)))
			//#define get_all(B) gets((B), RingBuffer_available_data((B)))
#define starts_at(B) ((B)->buffer + (B)->start)
#define ends_at(B) ((B)->buffer + (B)->end)
#define commit_read(B, A) ((B)->start = ((B)->start + (A)) % (B)->length)
#define commit_write(B, A) ((B)->end = ((B)->end + (A)) % (B)->length)

#ifdef __cplusplus
		};
#endif

	};
};


#endif
