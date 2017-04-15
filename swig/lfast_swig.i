/* -*- c++ -*- */

#define LFAST_API

%include "gnuradio.i"			// the common stuff

//load generated python docstrings
%include "lfast_swig_doc.i"

%{
#include "lfast/costas2.h"
#include "lfast/agc_fast.h"
#include "lfast/agc_fast_ff.h"
#include "lfast/CC2F2ByteVector.h"
%}


%include "lfast/costas2.h"
GR_SWIG_BLOCK_MAGIC2(lfast, costas2);
%include "lfast/agc_fast.h"
GR_SWIG_BLOCK_MAGIC2(lfast, agc_fast);
%include "lfast/agc_fast_ff.h"
GR_SWIG_BLOCK_MAGIC2(lfast, agc_fast_ff);
%include "lfast/CC2F2ByteVector.h"
GR_SWIG_BLOCK_MAGIC2(lfast, CC2F2ByteVector);
