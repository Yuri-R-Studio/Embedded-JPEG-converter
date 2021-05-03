
/**********************************************************/
//	This is a tiny jpeg tool for embbeded systems.
//
//
//	Yuri R
/**********************************************************/

#include "TinyJpegTool.h"
#include "tjpgd.h"

typedef struct {
        jpg_scale_t scale;
        jpg_reader_cb reader;
        jpg_writer_cb writer;
        void * arg;
        size_t len;
        size_t index;
} esp_jpg_decoder_t;

#define TAG "TinyJpeg"

static const char * jd_errors[] = {
    "Succeeded",
    "Interrupted by output function",
    "Device error or wrong termination of input stream",
    "Insufficient memory pool for the image",
    "Insufficient stream input buffer",
    "Parameter error",
    "Data format error",
    "Right format but not supported",
    "Not supported JPEG standard"
};

#define ZIG(n)	Zig[n]

static const uint8_t Zig[64] = {	/* Zigzag-order to raster-order conversion table */
	 0,  1,  8, 16,  9,  2,  3, 10, 17, 24, 32, 25, 18, 11,  4,  5,
	12, 19, 26, 33, 40, 48, 41, 34, 27, 20, 13,  6,  7, 14, 21, 28,
	35, 42, 49, 56, 57, 50, 43, 36, 29, 22, 15, 23, 30, 37, 44, 51,
	58, 59, 52, 45, 38, 31, 39, 46, 53, 60, 61, 54, 47, 55, 62, 63
};

/*-------------------------------------------------*/
/* Input scale factor of Arai algorithm            */
/* (scaled up 16 bits for fixed point operations)  */
/*-------------------------------------------------*/

#define IPSF(n)	Ipsf[n]

static const uint16_t Ipsf[64] = {	/* See also aa_idct.png */
	(uint16_t)(1.00000*8192), (uint16_t)(1.38704*8192), (uint16_t)(1.30656*8192), (uint16_t)(1.17588*8192), (uint16_t)(1.00000*8192), (uint16_t)(0.78570*8192), (uint16_t)(0.54120*8192), (uint16_t)(0.27590*8192),
	(uint16_t)(1.38704*8192), (uint16_t)(1.92388*8192), (uint16_t)(1.81226*8192), (uint16_t)(1.63099*8192), (uint16_t)(1.38704*8192), (uint16_t)(1.08979*8192), (uint16_t)(0.75066*8192), (uint16_t)(0.38268*8192),
	(uint16_t)(1.30656*8192), (uint16_t)(1.81226*8192), (uint16_t)(1.70711*8192), (uint16_t)(1.53636*8192), (uint16_t)(1.30656*8192), (uint16_t)(1.02656*8192), (uint16_t)(0.70711*8192), (uint16_t)(0.36048*8192),
	(uint16_t)(1.17588*8192), (uint16_t)(1.63099*8192), (uint16_t)(1.53636*8192), (uint16_t)(1.38268*8192), (uint16_t)(1.17588*8192), (uint16_t)(0.92388*8192), (uint16_t)(0.63638*8192), (uint16_t)(0.32442*8192),
	(uint16_t)(1.00000*8192), (uint16_t)(1.38704*8192), (uint16_t)(1.30656*8192), (uint16_t)(1.17588*8192), (uint16_t)(1.00000*8192), (uint16_t)(0.78570*8192), (uint16_t)(0.54120*8192), (uint16_t)(0.27590*8192),
	(uint16_t)(0.78570*8192), (uint16_t)(1.08979*8192), (uint16_t)(1.02656*8192), (uint16_t)(0.92388*8192), (uint16_t)(0.78570*8192), (uint16_t)(0.61732*8192), (uint16_t)(0.42522*8192), (uint16_t)(0.21677*8192),
	(uint16_t)(0.54120*8192), (uint16_t)(0.75066*8192), (uint16_t)(0.70711*8192), (uint16_t)(0.63638*8192), (uint16_t)(0.54120*8192), (uint16_t)(0.42522*8192), (uint16_t)(0.29290*8192), (uint16_t)(0.14932*8192),
	(uint16_t)(0.27590*8192), (uint16_t)(0.38268*8192), (uint16_t)(0.36048*8192), (uint16_t)(0.32442*8192), (uint16_t)(0.27590*8192), (uint16_t)(0.21678*8192), (uint16_t)(0.14932*8192), (uint16_t)(0.07612*8192)
};


namespace Tools 
{
TinyJpeg::TinyJpeg(const uint8_t *src, size_t src_len, uint8_t * out, jpg_scale_t scale)
{
    _jpgData.width = 0;
    _jpgData.height = 0;
    _jpgData.input = src;
    _jpgData.output = out;
    _jpgData.Length = src_len;
    _jpgData.data_offset = 0;

    if(decodeJpeg())
    {
        ESP_LOGE(TAG, "JPG Header Parse Failed!");
        assert(0);
    }
    return;
}

uint32_t TinyJpeg::jpg_read(void * arg, size_t index, uint8_t *buf, size_t len)
{
    JpegData * jpeg = (JpegData *)arg;
    if(buf) {
        memcpy(buf, jpeg->input + index, len);
    }
    return len;
}


bool TinyJpeg::decodeJpeg()
{
    uint8_t work[3100];
    JDEC decoder;
    esp_jpg_decoder_t jpeg;

    jpeg.len = _jpgData.Length;
    //jpeg.reader = reader;
    //jpeg.writer = writer;
    //jpeg.arg = arg;
    jpeg.scale = _jpgData.Scale;
    jpeg.index = 0;

    JRESULT jres = prepare(&decoder, work, 3100);
    if(jres != JDR_OK){
        ESP_LOGE(TAG, "JPG Header Parse Failed! %s", jd_errors[jres]);
        return ESP_FAIL;
    }

    //uint16_t output_width = decoder.width / (1 << (uint8_t)(jpeg.scale));
    //uint16_t output_height = decoder.height / (1 << (uint8_t)(jpeg.scale));

    ////output start
    //writer(arg, 0, 0, output_width, output_height, NULL);
    ////output write
    //jres = jd_decomp(&decoder, _jpg_write, (uint8_t)jpeg.scale);
    ////output end
    //writer(arg, output_width, output_height, output_width, output_height, NULL);

    //if (jres != JDR_OK) {
    //    ESP_LOGE(TAG, "JPG Decompression Failed! %s", jd_errors[jres]);
    //    return ESP_FAIL;
    //}
    ////check if all data has been consumed.
    //if (len && jpeg.index < len) {
    //    _jpg_read(&decoder, NULL, len - jpeg.index);
    //}

    return true;
}

/*-----------------------------------------------------------------------*/
/* Analyze the JPEG image and Initialize decompressor object             */
/*-----------------------------------------------------------------------*/

#define	LDB_WORD(ptr)		(uint16_t)(((uint16_t)*((uint8_t*)(ptr))<<8)|(uint16_t)*(uint8_t*)((ptr)+1))

/*-----------------------------------------------------------------------*/
/* Allocate a memory block from memory pool                              */
/*-----------------------------------------------------------------------*/

static void* alloc_pool (	/* Pointer to allocated memory block (NULL:no memory available) */
	JDEC* jd,		/* Pointer to the decompressor object */
	uint16_t nd		/* Number of bytes to allocate */
)
{
	char *rp = 0;


	nd = (nd + 3) & ~3;			/* Align block size to the word boundary */

	if (jd->sz_pool >= nd) {
		jd->sz_pool -= nd;
		rp = (char*)jd->pool;			/* Get start of available memory pool */
		jd->pool = (void*)(rp + nd);	/* Allocate requierd bytes */
	}

	return (void*)rp;	/* Return allocated memory block (NULL:no memory to allocate) */
}

/*-----------------------------------------------------------------------*/
/* Create huffman code tables with a DHT segment                         */
/*-----------------------------------------------------------------------*/

static int create_huffman_tbl (	/* 0:OK, !0:Failed */
	JDEC* jd,					/* Pointer to the decompressor object */
	const uint8_t* data,		/* Pointer to the packed huffman tables */
	uint16_t ndata				/* Size of input data */
)
{
	uint16_t i, j, b, np, cls, num;
	uint8_t d, *pb, *pd;
	uint16_t hc, *ph;


	while (ndata) {	/* Process all tables in the segment */
		if (ndata < 17) return JDR_FMT1;	/* Err: wrong data size */
		ndata -= 17;
		d = *data++;						/* Get table number and class */
		if (d & 0xEE) return JDR_FMT1;		/* Err: invalid class/number */
		cls = d >> 4; num = d & 0x0F;		/* class = dc(0)/ac(1), table number = 0/1 */
		pb = (uint8_t*) alloc_pool(jd, 16);			/* Allocate a memory block for the bit distribution table */
		if (!pb) return JDR_MEM1;			/* Err: not enough memory */
		jd->huffbits[num][cls] = pb;
		for (np = i = 0; i < 16; i++) {		/* Load number of patterns for 1 to 16-bit code */
			np += (pb[i] = *data++);		/* Get sum of code words for each code */
		}
		ph = (uint16_t*) alloc_pool(jd, (uint16_t)(np * sizeof (uint16_t)));/* Allocate a memory block for the code word table */
		if (!ph) return JDR_MEM1;			/* Err: not enough memory */
		jd->huffcode[num][cls] = ph;
		hc = 0;
		for (j = i = 0; i < 16; i++) {		/* Re-build huffman code word table */
			b = pb[i];
			while (b--) ph[j++] = hc++;
			hc <<= 1;
		}

		if (ndata < np) return JDR_FMT1;	/* Err: wrong data size */
		ndata -= np;
		pd = (uint8_t*)alloc_pool(jd, np);			/* Allocate a memory block for the decoded data */
		if (!pd) return JDR_MEM1;			/* Err: not enough memory */
		jd->huffdata[num][cls] = pd;
		for (i = 0; i < np; i++) {			/* Load decoded data corresponds to each code ward */
			d = *data++;
			if (!cls && d > 11) return JDR_FMT1;
			*pd++ = d;
		}
	}

	return JDR_OK;
}

/*-----------------------------------------------------------------------*/
/* Create de-quantization and prescaling tables with a DQT segment       */
/*-----------------------------------------------------------------------*/

static int create_qt_tbl (	/* 0:OK, !0:Failed */
	JDEC* jd,				/* Pointer to the decompressor object */
	const uint8_t* data,	/* Pointer to the quantizer tables */
	uint16_t ndata			/* Size of input data */
)
{
	uint16_t i;
	uint8_t d, z;
	int32_t *pb;


	while (ndata) {	/* Process all tables in the segment */
		if (ndata < 65) return JDR_FMT1;	/* Err: table size is unaligned */
		ndata -= 65;
		d = *data++;							/* Get table property */
		if (d & 0xF0) return JDR_FMT1;			/* Err: not 8-bit resolution */
		i = d & 3;								/* Get table ID */
		pb = (int32_t *) alloc_pool(jd, 64 * sizeof (int32_t));/* Allocate a memory block for the table */
		if (!pb) return JDR_MEM1;				/* Err: not enough memory */
		jd->qttbl[i] = pb;						/* Register the table */
		for (i = 0; i < 64; i++) {				/* Load the table */
			z = ZIG(i);							/* Zigzag-order to raster-order conversion */
			pb[z] = (int32_t)((uint32_t)*data++ * IPSF(z));	/* Apply scale factor of Arai algorithm to the de-quantizers */
		}
	}

	return JDR_OK;
}



JRESULT TinyJpeg::prepare (
	JDEC* jd,			/* Blank decompressor object */
	void* pool,			/* Working buffer for the decompression session */
	uint16_t sz_pool	/* Size of working buffer */
)
{
	uint8_t *seg, b;
	uint16_t marker;
	uint32_t ofs;
	uint16_t n, i, j, len;
	JRESULT rc;


	if (!pool) return JDR_PAR;

	jd->pool = pool;		/* Work memroy */
	jd->sz_pool = sz_pool;	/* Size of given work memory */
	//jd->infunc = infunc;	/* Stream input function */
	jd->nrst = 0;			/* No restart interval (default) */

	for (i = 0; i < 2; i++) {	/* Nulls pointers */
		for (j = 0; j < 2; j++) {
			jd->huffbits[i][j] = 0;
			jd->huffcode[i][j] = 0;
			jd->huffdata[i][j] = 0;
		}
	}
	for (i = 0; i < 4; jd->qttbl[i++] = 0) ;

	jd->inbuf = seg = (uint8_t *)alloc_pool(jd, JD_SZBUF);		/* Allocate stream input buffer */
	if (!seg) return JDR_MEM1;

	if (jd->infunc(jd, seg, 2) != 2) return JDR_INP;/* Check SOI marker */
	if (LDB_WORD(seg) != 0xFFD8) return JDR_FMT1;	/* Err: SOI is not detected */
	ofs = 2;

	for (;;) {
		/* Get a JPEG marker */
		if (jd->infunc(jd, seg, 4) != 4) return JDR_INP;
		marker = LDB_WORD(seg);		/* Marker */
		len = LDB_WORD(seg + 2);	/* Length field */
		if (len <= 2 || (marker >> 8) != 0xFF) return JDR_FMT1;
		len -= 2;		/* Content size excluding length field */
		ofs += 4 + len;	/* Number of bytes loaded */

		switch (marker & 0xFF) {
		case 0xC0:	/* SOF0 (baseline JPEG) */
			/* Load segment data */
			if (len > JD_SZBUF) return JDR_MEM2;
			if (jd->infunc(jd, seg, len) != len) return JDR_INP;

			jd->width = LDB_WORD(seg+3);		/* Image width in unit of pixel */
			jd->height = LDB_WORD(seg+1);		/* Image height in unit of pixel */
			if (seg[5] != 3) return JDR_FMT3;	/* Err: Supports only Y/Cb/Cr format */

			/* Check three image components */
			for (i = 0; i < 3; i++) {
				b = seg[7 + 3 * i];							/* Get sampling factor */
				if (!i) {	/* Y component */
					if (b != 0x11 && b != 0x22 && b != 0x21) {	/* Check sampling factor */
						return JDR_FMT3;					/* Err: Supports only 4:4:4, 4:2:0 or 4:2:2 */
					}
					jd->msx = b >> 4; jd->msy = b & 15;		/* Size of MCU [blocks] */
				} else {	/* Cb/Cr component */
					if (b != 0x11) return JDR_FMT3;			/* Err: Sampling factor of Cr/Cb must be 1 */
				}
				b = seg[8 + 3 * i];							/* Get dequantizer table ID for this component */
				if (b > 3) return JDR_FMT3;					/* Err: Invalid ID */
				jd->qtid[i] = b;
			}
			break;

		case 0xDD:	/* DRI */
			/* Load segment data */
			if (len > JD_SZBUF) return JDR_MEM2;
			if (jd->infunc(jd, seg, len) != len) return JDR_INP;

			/* Get restart interval (MCUs) */
			jd->nrst = LDB_WORD(seg);
			break;

		case 0xC4:	/* DHT */
			/* Load segment data */
			if (len > JD_SZBUF) return JDR_MEM2;
			if (jd->infunc(jd, seg, len) != len) return JDR_INP;

			/* Create huffman tables */
			rc = (JRESULT)create_huffman_tbl(jd, seg, len);
			if (rc) return rc;
			break;

		case 0xDB:	/* DQT */
			/* Load segment data */
			if (len > JD_SZBUF) return JDR_MEM2;
			if (jd->infunc(jd, seg, len) != len) return JDR_INP;

			/* Create de-quantizer tables */
			rc = (JRESULT)create_qt_tbl(jd, seg, len);
			if (rc) return rc;
			break;

		case 0xDA:	/* SOS */
			/* Load segment data */
			if (len > JD_SZBUF) return JDR_MEM2;
			if (jd->infunc(jd, seg, len) != len) return JDR_INP;

			if (!jd->width || !jd->height) return JDR_FMT1;	/* Err: Invalid image size */

			if (seg[0] != 3) return JDR_FMT3;				/* Err: Supports only three color components format */

			/* Check if all tables corresponding to each components have been loaded */
			for (i = 0; i < 3; i++) {
				b = seg[2 + 2 * i];	/* Get huffman table ID */
				if (b != 0x00 && b != 0x11)	return JDR_FMT3;	/* Err: Different table number for DC/AC element */
				b = i ? 1 : 0;
				if (!jd->huffbits[b][0] || !jd->huffbits[b][1]) {	/* Check dc/ac huffman table for this component */
					return JDR_FMT1;					/* Err: Nnot loaded */
				}
				if (!jd->qttbl[jd->qtid[i]]) {			/* Check dequantizer table for this component */
					return JDR_FMT1;					/* Err: Not loaded */
				}
			}

			/* Allocate working buffer for MCU and RGB */
			n = jd->msy * jd->msx;						/* Number of Y blocks in the MCU */
			if (!n) return JDR_FMT1;					/* Err: SOF0 has not been loaded */
			len = n * 64 * 2 + 64;						/* Allocate buffer for IDCT and RGB output */ // <--- Fix Me Yuri! Here where the ESP32 doesn't have memory
			if (len < 256) len = 256;					/* but at least 256 byte is required for IDCT */
			jd->workbuf = alloc_pool(jd, len);			/* and it may occupy a part of following MCU working buffer for RGB output */
			if (!jd->workbuf) return JDR_MEM1;			/* Err: not enough memory */
			jd->mcubuf = (uint8_t*)alloc_pool(jd, (uint16_t)((n + 2) * 64));	/* Allocate MCU working buffer */
			if (!jd->mcubuf) return JDR_MEM1;			/* Err: not enough memory */

			/* Pre-load the JPEG data to extract it from the bit stream */
			jd->dptr = seg; jd->dctr = 0; jd->dmsk = 0;	/* Prepare to read bit stream */
			if (ofs %= JD_SZBUF) {						/* Align read offset to JD_SZBUF */
				jd->dctr = jd->infunc(jd, seg + ofs, (uint16_t)(JD_SZBUF - ofs));
				jd->dptr = seg + ofs - 1;
			}

			return JDR_OK;		/* Initialization succeeded. Ready to decompress the JPEG image. */

		case 0xC1:	/* SOF1 */
		case 0xC2:	/* SOF2 */
		case 0xC3:	/* SOF3 */
		case 0xC5:	/* SOF5 */
		case 0xC6:	/* SOF6 */
		case 0xC7:	/* SOF7 */
		case 0xC9:	/* SOF9 */
		case 0xCA:	/* SOF10 */
		case 0xCB:	/* SOF11 */
		case 0xCD:	/* SOF13 */
		case 0xCE:	/* SOF14 */
		case 0xCF:	/* SOF15 */
		case 0xD9:	/* EOI */
			return JDR_FMT3;	/* Unsuppoted JPEG standard (may be progressive JPEG) */

		default:	/* Unknown segment (comment, exif or etc..) */
			/* Skip segment data */
			if (jd->infunc(jd, 0, len) != len) {	/* Null pointer specifies to skip bytes of stream */
				return JDR_INP;
			}
		}
	}
}

}