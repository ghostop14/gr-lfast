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
#include "costas4_impl.h"
#include <gnuradio/sincos.h>
#include <gnuradio/expj.h>
#include <gnuradio/math.h>
#include "clSComplex.h"

#define CL_TWO_PI 6.28318530717958647692
#define CL_MINUS_TWO_PI -6.28318530717958647692

namespace gr {
  namespace lfast {

    costas4::sptr
    costas4::make(float loop_bw, int order)
    {
      return gnuradio::get_initial_sptr
        (new costas4_impl(loop_bw, order));
    }

    /*
     * The private constructor
     */
    costas4_impl::costas4_impl(float loop_bw, int order)
      : gr::sync_block("costas4",
              gr::io_signature::make(1, 1, sizeof(gr_complex)),
              gr::io_signature::make(1, 1, sizeof(gr_complex))),
			  blocks::control_loop(loop_bw, 1.0, -1.0),
			  d_order(order), d_error(0), d_noise(1.0), d_phase_detector(NULL)
    {
    	// Only set up for 2nd order right now.
        d_phase_detector = &costas4_impl::phase_detector_4;
/*
        message_port_register_in(pmt::mp("noise"));
        set_msg_handler(
          pmt::mp("noise"),
          boost::bind(&costas4_impl::handle_set_noise,
                      this, _1));
*/
    }

    /*
     * Our virtual destructor.
     */
    costas4_impl::~costas4_impl()
    {
    }

    float
    costas4_impl::error() const
    {
      return d_error;
    }

    void
    costas4_impl::handle_set_noise(pmt::pmt_t msg)
    {
      if(pmt::is_real(msg)) {
        d_noise = pmt::to_double(msg);
        d_noise = powf(10.0f, d_noise/10.0f);
      }
    }

    int
    costas4_impl::work_original(int noutput_items,
			      gr_vector_const_void_star &input_items,
			      gr_vector_void_star &output_items)
    {
      const gr_complex *iptr = (gr_complex *) input_items[0];
      gr_complex *optr = (gr_complex *) output_items[0];
      float *foptr = (float *) output_items[1];

      bool write_foptr = output_items.size() >= 2;

      gr_complex nco_out;
      float i_r,i_i,n_r,n_i;

      std::vector<tag_t> tags;
      /*
      get_tags_in_range(tags, 0, nitems_read(0),
                        nitems_read(0)+noutput_items,
                        pmt::intern("phase_est"));
		*/
      if(write_foptr) {
        for(int i = 0; i < noutput_items; i++) {
          if(tags.size() > 0) {
            if(tags[0].offset-nitems_read(0) == (size_t)i) {
              d_phase = (float)pmt::to_double(tags[0].value);
              tags.erase(tags.begin());
            }
          }
          nco_out = gr_expj(-d_phase);

          optr[i] = iptr[i] * nco_out;

          d_error = phase_detector_4(optr[i]);
          d_error = gr::branchless_clip(d_error, 1.0);

          advance_loop(d_error);
          phase_wrap();
          frequency_limit();

          foptr[i] = d_freq;
        }
      }
      else {
        for(int i = 0; i < noutput_items; i++) {
          if(tags.size() > 0) {
            if(tags[0].offset-nitems_read(0) == (size_t)i) {
              d_phase = (float)pmt::to_double(tags[0].value);
              tags.erase(tags.begin());
            }
          }
          // gr_expj does a sine/cosine
          // EXPENSIVE LINE
          nco_out = gr_expj(-d_phase);

          optr[i] = iptr[i] * nco_out;

          // EXPENSIVE LINE
          d_error = (*this.*d_phase_detector)(optr[i]);
          d_error = gr::branchless_clip(d_error, 1.0);

          advance_loop(d_error);
          phase_wrap();
          frequency_limit();
        }
      }

      return noutput_items;
    }

    int
    costas4_impl::work_test(int noutput_items,
        gr_vector_const_void_star &input_items,
        gr_vector_void_star &output_items)
    {
        // const gr_complex *iptr = (gr_complex *) input_items[0];
        // gr_complex *optr = (gr_complex *) output_items[0];
        const SComplex *iptr = (SComplex *) input_items[0];
        SComplex *optr = (SComplex *) output_items[0];
        // gr_complex nco_out;
        float i_r,i_i,n_r,n_i,o_r,o_i;
        // float x1,x2;
        int i;
        float angle_rad,sin,cos;

        for(i = 0; i < noutput_items; i++) {
          // nco_out = gr_expj(-d_phase);
      	  // returns this:  nco_out.real = n_r, nco_out.imag = n_i
         // Trig functions killing performance.  Tried a number of replacement options but no luck:
        	// gnuradio lookup functions - expensive with float_to_fixed in each iteration
        	// Tried rolling our own lookup tables - Same performance as straight trig
        	// Tried quadratic curve inline approximation - Tiny bit faster
       	 //gr::sincosf(-d_phase, &n_i, &n_r);
          n_i = sinf(-d_phase);
          n_r = cosf(-d_phase);

          //optr[i] = iptr[i] * nco_out;
          i_r = iptr[i].real;
          i_i = iptr[i].imag;
          o_r = (i_r * n_r) - (i_i*n_i);
          o_i = (i_r * n_i) + (i_i * n_r);
          optr[i].real = o_r;
          optr[i].imag = o_i;

          //d_error = (*this.*d_phase_detector)(optr[i]);
          // 4th order in-place
          // d_error = (optr[i].real()>0 ? 1.0 : -1.0) * optr[i].imag() - (optr[i].imag()>0 ? 1.0 : -1.0) * optr[i].real();
          d_error = (o_r>0 ? 1.0 : -1.0) * o_i - (o_i>0 ? 1.0 : -1.0) * o_r;

          // d_error = gr::branchless_clip(d_error, 1.0);
          /*
          x1 = fabsf(d_error+1);
          x2 = fabsf(d_error-1);
          x1 -= x2;
          d_error = 0.5*x1;
		  */
          d_error = 0.5 * (fabsf(d_error+1) - fabsf(d_error-1));

          //advance_loop(d_error);
          d_freq = d_beta * d_error + d_freq;
          //d_freq = __builtin_fmaf(d_beta,d_error,d_freq);
          // This line is causing one of the greatest performance drops!  100 Msps -> 33 Msps!
          d_phase = d_alpha * d_error + d_phase + d_freq;
          // d_phase = d_phase + d_freq + d_alpha * d_error;
          // d_phase = d_phase + __builtin_fmaf(d_alpha,d_error,d_freq);

          //phase_wrap();
          if (d_phase > CL_TWO_PI) {
  			while(d_phase>CL_TWO_PI)
  			  d_phase -= CL_TWO_PI;
          }
          else {
  			while(d_phase< CL_MINUS_TWO_PI)
  			  d_phase += CL_TWO_PI;
          }

          //frequency_limit();
          if(d_freq > d_max_freq)
            d_freq = d_max_freq;
          else if(d_freq < d_min_freq)
            d_freq = d_min_freq;
        }

        return noutput_items;
    }

    int
    costas4_impl::work(int noutput_items,
        gr_vector_const_void_star &input_items,
        gr_vector_void_star &output_items)
    {
        // const gr_complex *iptr = (gr_complex *) input_items[0];
        // gr_complex *optr = (gr_complex *) output_items[0];
        const SComplex *iptr = (SComplex *) input_items[0];
        SComplex *optr = (SComplex *) output_items[0];
        // gr_complex nco_out;
        float i_r,i_i,n_r,n_i,o_r,o_i;
        // float x1,x2;
        int i;
        float angle_rad,sin,cos;

        for(i = 0; i < noutput_items; i++) {
          // nco_out = gr_expj(-d_phase);
      	  // returns this:  nco_out.real = n_r, nco_out.imag = n_i
         // Trig functions killing performance.  Tried a number of replacement options but no luck:
        	// gnuradio lookup functions - expensive with float_to_fixed in each iteration
        	// Tried rolling our own lookup tables - Same performance as straight trig
        	// Tried quadratic curve inline approximation - Tiny bit faster
       	 //gr::sincosf(-d_phase, &n_i, &n_r);
          n_i = sinf(-d_phase);
          n_r = cosf(-d_phase);

          //optr[i] = iptr[i] * nco_out;
          i_r = iptr[i].real;
          i_i = iptr[i].imag;
          o_r = (i_r * n_r) - (i_i*n_i);
          o_i = (i_r * n_i) + (i_i * n_r);
          optr[i].real = o_r;
          optr[i].imag = o_i;

          //d_error = (*this.*d_phase_detector)(optr[i]);
          // 4th order in-place
          // d_error = (optr[i].real()>0 ? 1.0 : -1.0) * optr[i].imag() - (optr[i].imag()>0 ? 1.0 : -1.0) * optr[i].real();
          d_error = (o_r>0 ? 1.0 : -1.0) * o_i - (o_i>0 ? 1.0 : -1.0) * o_r;

          // d_error = gr::branchless_clip(d_error, 1.0);
          /*
          x1 = fabsf(d_error+1);
          x2 = fabsf(d_error-1);
          x1 -= x2;
          d_error = 0.5*x1;
		  */
          d_error = 0.5 * (fabsf(d_error+1) - fabsf(d_error-1));

          //advance_loop(d_error);
          d_freq = d_beta * d_error + d_freq;
          //d_freq = __builtin_fmaf(d_beta,d_error,d_freq);
          // This line is causing one of the greatest performance drops!  100 Msps -> 33 Msps!
          d_phase = d_alpha * d_error + d_phase + d_freq;
          // d_phase = d_phase + d_freq + d_alpha * d_error;
          // d_phase = d_phase + __builtin_fmaf(d_alpha,d_error,d_freq);

          //phase_wrap();
          if (d_phase > CL_TWO_PI) {
  			while(d_phase>CL_TWO_PI)
  			  d_phase -= CL_TWO_PI;
          }
          else {
  			while(d_phase< CL_MINUS_TWO_PI)
  			  d_phase += CL_TWO_PI;
          }

          //frequency_limit();
          if(d_freq > d_max_freq)
            d_freq = d_max_freq;
          else if(d_freq < d_min_freq)
            d_freq = d_min_freq;
        }

        return noutput_items;
    }

    void
    costas4_impl::setup_rpc()
    {
#ifdef GR_CTRLPORT
      // Getters
        rpcbasic_sptr(new rpcbasic_register_get<costas4_cc, float>(
	      alias(), "error",
	      &costas_loop_cc::error,
	      pmt::mp(-2.0f), pmt::mp(2.0f), pmt::mp(0.0f),
	      "", "Error signal of loop", RPC_PRIVLVL_MIN,
            DISPTIME | DISPOPTSTRIP)));
      add_rpc_variable(
          rpcbasic_sptr(new rpcbasic_register_get<control_loop, float>(
	      alias(), "loop_bw",
	      &control_loop::get_loop_bandwidth,
	      pmt::mp(0.0f), pmt::mp(2.0f), pmt::mp(0.0f),
	      "", "Loop bandwidth", RPC_PRIVLVL_MIN,
              DISPTIME | DISPOPTSTRIP)));

      // Setters
      add_rpc_variable(
          rpcbasic_sptr(new rpcbasic_register_set<control_loop, float>(
	      alias(), "loop_bw",
	      &control_loop::set_loop_bandwidth,
	      pmt::mp(0.0f), pmt::mp(1.0f), pmt::mp(0.0f),
	      "", "Loop bandwidth",
	      RPC_PRIVLVL_MIN, DISPNULL)));
#endif /* GR_CTRLPORT */
    }

  } /* namespace lfast */
} /* namespace gr */

