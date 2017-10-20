#include <windows.h>
#include <assert.h>
#include <cstdlib>
#include <cstdio>
#include <string>

#include <sirius_circular_buffer.h>

sirius::circular::buffer_t * sirius::circular::create(int32_t length)
{
	sirius::circular::buffer_t * buffer = static_cast<sirius::circular::buffer_t*>(calloc(1, sizeof(sirius::circular::buffer_t)));
	buffer->length = length;
	buffer->available = length;
	buffer->start = 0;
	buffer->end = 0;
	buffer->buffer = static_cast<uint8_t*>(calloc(buffer->length, 1));
	return buffer;
}

void sirius::circular::destroy(sirius::circular::buffer_t * buffer)
{
	if (buffer)
	{
		free(buffer->buffer);
		free(buffer);
	}
}

int32_t sirius::circular::write(sirius::circular::buffer_t * buffer, const uint8_t * data, int32_t length)
{
	if (buffer->available < length)
		return -1;
	int copy_size = 0;
	if (buffer->length - buffer->end < length)
		copy_size = buffer->length - buffer->end;
	else
		copy_size = length;
	memcpy(buffer->buffer + buffer->end, data, copy_size);
	buffer->end += copy_size;
	if (copy_size < length)
	{
		memcpy(buffer->buffer, data + copy_size, length - copy_size);
		buffer->end = length - copy_size;
	}
	buffer->available -= length;
	return length;
}

int32_t sirius::circular::read(sirius::circular::buffer_t * buffer, uint8_t * target, int32_t length)
{
	if (buffer->length - buffer->available < length)
		return -1;
	if (target)
	{
		if (buffer->start < buffer->end)
		{
			memcpy(target, buffer->buffer + buffer->start, length);
		}
		else
		{
			if ((buffer->length - buffer->start) < length)
			{
				memcpy(target, buffer->buffer + buffer->start, buffer->length - buffer->start);
				memcpy(target + (buffer->length - buffer->start), buffer->buffer, length - buffer->length + buffer->start);
			}
			else
			{
				memcpy(target, buffer->buffer + buffer->start, length);
			}
		}
	}
	buffer->start = (buffer->start + length) % buffer->length;
	buffer->available += length;
	return length;
}
