#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <jpeglib.h>
#include <string.h>
#include "mjpeg.h"



static void init_source(j_decompress_ptr cinfo)
{

}

static uint8_t EOI_data[2] = { 0xFF, 0xD9 };

static boolean fill_input_buffer(j_decompress_ptr cinfo)
{
    cinfo->src->next_input_byte = EOI_data;
    cinfo->src->bytes_in_buffer = 2;
    return TRUE;
}

static void skip_input_data(j_decompress_ptr cinfo, long num_bytes)
{
    if(num_bytes > 0)
	{
		if(num_bytes > (long)cinfo->src->bytes_in_buffer)
			num_bytes = (long)cinfo->src->bytes_in_buffer;
		cinfo->src->next_input_byte += (size_t)num_bytes;
		cinfo->src->bytes_in_buffer -= (size_t)num_bytes;
	}
}

static void term_source(j_decompress_ptr cinfo)
{

}

static void jpeg_buffer_src(j_decompress_ptr cinfo, unsigned char *buffer, long num)
{
	if(cinfo->src == NULL)
	{
		cinfo->src = (struct jpeg_source_mgr *)(*cinfo->mem->alloc_small)((j_common_ptr)cinfo, JPOOL_PERMANENT,	sizeof(struct jpeg_source_mgr));
	}

	cinfo->src->init_source = init_source;
	cinfo->src->fill_input_buffer = fill_input_buffer;
	cinfo->src->skip_input_data = skip_input_data;
	cinfo->src->resync_to_restart = jpeg_resync_to_restart; /* use default method */
	cinfo->src->term_source = term_source;
	cinfo->src->bytes_in_buffer = num;
	cinfo->src->next_input_byte = (JOCTET *)buffer;
}

static void add_huff_table(j_decompress_ptr dinfo, JHUFF_TBL **htblptr, const UINT8 *bits, const UINT8 *val)
{
	int nsymbols, len;

	if(*htblptr == NULL)
		*htblptr = jpeg_alloc_huff_table((j_common_ptr)dinfo);

	memcpy((*htblptr)->bits, bits, sizeof((*htblptr)->bits));
	nsymbols = 0;

	for(len = 1; len <= 16; len++)
		nsymbols += bits[len];

	memcpy((*htblptr)->huffval, val, nsymbols * sizeof(UINT8));
}

static void std_huff_tables(j_decompress_ptr dinfo)
{
	static const UINT8 bits_dc_luminance[17] = { /* 0-base */0, 0, 1, 5, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0 };
	static const UINT8 val_dc_luminance[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11 };

	static const UINT8 bits_dc_chrominance[17] = { /* 0-base */0, 0, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0 };
	static const UINT8 val_dc_chrominance[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11 };

	static const UINT8 bits_ac_luminance[17] = { /* 0-base */0, 0, 2, 1, 3, 3, 2, 4, 3, 5, 5, 4, 4, 0, 0, 1, 0x7d };
	static const UINT8 val_ac_luminance[] =
	{ 0x01, 0x02, 0x03, 0x00, 0x04, 0x11, 0x05, 0x12, 0x21, 0x31, 0x41, 0x06, 0x13, 0x51, 0x61, 0x07, 0x22, 0x71, 0x14, 0x32, 0x81, 0x91,
			0xa1, 0x08, 0x23, 0x42, 0xb1, 0xc1, 0x15, 0x52, 0xd1, 0xf0, 0x24, 0x33, 0x62, 0x72, 0x82, 0x09, 0x0a, 0x16, 0x17, 0x18,
			0x19, 0x1a, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x43, 0x44, 0x45, 0x46, 0x47,
			0x48, 0x49, 0x4a, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5a, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6a, 0x73,
			0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7a, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8a, 0x92, 0x93, 0x94, 0x95, 0x96,
			0x97, 0x98, 0x99, 0x9a, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7, 0xa8, 0xa9, 0xaa, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7, 0xb8,
			0xb9, 0xba, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda,
			0xe1, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9, 0xea, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8, 0xf9, 0xfa };

	static const UINT8 bits_ac_chrominance[17] =
	{ /* 0-base */0, 0, 2, 1, 2, 4, 4, 3, 4, 7, 5, 4, 4, 0, 1, 2, 0x77 };
	static const UINT8 val_ac_chrominance[] =
	{ 0x00, 0x01, 0x02, 0x03, 0x11, 0x04, 0x05, 0x21, 0x31, 0x06, 0x12, 0x41, 0x51, 0x07, 0x61, 0x71, 0x13, 0x22, 0x32, 0x81, 0x08, 0x14,
			0x42, 0x91, 0xa1, 0xb1, 0xc1, 0x09, 0x23, 0x33, 0x52, 0xf0, 0x15, 0x62, 0x72, 0xd1, 0x0a, 0x16, 0x24, 0x34, 0xe1, 0x25,
			0xf1, 0x17, 0x18, 0x19, 0x1a, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x43, 0x44, 0x45, 0x46,
			0x47, 0x48, 0x49, 0x4a, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5a, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6a,
			0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7a, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8a, 0x92, 0x93, 0x94,
			0x95, 0x96, 0x97, 0x98, 0x99, 0x9a, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7, 0xa8, 0xa9, 0xaa, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6,
			0xb7, 0xb8, 0xb9, 0xba, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6, 0xd7, 0xd8,
			0xd9, 0xda, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9, 0xea, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8, 0xf9, 0xfa };

	add_huff_table(dinfo, &dinfo->dc_huff_tbl_ptrs[0], bits_dc_luminance, val_dc_luminance);
	add_huff_table(dinfo, &dinfo->ac_huff_tbl_ptrs[0], bits_ac_luminance, val_ac_luminance);
	add_huff_table(dinfo, &dinfo->dc_huff_tbl_ptrs[1], bits_dc_chrominance, val_dc_chrominance);
	add_huff_table(dinfo, &dinfo->ac_huff_tbl_ptrs[1], bits_ac_chrominance, val_ac_chrominance);
}


void jpeg_to_raw(unsigned char* input_image, const int input_size, unsigned char* raw_image)
{
	/* these are standard libjpeg structures for reading(decompression) */
	struct jpeg_decompress_struct cinfo;
	struct jpeg_error_mgr jerr;

	/* libjpeg data structure for storing one row, that is, scanline of an image */
	JSAMPROW row_pointer[1];
	unsigned int size;
	unsigned long location = 0;
	int i = 0;

	cinfo.err = jpeg_std_error(&jerr);

	/* setup decompression process and source, then read JPEG header */
	jpeg_create_decompress(&cinfo);

	/* this makes the library read from infile */
	jpeg_buffer_src(&cinfo, input_image, input_size);

	/* reading the image header which contains image information */
	jpeg_read_header(&cinfo, TRUE);
	std_huff_tables(&cinfo);
	jpeg_start_decompress(&cinfo);

	/* allocate memory to hold the uncompressed image */
	size = cinfo.output_width * cinfo.output_height * cinfo.num_components;

	/* now actually read the jpeg into the raw buffer */
	row_pointer[0] = (unsigned char *)malloc(cinfo.output_width * cinfo.num_components);

	/* read one scan line at a time */
	while(cinfo.output_scanline < cinfo.image_height)
	{
		jpeg_read_scanlines(&cinfo, row_pointer, 1);
		for(i = 0; i < cinfo.image_width * cinfo.num_components; i++)
		{
			raw_image[location++] = row_pointer[0][i];
		}
	}

	/* wrap up decompression, destroy objects, free pointers and close open files */
	jpeg_finish_decompress(&cinfo);
	jpeg_destroy_decompress(&cinfo);
	free(row_pointer[0]);
}
