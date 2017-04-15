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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gnuradio/io_signature.h>
#include "agc_fast_ff_impl.h"
#include <volk/volk.h>

namespace gr {
  namespace lfast {

    agc_fast_ff::sptr
    agc_fast_ff::make(float rate, float reference, float gain)
    {
      return gnuradio::get_initial_sptr
        (new agc_fast_ff_impl(rate, reference, gain));
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
      const float *in = (const float*)input_items[0];
      float *out = (float*)output_items[0];
      scaleN(out, in, noutput_items);
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

    	  // _gain += (_reference - fabsf (out[i])) * _rate;

    	  if (out[i] >= 0)
    		  _gain += (_reference - out[i]) * _rate;
    	  else
    		  // fabs replacement
    		  _gain += (_reference + out[i]) * _rate;

    	  if(_max_gain > 0.0 && _gain > _max_gain)
    	    _gain = _max_gain;

      }
      return noutput_items;
    }
  } /* namespace lfast */
} /* namespace gr */

