// Copyright 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can
// be found in the LICENSE file.

// Copied from libjpeg version 8.

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

extern "C" {
#include <jerror.h>
#include <jpeglib.h>
}

/*
 * Prepare for input from a supplied memory buffer.
 * The buffer must contain the whole JPEG data.
 */
static void init_mem_source(j_decompress_ptr cinfo) {
  /* no work necessary here */
}

static void skip_input_data(j_decompress_ptr cinfo, long int num_bytes) {
  struct jpeg_source_mgr * src = cinfo->src;

  /* Just a dumb implementation for now.  Could use fseek() except
   * it doesn't work on pipes.  Not clear that being smart is worth
   * any trouble anyway --- large skips are infrequent.
   */
  if (num_bytes > 0) {
    while (num_bytes > src->bytes_in_buffer) {
      num_bytes -= src->bytes_in_buffer;
      (void) (*src->fill_input_buffer) (cinfo);
      /* note we assume that fill_input_buffer will never return FALSE,
       * so suspension need not be handled.
       */
    }
    src->next_input_byte += (size_t) num_bytes;
    src->bytes_in_buffer -= (size_t) num_bytes;
  }
}

static void term_source(j_decompress_ptr cinfo) {
  /* no work necessary here */
}

static boolean fill_mem_input_buffer(j_decompress_ptr cinfo) {
  static JOCTET mybuffer[4];

  /* The whole JPEG data is expected to reside in the supplied memory
   * buffer, so any request for more data beyond the given buffer size
   * is treated as an error.
   */
  WARNMS(cinfo, JWRN_JPEG_EOF);
  /* Insert a fake EOI marker */
  mybuffer[0] = (JOCTET) 0xFF;
  mybuffer[1] = (JOCTET) JPEG_EOI;

  cinfo->src->next_input_byte = mybuffer;
  cinfo->src->bytes_in_buffer = 2;

  return TRUE;
}

void jpeg_mem_src(j_decompress_ptr cinfo,
                  uint8_t* inbuffer,
                  size_t insize) {
  struct jpeg_source_mgr* src;

  if (inbuffer == NULL || insize == 0)  /* Treat empty input as fatal error */
    ERREXIT(cinfo, JERR_INPUT_EMPTY);

  /* The source object is made permanent so that a series of JPEG images
   * can be read from the same buffer by calling jpeg_mem_src only before
   * the first one.
   */
  if (cinfo->src == NULL) {  /* first time for this JPEG object? */
    cinfo->src = static_cast<jpeg_source_mgr*>((*cinfo->mem->alloc_small)
      ((j_common_ptr) cinfo, JPOOL_PERMANENT, sizeof(jpeg_source_mgr)));
  }

  src = cinfo->src;
  src->init_source = init_mem_source;
  src->fill_input_buffer = fill_mem_input_buffer;
  src->skip_input_data = skip_input_data;
  src->resync_to_restart = jpeg_resync_to_restart; /* use default method */
  src->term_source = term_source;
  src->bytes_in_buffer = insize;
  src->next_input_byte = static_cast<JOCTET*>(inbuffer);
}
