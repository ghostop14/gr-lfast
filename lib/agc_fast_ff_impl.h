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

#ifndef INCLUDED_LFAST_AGC_FAST_FF_IMPL_H
#define INCLUDED_LFAST_AGC_FAST_FF_IMPL_H

#include <lfast/agc_fast_ff.h>
#include <gnuradio/analog/agc.h>

namespace gr {
  namespace lfast {

    class agc_fast_ff_impl : public agc_fast_ff,gr::analog::kernel::agc_ff
    {
     private:
      // Nothing to declare in this block.

     public:
      agc_fast_ff_impl(float rate = 1e-4, float reference = 1.0,
    		  float gain = 1.0);
      ~agc_fast_ff_impl();

      float rate() const { return gr::analog::kernel::agc_ff::rate(); }
      float reference() const { return gr::analog::kernel::agc_ff::reference(); }
      float gain() const { return gr::analog::kernel::agc_ff::gain(); }
      float max_gain() const { return gr::analog::kernel::agc_ff::max_gain(); }

      void set_rate(float rate) { gr::analog::kernel::agc_ff::set_rate(rate); }
      void set_reference(float reference) { gr::analog::kernel::agc_ff::set_reference(reference); }
      void set_gain(float gain) { gr::analog::kernel::agc_ff::set_gain(gain); }
      virtual void set_max_gain(float max_gain) { gr::analog::kernel::agc_ff::set_max_gain(max_gain); }

      // Where all the action really happens
      int work(int noutput_items,
         gr_vector_const_void_star &input_items,
         gr_vector_void_star &output_items);

      int work_original(int noutput_items,
	       gr_vector_const_void_star &input_items,
	       gr_vector_void_star &output_items);
      int work_test(int noutput_items,
	       gr_vector_const_void_star &input_items,
	       gr_vector_void_star &output_items);
    };

  } // namespace lfast
} // namespace gr

#endif /* INCLUDED_LFAST_AGC_FAST_IMPL_H */

