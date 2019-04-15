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

#ifndef INCLUDED_TESTTIMING_CC2F2BYTEVECTOR_IMPL_H
#define INCLUDED_TESTTIMING_CC2F2BYTEVECTOR_IMPL_H

#include <lfast/CC2F2ByteVector.h>
#include <boost/thread/thread.hpp>

#define LF_MAX_THREADS 8

namespace gr {
  namespace lfast {

    class CC2F2ByteVector_impl : public CC2F2ByteVector
    {
    protected:
		int d_scale;
		int d_vlen;

		boost::thread *threads[LF_MAX_THREADS];
		bool dataReady[LF_MAX_THREADS];
		long threadBlockSize[LF_MAX_THREADS];
		long startIndex[LF_MAX_THREADS];
		const gr_complex *inBuffer;
		char *outBuffer;

		unsigned concurentThreadsSupported;
		unsigned concurrentMinus1;

		bool stopThreads;

        boost::mutex d_mutex;

		float min_val = -128;
		float max_val = 127;

		float *floatBuff = NULL;
		int curBufferSize=0;

		void processItems(int threadIndex);

     public:
      CC2F2ByteVector_impl(int scale,int vecLength,int numVecItems);
      ~CC2F2ByteVector_impl();
      virtual bool stop();
      void setBufferLength(int numItems);

      // Where all the action really happens
      int work_original(int noutput_items,
         gr_vector_const_void_star &input_items,
         gr_vector_void_star &output_items);
      int work_test(int noutput_items,
         gr_vector_const_void_star &input_items,
         gr_vector_void_star &output_items);
      int work(int noutput_items,
         gr_vector_const_void_star &input_items,
         gr_vector_void_star &output_items);
    };

  } // namespace testtiming
} // namespace gr

#endif /* INCLUDED_TESTTIMING_CC2F2BYTEVECTOR_IMPL_H */

