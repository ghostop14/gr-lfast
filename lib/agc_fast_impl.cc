/* -*- c++ -*- */
/* 
 * Copyright 2017 ghostop14.
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

#include <gnuradio/io_signature.h>
#include "agc_fast_impl.h"
#include <volk/volk.h>
#include "scomplex.h"

// assisted detection of Fused Multiply Add (FMA) functionality
#if !defined(__FMA__) && defined(__AVX2__)
#define __FMA__ 1
#endif

#if defined(FP_FAST_FMA)
#define __FMA__ 1
#endif

#if defined(__FMA__)
#pragma message "FMA support detected.  Compiling for Fused Multiply/Add support."
#else
#pragma message "No FMA support detected.  Compiling for normal math."
#endif

namespace gr {
namespace lfast {

agc_fast::sptr agc_fast::make(float rate, float reference, float gain)
{
	return gnuradio::make_block_sptr<agc_fast_impl>(rate, reference, gain);
}

/*
 * The private constructor
 */
agc_fast_impl::agc_fast_impl(float rate, float reference, float gain)
: gr::sync_block("agc_fast",
		io_signature::make(1, 1, sizeof(gr_complex)),
		io_signature::make(1, 1, sizeof(gr_complex))),
		kernel::agc_cc(rate, reference, gain, 65536)
{
	const int alignment_multiple =
			volk_get_alignment() / sizeof(gr_complex);
	set_alignment(std::max(1, alignment_multiple));
}

/*
 * Our virtual destructor.
 */
agc_fast_impl::~agc_fast_impl()
{
}

int
agc_fast_impl::work_original(int noutput_items,
		gr_vector_const_void_star &input_items,
		gr_vector_void_star &output_items)
{
	const gr_complex *in = (const gr_complex*)input_items[0];
	gr_complex *out = (gr_complex*)output_items[0];
	scaleN(out, in, noutput_items);
	return noutput_items;
}
int
agc_fast_impl::work_test(int noutput_items,
		gr_vector_const_void_star &input_items,
		gr_vector_void_star &output_items)
{
	const gr_complex *in = (const gr_complex*)input_items[0];
	gr_complex *out = (gr_complex*)output_items[0];
	StructComplex *sOut = (StructComplex*)output_items[0];

	float o_i,o_r;
	/*
      union
      {
        int i;
        float x;
      } u;
	 */
	float val,sqrt_val;
	int *i_ptr;

	for (int i=0;i<noutput_items;i++) {
		out[i] = in[i] * _gain;

		// save on the first, n*scale, and n*(.imag() and .real()) function jumps
		o_i = sOut[i].imag;
		o_r = sOut[i].real;

		// See https://www.codeproject.com/Articles/69941/Best-Square-Root-Method-Algorithm-Function-Precisi
		// for faster square root examples.
		// Tried this one that should be 3x faster but in testing it ran considerably slower.
		/*
    	  val = sqrt_val = o_r*o_r + o_i*o_i;
    	  i_ptr = (int *)&sqrt_val;
		 *i_ptr = (1<<29) + (*i_ptr >> 1) - (1<<22);
    	  sqrt_val = sqrt_val + val/sqrt_val;
    	  sqrt_val = _reference - 0.25f*sqrt_val + val/sqrt_val;

    	  // _gain += _rate * sqrt_val;
		 */

#if defined(__FMA__)
		_gain =  _gain + _rate * (_reference - sqrt(__builtin_fmaf(o_r,o_r,o_i*o_i)));
#else
		_gain =  _gain + _rate * (_reference - sqrt(o_r*o_r + o_i*o_i));
#endif

		if(_max_gain > 0.0 && _gain > _max_gain) {
			_gain = _max_gain;
		}

	}
	return noutput_items;
}

int
agc_fast_impl::work(int noutput_items,
		gr_vector_const_void_star &input_items,
		gr_vector_void_star &output_items)
{
	const gr_complex *in = (const gr_complex*)input_items[0];
	gr_complex *out = (gr_complex*)output_items[0];
	StructComplex *sOut = (StructComplex*)output_items[0];

	float o_i,o_r;
	/*
        union
        {
          int i;
          float x;
        } u;
	 */
	float val,sqrt_val;
	int *i_ptr;

	for (int i=0;i<noutput_items;i++) {
		out[i] = in[i] * _gain;

		// save on the first, n*scale, and n*(.imag() and .real()) function jumps
		o_i = sOut[i].imag;
		o_r = sOut[i].real;

		// See https://www.codeproject.com/Articles/69941/Best-Square-Root-Method-Algorithm-Function-Precisi
		// for faster square root examples.
		// Tried this one that should be 3x faster but in testing it ran considerably slower.
		/*
      	  val = sqrt_val = o_r*o_r + o_i*o_i;
      	  i_ptr = (int *)&sqrt_val;
		 *i_ptr = (1<<29) + (*i_ptr >> 1) - (1<<22);
      	  sqrt_val = sqrt_val + val/sqrt_val;
      	  sqrt_val = _reference - 0.25f*sqrt_val + val/sqrt_val;

      	  // _gain += _rate * sqrt_val;
		 */

#if defined(__FMA__)
		_gain =  _gain + _rate * (_reference - sqrt(__builtin_fmaf(o_r,o_r,o_i*o_i)));
#else
		_gain =  _gain + _rate * (_reference - sqrt(o_r*o_r + o_i*o_i));
#endif

		if(_max_gain > 0.0 && _gain > _max_gain) {
			_gain = _max_gain;
		}

	}
	return noutput_items;
}
} /* namespace lfast */
} /* namespace gr */

