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


#ifndef INCLUDED_LFAST_COSTAS4_H
#define INCLUDED_LFAST_COSTAS4_H

#include <lfast/api.h>
#include <gnuradio/sync_block.h>
#include <gnuradio/blocks/control_loop.h>

namespace gr {
  namespace lfast {

    /*!
     * \brief <+description of block+>
     * \ingroup lfast
     *
     */
    class LFAST_API costas4
	: virtual public gr::sync_block,
	  virtual public blocks::control_loop
    {
     public:
      typedef std::shared_ptr<costas4> sptr;

      /*!
       * \brief Return a shared_ptr to a new instance of lfast::costas4.
       *
       * To avoid accidental use of raw pointers, lfast::costas2's
       * constructor is in a private implementation
       * class. lfast::costas2::make is the public interface for
       * creating new instances.
       */
      static sptr make(float loop_bw, int order, bool genPDUs);

      virtual float error() const = 0;
    };

  } // namespace lfast
} // namespace gr

#endif /* INCLUDED_LFAST_COSTAS4_H */

