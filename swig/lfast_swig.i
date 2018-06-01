/* -*- c++ -*- */

#define LFAST_API

%include "gnuradio.i"			// the common stuff

//load generated python docstrings
%include "lfast_swig_doc.i"

%{
#include "lfast/costas2.h"
#include "lfast/costas4.h"
#include "lfast/agc_fast.h"
#include "lfast/agc_fast_ff.h"
#include "lfast/CC2F2ByteVector.h"
#include "lfast/nlog10volk.h"
#include "lfast/quad_demod_volk.h"
%}


%include "lfast/costas2.h"
GR_SWIG_BLOCK_MAGIC2(lfast, costas2);
%include "lfast/costas4.h"
GR_SWIG_BLOCK_MAGIC2(lfast, costas4);
%include "lfast/agc_fast.h"
GR_SWIG_BLOCK_MAGIC2(lfast, agc_fast);
%include "lfast/agc_fast_ff.h"
GR_SWIG_BLOCK_MAGIC2(lfast, agc_fast_ff);
%include "lfast/CC2F2ByteVector.h"
GR_SWIG_BLOCK_MAGIC2(lfast, CC2F2ByteVector);
%include "lfast/nlog10volk.h"
GR_SWIG_BLOCK_MAGIC2(lfast, nlog10volk);
%include "lfast/quad_demod_volk.h"
GR_SWIG_BLOCK_MAGIC2(lfast, quad_demod_volk);
