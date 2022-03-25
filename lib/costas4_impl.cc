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
#include "costas4_impl.h"
#include <gnuradio/sincos.h>
#include <gnuradio/expj.h>
#include <gnuradio/math.h>
#include "clSComplex.h"

#define CL_TWO_PI 6.28318530717958647692
#define CL_ONE_OVER_2PI 0.15915494309189533577
#define CL_MINUS_TWO_PI -6.28318530717958647692

// assisted detection of Fused Multiply Add (FMA) functionality
#if !defined(__FMA__) && defined(__AVX2__)
#define __FMA__ 1
#endif

#if defined(FP_FAST_FMA)
#define __FMA__ 1
#endif

/*
#if defined(__FMA__)
#pragma message "FMA support detected.  Compiling for Fused Multiply/Add support."
#else
#pragma message "No FMA support detected.  Compiling for normal math."
#endif
 */

namespace gr {
namespace lfast {

costas4::sptr costas4::make(float loop_bw, int order, bool genPDUs)
{
	return gnuradio::make_block_sptr<costas4_impl>(loop_bw, order, genPDUs);
}

/*
 * The private constructor
 */
costas4_impl::costas4_impl(float loop_bw, int order, bool genPDUs)
: gr::sync_block("costas4",
		gr::io_signature::make(1, 1, sizeof(gr_complex)),
		gr::io_signature::make(1, 1, sizeof(gr_complex))),
		blocks::control_loop(loop_bw, 1.0, -1.0),
		d_order(order), d_error(0), d_noise(1.0), d_phase_detector(NULL)
{
	d_genSignalPDUs = genPDUs;
	// Only set up for 2nd order right now.
	d_phase_detector = &costas4_impl::phase_detector_4;
	/*
        message_port_register_in(pmt::mp("noise"));
        set_msg_handler(
          pmt::mp("noise"),
          boost::bind(&costas4_impl::handle_set_noise,
                      this, _1));
	 */
	message_port_register_in(pmt::mp("msgin"));
	set_msg_handler(pmt::mp("msgin"), [this](pmt::pmt_t msg) { this->handleMsgIn(msg); });

	message_port_register_out(pmt::mp("msgout"));
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

void costas4_impl::handleMsgIn(pmt::pmt_t msg) {
	if (!d_genSignalPDUs)
		return;

	pmt::pmt_t inputMetadata = pmt::car(msg);
	pmt::pmt_t data = pmt::cdr(msg);
	size_t noutput_items = pmt::length(data);
	const gr_complex *cc_samples;

	cc_samples = pmt::c32vector_elements(data,noutput_items);

	gr_complex out[noutput_items];
	std::vector<const void *> items_in;
	std::vector<void *> items_out;
	items_out.push_back(&out[0]);
	items_in.push_back(&cc_samples[0]);

	int retVal = work_test(noutput_items,items_in,items_out);

	pmt::pmt_t data_out(pmt::init_c32vector(noutput_items, &out[0]));
	pmt::pmt_t pdu = pmt::cons( inputMetadata, data_out );
	message_port_pub(pmt::mp("msgout"),pdu);
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
		if ((d_phase > CL_TWO_PI) || (d_phase < CL_MINUS_TWO_PI)) {
			// d_phase = d_phase / CL_TWO_PI - (float)((int)(d_phase / CL_TWO_PI));
			// switch to multiplication for faster op
#if defined(__FMA__)
			d_phase = __builtin_fmaf(d_phase,CL_ONE_OVER_2PI,-(float)((int)(d_phase * CL_ONE_OVER_2PI)));
#else
			d_phase = d_phase * CL_ONE_OVER_2PI - (float)((int)(d_phase * CL_ONE_OVER_2PI));
#endif
			d_phase = d_phase * CL_TWO_PI;
		}
		n_i = sinf(-d_phase);
		n_r = cosf(-d_phase);

		//optr[i] = iptr[i] * nco_out;
		// FMA stands for fused multiply-add operations where FMA(a,b,c)=(a*b)+c and it does it as a single operation.
#if defined(__FMA__)
		o_r = __builtin_fmaf(iptr[i].real,n_r,-iptr[i].imag*n_i);
		o_i = __builtin_fmaf(iptr[i].real,n_i,iptr[i].imag*n_r);
#else
		i_r = iptr[i].real;
		i_i = iptr[i].imag;
		o_r = (i_r * n_r) - (i_i*n_i);
		o_i = (i_r * n_i) + (i_i * n_r);
#endif
		optr[i].real = o_r;
		optr[i].imag = o_i;

		//d_error = (*this.*d_phase_detector)(optr[i]);
		// 4th order in-place
		// d_error = (optr[i].real()>0 ? 1.0 : -1.0) * optr[i].imag() - (optr[i].imag()>0 ? 1.0 : -1.0) * optr[i].real();
		d_error = (o_r>0 ? 1.0 : -1.0) * o_i - (o_i>0 ? 1.0 : -1.0) * o_r;

		// d_error = gr::branchless_clip(d_error, 1.0);
		/*  Taken out for speed and consolidated
          x1 = fabsf(d_error+1);
          x2 = fabsf(d_error-1);
          x1 -= x2;
          d_error = 0.5*x1;
		 */
		// d_error = 0.5 * (fabsf(d_error+1) - fabsf(d_error-1));
		d_error = 0.5 * (std::abs(d_error+1) - std::abs(d_error-1));

		//advance_loop(d_error);
#if defined(__FMA__)
		d_freq = __builtin_fmaf(d_beta,d_error,d_freq);
#else
		d_freq = d_beta * d_error + d_freq;
#endif
		//d_freq = __builtin_fmaf(d_beta,d_error,d_freq);
		// This line is causing one of the greatest performance drops!  100 Msps -> 33 Msps!
#if defined(__FMA__)
		d_phase = d_phase + __builtin_fmaf(d_alpha,d_error,d_freq);
#else
		d_phase = d_phase + d_alpha * d_error + d_freq;
#endif
		// d_phase = d_phase + d_freq + d_alpha * d_error;
		// d_phase = d_phase + __builtin_fmaf(d_alpha,d_error,d_freq);

		//phase_wrap();
		// Moved up top

		/*
            if (d_phase > CL_TWO_PI) {
    			while(d_phase>CL_TWO_PI) {
    			  d_phase -= CL_TWO_PI;
    			}
            }
            else if (d_phase < CL_MINUS_TWO_PI) {
    			while(d_phase < CL_MINUS_TWO_PI) {
    			  d_phase += CL_TWO_PI;
    			}
            }
		 */

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
		if ((d_phase > CL_TWO_PI) || (d_phase < CL_MINUS_TWO_PI)) {
			// d_phase = d_phase / CL_TWO_PI - (float)((int)(d_phase / CL_TWO_PI));
			// switch to multiplication for faster op
#if defined(__FMA__)
			d_phase = __builtin_fmaf(d_phase,CL_ONE_OVER_2PI,-(float)((int)(d_phase * CL_ONE_OVER_2PI)));
#else
			d_phase = d_phase * CL_ONE_OVER_2PI - (float)((int)(d_phase * CL_ONE_OVER_2PI));
#endif
			d_phase = d_phase * CL_TWO_PI;
		}
		n_i = sinf(-d_phase);
		n_r = cosf(-d_phase);

		//optr[i] = iptr[i] * nco_out;
		// FMA stands for fused multiply-add operations where FMA(a,b,c)=(a*b)+c and it does it as a single operation.
#if defined(__FMA__)
		o_r = __builtin_fmaf(iptr[i].real,n_r,-iptr[i].imag*n_i);
		o_i = __builtin_fmaf(iptr[i].real,n_i,iptr[i].imag*n_r);
#else
		i_r = iptr[i].real;
		i_i = iptr[i].imag;
		o_r = (i_r * n_r) - (i_i*n_i);
		o_i = (i_r * n_i) + (i_i * n_r);
#endif
		optr[i].real = o_r;
		optr[i].imag = o_i;

		//d_error = (*this.*d_phase_detector)(optr[i]);
		// 4th order in-place
		// d_error = (optr[i].real()>0 ? 1.0 : -1.0) * optr[i].imag() - (optr[i].imag()>0 ? 1.0 : -1.0) * optr[i].real();
		d_error = (o_r>0 ? 1.0 : -1.0) * o_i - (o_i>0 ? 1.0 : -1.0) * o_r;

		// d_error = gr::branchless_clip(d_error, 1.0);
		/*  Taken out for speed and consolidated
          x1 = fabsf(d_error+1);
          x2 = fabsf(d_error-1);
          x1 -= x2;
          d_error = 0.5*x1;
		 */
		// d_error = 0.5 * (fabsf(d_error+1) - fabsf(d_error-1));
		d_error = 0.5 * (std::abs(d_error+1) - std::abs(d_error-1));

		//advance_loop(d_error);
#if defined(__FMA__)
		d_freq = __builtin_fmaf(d_beta,d_error,d_freq);
#else
		d_freq = d_beta * d_error + d_freq;
#endif
		//d_freq = __builtin_fmaf(d_beta,d_error,d_freq);
		// This line is causing one of the greatest performance drops!  100 Msps -> 33 Msps!
#if defined(__FMA__)
		d_phase = d_phase + __builtin_fmaf(d_alpha,d_error,d_freq);
#else
		d_phase = d_phase + d_alpha * d_error + d_freq;
#endif
		// d_phase = d_phase + d_freq + d_alpha * d_error;
		// d_phase = d_phase + __builtin_fmaf(d_alpha,d_error,d_freq);

		//phase_wrap();
		// Moved up top
		/*
            if (d_phase > CL_TWO_PI) {
    			while(d_phase>CL_TWO_PI) {
    			  d_phase -= CL_TWO_PI;
    			}
            }
            else if (d_phase < CL_MINUS_TWO_PI) {
    			while(d_phase < CL_MINUS_TWO_PI) {
    			  d_phase += CL_TWO_PI;
    			}
            }
		 */

		//frequency_limit();
		if(d_freq > d_max_freq)
			d_freq = d_max_freq;
		else if(d_freq < d_min_freq)
			d_freq = d_min_freq;
	}

	if (d_genSignalPDUs) {
		pmt::pmt_t meta = pmt::make_dict();
		pmt::pmt_t data_out(pmt::init_c32vector(noutput_items, (gr_complex *)optr));
		pmt::pmt_t pdu = pmt::cons( meta, data_out );
		message_port_pub(pmt::mp("msgout"),pdu);
	}

	return noutput_items;
}

} /* namespace lfast */
} /* namespace gr */

