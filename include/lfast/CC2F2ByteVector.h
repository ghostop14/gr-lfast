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


#ifndef INCLUDED_TESTTIMING_CC2F2BYTEVECTOR_H
#define INCLUDED_TESTTIMING_CC2F2BYTEVECTOR_H

#include <lfast/api.h>
#include <gnuradio/sync_decimator.h>

namespace gr {
  namespace lfast {

    /*!
     * \brief <+description of block+>
     * \ingroup testtiming
     *
     */
    class LFAST_API CC2F2ByteVector : virtual public gr::sync_decimator
    {
     public:
      typedef std::shared_ptr<CC2F2ByteVector> sptr;

      /*!
       * \brief Return a shared_ptr to a new instance of testtiming::CC2F2ByteVector.
       *
       * To avoid accidental use of raw pointers, testtiming::CC2F2ByteVector's
       * constructor is in a private implementation
       * class. testtiming::CC2F2ByteVector::make is the public interface for
       * creating new instances.
       */
      static sptr make(int scale=1,int vecLength=1,int numVecItems=1);
    };

  } // namespace testtiming
} // namespace gr

#endif /* INCLUDED_TESTTIMING_CC2F2BYTEVECTOR_H */

