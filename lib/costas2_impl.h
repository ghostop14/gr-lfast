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

#ifndef INCLUDED_LFAST_COSTAS2_IMPL_H
#define INCLUDED_LFAST_COSTAS2_IMPL_H

#include <lfast/costas2.h>

namespace gr {
  namespace lfast {

    class costas2_impl : public costas2
    {
     private:
       int d_order;
       float d_error;
       float d_noise;

       bool d_genSignalPDUs;

       float
       phase_detector_2(gr_complex sample) const
       {
         return (sample.real()*sample.imag());
       }

       float (costas2_impl::*d_phase_detector)(gr_complex sample) const;

     public:
      costas2_impl(float loop_bw, int order, bool genPDUs);
      ~costas2_impl();

      float error() const;

      // void printSineError();

      void handle_set_noise(pmt::pmt_t msg);

      void handleMsgIn(pmt::pmt_t msg);

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

#endif /* INCLUDED_LFAST_COSTAS2_IMPL_H */

