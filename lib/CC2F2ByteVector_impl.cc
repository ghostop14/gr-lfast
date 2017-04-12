/* -*- c++ -*- */
/* 
 * Copyright 2017 <+YOU OR YOUR COMPANY+>.
 * 
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 * 
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this software; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gnuradio/io_signature.h>
#include "CC2F2ByteVector_impl.h"
#include <volk/volk.h>
#include "clSComplex.h"

namespace gr {
  namespace lfast {

    CC2F2ByteVector::sptr
    CC2F2ByteVector::make(int scale,int vecLength,int numVecItems)
    {
      return gnuradio::get_initial_sptr
        (new CC2F2ByteVector_impl(scale, vecLength, numVecItems));
    }

    /*
     * The private constructor
     */
    CC2F2ByteVector_impl::CC2F2ByteVector_impl(int scale,int vecLength,int numVecItems)
      : gr::sync_decimator("CC2F2ByteVector",
              gr::io_signature::make(1, 1, sizeof(gr_complex)*vecLength),
              gr::io_signature::make(1, 1, sizeof(char)*vecLength*numVecItems),
			  numVecItems),d_scale(scale),d_vlen(vecLength)
    {
        const int alignment_multiple =
  	volk_get_alignment() / sizeof(float);
        set_alignment(std::max(1,alignment_multiple));

    	int imaxItems=gr::block::max_noutput_items();
    	if (imaxItems==0)
    		imaxItems=8192;

    	setBufferLength(imaxItems);
    }

    /*
     * Our virtual destructor.
     */
    CC2F2ByteVector_impl::~CC2F2ByteVector_impl()
    {
    	stop();
    }

    bool CC2F2ByteVector_impl::stop() {
		if (floatBuff) {
			delete floatBuff;
			floatBuff = NULL;
		}

		curBufferSize = 0;

     return true;
    }

    void CC2F2ByteVector_impl::setBufferLength(int numItems) {
    	if (floatBuff)
    		delete floatBuff;

    	floatBuff = new float[numItems];
    	curBufferSize = numItems;
    }

    int
    CC2F2ByteVector_impl::work_original(int noutput_items,
        gr_vector_const_void_star &input_items,
        gr_vector_void_star &output_items)
    {
      const gr_complex *in = (const gr_complex *) input_items[0];
      char *out = (char *) output_items[0];
      size_t block_size = output_signature()->sizeof_stream_item (0);

      // Have to adjust the number of items for testing.
      unsigned int noi = noutput_items; // block_size * noutput_items;

      // Complex to real
      float ftmp[noi];

      volk_32fc_deinterleave_real_32f((float *)&ftmp[0], in, noi);

      // float/real to char
      char ctmp[noi];
      volk_32f_s32f_convert_8i((int8_t *)&ctmp[0], &ftmp[0], d_scale, noi);

      // char to stream
//      memcpy (out, &ctmp, noutput_items * block_size);
      memcpy (out, &ctmp, noutput_items);

      return noutput_items;
    }

    int
    CC2F2ByteVector_impl::work_test(int noutput_items,
        gr_vector_const_void_star &input_items,
        gr_vector_void_star &output_items)
    {
    /*
      const SComplex *in = (const SComplex *) input_items[0];
      char *out = (char *) output_items[0];
      size_t block_size = output_signature()->sizeof_stream_item (0);
      int noi = block_size * noutput_items;
      float r;

      // complex to real and float to char in one pass
      for(int i=0; i < noi; i++){
        r = in[i].real * d_scale;
        if(r > max_val)
          r = max_val;
        else if(r < min_val)
          r = min_val;
        out[i] = (int8_t)(r);
      }

      return noutput_items;
    */

	const gr_complex *in = (const gr_complex *) input_items[0];
	char *out = (char *) output_items[0];
	size_t block_size = output_signature()->sizeof_stream_item (0);
	unsigned int noi = noutput_items; // block_size * noutput_items;

	if (noi > curBufferSize) {
		setBufferLength(noi);
	}

	// Complex to real
//	float ftmp[noi];

	volk_32fc_deinterleave_real_32f(floatBuff, in, noi);

	// float/real to char and char to stream
	volk_32f_s32f_convert_8i((int8_t *)out, floatBuff, d_scale, noi);

	return noutput_items;
    }

    int
    CC2F2ByteVector_impl::work(int noutput_items,
        gr_vector_const_void_star &input_items,
        gr_vector_void_star &output_items)
    {
    	const gr_complex *in = (const gr_complex *) input_items[0];
    	char *out = (char *) output_items[0];
    	size_t block_size = output_signature()->sizeof_stream_item (0);
    	unsigned int noi = block_size * noutput_items;

    	if (noi > curBufferSize) {
    		setBufferLength(noi);
    	}

    	// Complex to real
    //	float ftmp[noi];

    	volk_32fc_deinterleave_real_32f(floatBuff, in, noi);

    	// float/real to char and char to stream
    	volk_32f_s32f_convert_8i((int8_t *)out, floatBuff, d_scale, noi);

    	return noutput_items;
    }

  } /* namespace testtiming */
} /* namespace gr */

