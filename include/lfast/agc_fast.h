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


#ifndef INCLUDED_LFAST_AGC_FAST_H
#define INCLUDED_LFAST_AGC_FAST_H

#include <lfast/api.h>
#include <gnuradio/sync_block.h>
#include <lfast/agc.h>

namespace gr {
  namespace lfast {

    /*!
     * \brief <+description of block+>
     * \ingroup lfast
     *
     */
    class LFAST_API agc_fast : virtual public gr::sync_block
    {
     public:
      typedef std::shared_ptr<agc_fast> sptr;

      /*!
       * \brief Return a shared_ptr to a new instance of lfast::agc_fast.
       *
       * To avoid accidental use of raw pointers, lfast::agc_fast's
       * constructor is in a private implementation
       * class. lfast::agc_fast::make is the public interface for
       * creating new instances.
       */
      static sptr make(float rate = 1e-4, float reference = 1.0, float gain = 1.0);
      virtual float rate() const = 0;
      virtual float reference() const = 0;
      virtual float gain() const = 0;
      virtual float max_gain() const = 0;

      virtual void set_rate(float rate) = 0;
      virtual void set_reference(float reference) = 0;
      virtual void set_gain(float gain) = 0;
      virtual void set_max_gain(float max_gain) = 0;
    };

  } // namespace lfast
} // namespace gr

#endif /* INCLUDED_LFAST_AGC_FAST_H */

