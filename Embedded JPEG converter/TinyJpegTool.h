#pragma once

/**********************************************************/
//	This is a tiny jpeg tool for embbeded systems.
//
//
//	Yuri R
/**********************************************************/
#include "custom_types.h"
#include "esp_jpg_decode.h"
#include "string.h"
#include "tjpgd.h"

namespace Tools 
{

class TinyJpeg
{
public:

	TinyJpeg(const uint8_t *src, size_t src_len, uint8_t * out, jpg_scale_t scale);

private:

	struct JpegData
	{
		const uint8_t *input;
		uint8_t *output;
		uint16_t width;
		uint16_t height;
		uint16_t data_offset;
		uint32_t Length;
		jpg_scale_t Scale;
	};
#ifdef _WIN32
	static_assert(sizeof(JpegData) == 32,"JpegData has Wrong Size!");
#elif
	static_assert(sizeof(JpegData) == 18,"JpegData has Wrong Size!");
#endif // _WIN32

	JpegData _jpgData;

	bool decodeJpeg();

uint32_t jpg_read(void * arg, size_t index, uint8_t *buf, size_t len);

JRESULT prepare (
	JDEC* jd,			/* Blank decompressor object */
	void* pool,			/* Working buffer for the decompression session */
	uint16_t sz_pool	/* Size of working buffer */
);

};
}