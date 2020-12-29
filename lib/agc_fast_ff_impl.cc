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
#include "agc_fast_ff_impl.h"
#include <volk/volk.h>

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

agc_fast_ff::sptr agc_fast_ff::make(float rate, float reference, float gain)
{
	return gnuradio::make_block_sptr<agc_fast_ff_impl>(rate, reference, gain);
}

/*
 * The private constructor
 */
agc_fast_ff_impl::agc_fast_ff_impl(float rate, float reference, float gain)
: gr::sync_block("agc_fast_ff",
		io_signature::make(1, 1, sizeof(float)),
		io_signature::make(1, 1, sizeof(float))),
		gr::analog::kernel::agc_ff(rate, reference, gain, 65536)
{
	const int alignment_multiple =
			volk_get_alignment() / sizeof(float);
	set_alignment(std::max(1, alignment_multiple));
}

/*
 * Our virtual destructor.
 */
agc_fast_ff_impl::~agc_fast_ff_impl()
{
}

int
agc_fast_ff_impl::work(int noutput_items,
		gr_vector_const_void_star &input_items,
		gr_vector_void_star &output_items)
{
	const float *in = (const float *)input_items[0];
	float *out = (float *)output_items[0];

	for (int i=0;i<noutput_items;i++) {
		out[i] = in[i] * _gain;

#if defined(__FMA__)
      			  _gain = __builtin_fmaf((_reference - std::abs(out[i])),_rate,_gain);
      			  // _gain = (_reference - std::abs(out[i])) * _rate + _gain;
#else
      			  _gain = (_reference - std::abs(out[i])) * _rate + _gain;
#endif

      			  if(_max_gain > 0.0 && _gain > _max_gain)
      				  _gain = _max_gain;

	}
	return noutput_items;
}

int
agc_fast_ff_impl::work_original(int noutput_items,
		gr_vector_const_void_star &input_items,
		gr_vector_void_star &output_items)
{
	const float *in = (const float *)input_items[0];
	float *out = (float *)output_items[0];
	scaleN(out, in, noutput_items);
	return noutput_items;
}
int
agc_fast_ff_impl::work_test(int noutput_items,
		gr_vector_const_void_star &input_items,
		gr_vector_void_star &output_items)
{
	const float *in = (const float *)input_items[0];
	float *out = (float *)output_items[0];

	for (int i=0;i<noutput_items;i++) {
		out[i] = in[i] * _gain;

#if defined(__FMA__)
    			  _gain = __builtin_fmaf((_reference - std::abs(out[i])),_rate,_gain);
    			  // _gain = (_reference - std::abs(out[i])) * _rate + _gain;
#else
    			  _gain = (_reference - std::abs(out[i])) * _rate + _gain;
#endif

    			  if(_max_gain > 0.0 && _gain > _max_gain)
    				  _gain = _max_gain;

	}
	return noutput_items;
}
} /* namespace lfast */
} /* namespace gr */

