/* -*- c++ -*- */

#define LFAST_API

%include "gnuradio.i"			// the common stuff

//load generated python docstrings
%include "lfast_swig_doc.i"

%{
#include "lfast/costas2.h"
%}


%include "lfast/costas2.h"
GR_SWIG_BLOCK_MAGIC2(lfast, costas2);
