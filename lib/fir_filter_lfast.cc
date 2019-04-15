/*
 * fir_filter_lfast.cc
 *
 *      Author: ghostop14
 */

#include "fir_filter_lfast.h"
#include "scomplex.h"

namespace gr {
  namespace lfast {
  template<class io_type, class tap_type> Filter<io_type,tap_type>::Filter() {
	alignedTaps = NULL;
	numTaps = 0;
	size_t memAlignment = volk_get_alignment();
	singlePointBuffer = (io_type *)volk_malloc(1*sizeof(io_type),memAlignment);
  }

  template<class io_type, class tap_type> Filter<io_type,tap_type>::Filter(const std::vector<tap_type>& newTaps) {
	alignedTaps = NULL;
	size_t memAlignment = volk_get_alignment();
	singlePointBuffer = (io_type *)volk_malloc(1*sizeof(io_type),memAlignment);

	setTaps(newTaps);
  }

  template<class io_type, class tap_type> Filter<io_type,tap_type>::~Filter() {
		if (alignedTaps) {
			volk_free(alignedTaps);
			alignedTaps = NULL;
		}

		volk_free(singlePointBuffer);
  }

  template<class io_type, class tap_type> std::vector<tap_type> Filter<io_type,tap_type>::getTaps() const {
		// Need to reverse it to return the true taps
	    std::vector<tap_type> revTaps = d_taps;
		std::reverse(revTaps.begin(), revTaps.end());

		return revTaps;
  }

  template<class io_type, class tap_type> void Filter<io_type,tap_type>::setTaps(const std::vector<tap_type>& newTaps) {
		// For a FIR filter, the taps have to be reversed to be applied.
		// Reversing them here makes it easy when we call the volk routine.
		d_taps = newTaps;
		std::reverse(d_taps.begin(), d_taps.end());

		// mem align taps for better SIMD performance
		if (alignedTaps) {
			volk_free(alignedTaps);
		}

		numTaps = d_taps.size();

		size_t memAlignment = volk_get_alignment();
		alignedTaps = (tap_type *)volk_malloc(numTaps*sizeof(tap_type),memAlignment);
		memcpy(alignedTaps,&d_taps[0],numTaps*sizeof(tap_type));
  }

  // -------------------------------------------
  // ----  Complex Inputs, float taps
  // -------------------------------------------
  FIRFilterCCF::FIRFilterCCF():Filter() {
  }

  FIRFilterCCF::FIRFilterCCF(const std::vector<float>& newTaps):Filter(newTaps) {
  }

  long FIRFilterCCF::filterN(gr_complex *outputBuffer, const gr_complex *inputBuffer, long numSamples) {
  	const gr_complex *in;
  	gr_complex *out;

  	// Using the pointers saves the offset dereferencing so it's slightly faster.
  	in=inputBuffer;
  	out=outputBuffer;

  	for (long i=0;i<numSamples;i++) {
  		// volk_32fc_32f_dot_prod_32fc_a(&outputBuffer[i],&inputBuffer[i],&alignedTaps[0],numTaps);
  		// volk_32fc_32f_dot_prod_32fc_a(out++,in++,&alignedTaps[0],numTaps);
  		volk_32fc_32f_dot_prod_32fc(out++,in++,&alignedTaps[0],numTaps);
  	}

  	return numSamples;
  }

  gr_complex FIRFilterCCF::filter(const gr_complex *inputBuffer) {
		volk_32fc_32f_dot_prod_32fc(singlePointBuffer,inputBuffer,&alignedTaps[0],numTaps);

		return *singlePointBuffer;
  }

  long FIRFilterCCF::filterNdec(gr_complex *outputBuffer, const gr_complex *inputBuffer, long numSamples, int decimation) {
  	long decimatedCount = numSamples / decimation;

  	// Using the pointers saves the offset dereferencing so it's slightly faster.
  	long j=0;

  	for (long i=0;i<decimatedCount;i++) {
  		volk_32fc_32f_dot_prod_32fc(&outputBuffer[i],&inputBuffer[j],&alignedTaps[0],numTaps);
  		j+= decimation;
  	}

  	return decimatedCount;
  }

  long FIRFilterCCF::filterCPU(gr_complex *outputBuffer, const gr_complex *inputBuffer, long numSamples) {

  	long index;
  	float *pFloat = (float *)inputBuffer;
  	float *pTap = &alignedTaps[0];
  	float realPart;
  	float imagPart;
  	StructComplex *structComplex;

  	for (long i=0;i<numSamples;i++) {
  		realPart = 0.0;
  		imagPart = 0.0;
  		pTap = &alignedTaps[0];
  		structComplex = (StructComplex *)&outputBuffer[i];
  		pFloat = (float *)&inputBuffer[i];

  		for (long j=0;j<numTaps;j++) {
  			realPart += (*pFloat++) * (*pTap);
  			imagPart += (*pFloat++) * (*pTap++);
  		}

  		structComplex->real = realPart;
  		structComplex->imag =imagPart;
  	}

  	return numSamples;
  }

  FIRFilterCCF::~FIRFilterCCF() {
  }


  // -------------------------------------------
  // ----  Float Inputs, float taps
  // -------------------------------------------
  FIRFilterFFF::FIRFilterFFF():Filter() {
  }

  FIRFilterFFF::FIRFilterFFF(const std::vector<float>& newTaps):Filter(newTaps) {
  }

  long FIRFilterFFF::filterN(float *outputBuffer, const float *inputBuffer, long numSamples) {
  	const float *in;
  	float *out;

  	// Using the pointers saves the offset dereferencing so it's slightly faster.
  	in=inputBuffer;
  	out=outputBuffer;

  	for (long i=0;i<numSamples;i++) {
  		volk_32f_x2_dot_prod_32f(out++,in++,&alignedTaps[0],numTaps);
  	}

  	return numSamples;
  }

  gr_complex FIRFilterFFF::filter(const float *inputBuffer) {
	  volk_32f_x2_dot_prod_32f(singlePointBuffer,inputBuffer,&alignedTaps[0],numTaps);

	  return *singlePointBuffer;
  }

  long FIRFilterFFF::filterNdec(float *outputBuffer, const float *inputBuffer, long numSamples, int decimation) {
  	long decimatedCount = numSamples / decimation;

  	// Using the pointers saves the offset dereferencing so it's slightly faster.
  	long j=0;

  	for (long i=0;i<decimatedCount;i++) {
  		volk_32f_x2_dot_prod_32f(&outputBuffer[i],&inputBuffer[j],&alignedTaps[0],numTaps);
  		j+= decimation;
  	}

  	return decimatedCount;
  }

  long FIRFilterFFF::filterCPU(float *outputBuffer, const float *inputBuffer, long numSamples) {

  	long index;
  	float *pFloat = (float *)inputBuffer;
  	float *pTap = &alignedTaps[0];
  	float sum;

  	for (long i=0;i<numSamples;i++) {
  		sum = 0.0;
  		pTap = &alignedTaps[0];
  		pFloat = (float *)&inputBuffer[i];

  		for (long j=0;j<numTaps;j++) {
  			sum += (*pFloat++) * (*pTap++);
  		}

  		outputBuffer[i] = sum;
  	}

  	return numSamples;
  }

  FIRFilterFFF::~FIRFilterFFF() {
  }

  // -------------------------------------------
  // ----  Complex Inputs, Complex taps
  // -------------------------------------------
  FIRFilterCCC::FIRFilterCCC():Filter() {
  }

  FIRFilterCCC::FIRFilterCCC(const std::vector<gr_complex>& newTaps):Filter(newTaps) {
  }

  long FIRFilterCCC::filterN(gr_complex *outputBuffer, const gr_complex *inputBuffer, long numSamples) {
  	const gr_complex *in;
  	gr_complex *out;

  	// Using the pointers saves the offset dereferencing so it's slightly faster.
  	in=inputBuffer;
  	out=outputBuffer;

  	for (long i=0;i<numSamples;i++) {
  		volk_32fc_x2_dot_prod_32fc(out++,in++,&alignedTaps[0],numTaps);
  	}

  	return numSamples;
  }

  gr_complex FIRFilterCCC::filter(const gr_complex *inputBuffer) {
	  volk_32fc_x2_dot_prod_32fc(singlePointBuffer,inputBuffer,&alignedTaps[0],numTaps);

	  return *singlePointBuffer;
  }

  long FIRFilterCCC::filterNdec(gr_complex *outputBuffer, const gr_complex *inputBuffer, long numSamples, int decimation) {
  	long decimatedCount = numSamples / decimation;

  	// Using the pointers saves the offset dereferencing so it's slightly faster.
  	long j=0;

  	for (long i=0;i<decimatedCount;i++) {
  		volk_32fc_x2_dot_prod_32fc(&outputBuffer[i],&inputBuffer[j],&alignedTaps[0],numTaps);
  		j+= decimation;
  	}

  	return decimatedCount;
  }

  long FIRFilterCCC::filterCPU(gr_complex *outputBuffer, const gr_complex *inputBuffer, long numSamples) {

  	long index;
  	gr_complex *pData = (gr_complex *)inputBuffer;
  	gr_complex *pTap = &alignedTaps[0];
  	gr_complex sum;

  	for (long i=0;i<numSamples;i++) {
  		sum = gr_complex(0.0,0.0);
  		pTap = &alignedTaps[0];
  		pData = (gr_complex *)&inputBuffer[i];

  		for (long j=0;j<numTaps;j++) {
  			sum += (*pData++) * (*pTap++);
  		}

  		outputBuffer[i] = sum;
  	}

  	return numSamples;
  }

  FIRFilterCCC::~FIRFilterCCC() {
  }


  // -----------------------------------------------------------------
  // ------  Multi-threaded filters ----------------------------------
  // -----------------------------------------------------------------

	/*
	 * Multi-threaded Complex FIR Filter with float taps
	 *
	 */

	template<class io_type> MTBase<io_type>::MTBase(int nthreads) {
		setThreads(nthreads);
		decimation = 1;
	}

	template<class io_type> void MTBase<io_type>::setThreads(int nthreads) {
        if (nthreads<1)
        	nthreads=1;
        else if (nthreads > 16)
        	nthreads = 16;

        d_nthreads = nthreads;

        for (int i=0;i<16;i++) {
        	threadRunning[i] = false;
        	threads[i] = NULL;
        }

        threadReady = true;
	}

	template<class io_type> MTBase<io_type>::~MTBase() {
		while (anyThreadRunning())
			usleep(10);

		for (int i=0;i<d_nthreads;i++) {
			if (threads[i]) {
				delete threads[i];
				threads[i] = NULL;
			}
		}
	}

    template<class io_type> bool MTBase<io_type>::anyThreadRunning() {
    	for (int i=0;i<d_nthreads;i++)
    		if (threadRunning[i]) {
    			return true;
    		}

    	return false;
    }

    template<class io_type> long MTBase<io_type>::calcDecimationBlockSize(long numSamples) {
    	long decBlockSize;

    	long nonDecBlockSize = numSamples / d_nthreads;

    	long decBlockUnit = nonDecBlockSize / decimation;

    	decBlockSize = decBlockUnit * decimation;

    	return decBlockSize;
    }

    template<class io_type> long MTBase<io_type>::calcDecimationIndex(long blockStartIndex) {
		// NOTE: calcDecimationIndex assumes the index is an even multiple of a decimation block size
		// In other words, calcDecimationBlockSize was called to get an appropriate block size,
		// then blockStartIndex is an integer multiple of that result.

    	return (blockStartIndex / decimation);
    }

    // ------------------------------------------------
    // Multi-threaded filter, complex data, float taps
    // ------------------------------------------------

    FIRFilterCCF_MT::FIRFilterCCF_MT(int nthreads):MTBase(nthreads),FIRFilterCCF() {

    }

    FIRFilterCCF_MT::FIRFilterCCF_MT(const std::vector<float>& newTaps, int nthreads):MTBase(nthreads),FIRFilterCCF(newTaps) {

	}

    FIRFilterCCF_MT::~FIRFilterCCF_MT() {

	}

    void FIRFilterCCF_MT::runThread1(long startIndex,long numSamples) {
    	if (decimating()) {
    		long decIndex = calcDecimationIndex(startIndex);

        	FIRFilterCCF::filterNdec(&pOutputBuffer[decIndex],&pInputBuffer[startIndex],numSamples,decimation);
    	}
    	else {
        	FIRFilterCCF::filterN(&pOutputBuffer[startIndex],&pInputBuffer[startIndex],numSamples);
    	}

    	threadRunning[0] = false;
    }

    void FIRFilterCCF_MT::runThread2(long startIndex,long numSamples) {
    	if (decimating()) {
    		long decIndex = calcDecimationIndex(startIndex);

        	FIRFilterCCF::filterNdec(&pOutputBuffer[decIndex],&pInputBuffer[startIndex],numSamples,decimation);
    	}
    	else {
        	FIRFilterCCF::filterN(&pOutputBuffer[startIndex],&pInputBuffer[startIndex],numSamples);
    	}
    	threadRunning[1] = false;
    }

    void FIRFilterCCF_MT::runThread3(long startIndex,long numSamples) {
    	if (decimating()) {
    		long decIndex = calcDecimationIndex(startIndex);

        	FIRFilterCCF::filterNdec(&pOutputBuffer[decIndex],&pInputBuffer[startIndex],numSamples,decimation);
    	}
    	else {
        	FIRFilterCCF::filterN(&pOutputBuffer[startIndex],&pInputBuffer[startIndex],numSamples);
    	}
    	threadRunning[2] = false;
    }

    void FIRFilterCCF_MT::runThread4(long startIndex,long numSamples) {
    	if (decimating()) {
    		long decIndex = calcDecimationIndex(startIndex);

        	FIRFilterCCF::filterNdec(&pOutputBuffer[decIndex],&pInputBuffer[startIndex],numSamples,decimation);
    	}
    	else {
        	FIRFilterCCF::filterN(&pOutputBuffer[startIndex],&pInputBuffer[startIndex],numSamples);
    	}
    	threadRunning[3] = false;
    }

    void FIRFilterCCF_MT::runThread5(long startIndex,long numSamples) {
    	if (decimating()) {
    		long decIndex = calcDecimationIndex(startIndex);

        	FIRFilterCCF::filterNdec(&pOutputBuffer[decIndex],&pInputBuffer[startIndex],numSamples,decimation);
    	}
    	else {
        	FIRFilterCCF::filterN(&pOutputBuffer[startIndex],&pInputBuffer[startIndex],numSamples);
    	}
    	threadRunning[4] = false;
    }

    void FIRFilterCCF_MT::runThread6(long startIndex,long numSamples) {
    	if (decimating()) {
    		long decIndex = calcDecimationIndex(startIndex);

        	FIRFilterCCF::filterNdec(&pOutputBuffer[decIndex],&pInputBuffer[startIndex],numSamples,decimation);
    	}
    	else {
        	FIRFilterCCF::filterN(&pOutputBuffer[startIndex],&pInputBuffer[startIndex],numSamples);
    	}
    	threadRunning[5] = false;
    }

    void FIRFilterCCF_MT::runThread7(long startIndex,long numSamples) {
    	if (decimating()) {
    		long decIndex = calcDecimationIndex(startIndex);

        	FIRFilterCCF::filterNdec(&pOutputBuffer[decIndex],&pInputBuffer[startIndex],numSamples,decimation);
    	}
    	else {
        	FIRFilterCCF::filterN(&pOutputBuffer[startIndex],&pInputBuffer[startIndex],numSamples);
    	}
    	threadRunning[6] = false;
    }

    void FIRFilterCCF_MT::runThread8(long startIndex,long numSamples) {
    	if (decimating()) {
    		long decIndex = calcDecimationIndex(startIndex);

        	FIRFilterCCF::filterNdec(&pOutputBuffer[decIndex],&pInputBuffer[startIndex],numSamples,decimation);
    	}
    	else {
        	FIRFilterCCF::filterN(&pOutputBuffer[startIndex],&pInputBuffer[startIndex],numSamples);
    	}
    	threadRunning[7] = false;
    }

    void FIRFilterCCF_MT::runThread9(long startIndex,long numSamples) {
    	if (decimating()) {
    		long decIndex = calcDecimationIndex(startIndex);

        	FIRFilterCCF::filterNdec(&pOutputBuffer[decIndex],&pInputBuffer[startIndex],numSamples,decimation);
    	}
    	else {
        	FIRFilterCCF::filterN(&pOutputBuffer[startIndex],&pInputBuffer[startIndex],numSamples);
    	}
    	threadRunning[8] = false;
    }

    void FIRFilterCCF_MT::runThread10(long startIndex,long numSamples) {
    	if (decimating()) {
    		long decIndex = calcDecimationIndex(startIndex);

        	FIRFilterCCF::filterNdec(&pOutputBuffer[decIndex],&pInputBuffer[startIndex],numSamples,decimation);
    	}
    	else {
        	FIRFilterCCF::filterN(&pOutputBuffer[startIndex],&pInputBuffer[startIndex],numSamples);
    	}
    	threadRunning[9] = false;
    }

    void FIRFilterCCF_MT::runThread11(long startIndex,long numSamples) {
    	if (decimating()) {
    		long decIndex = calcDecimationIndex(startIndex);

        	FIRFilterCCF::filterNdec(&pOutputBuffer[decIndex],&pInputBuffer[startIndex],numSamples,decimation);
    	}
    	else {
        	FIRFilterCCF::filterN(&pOutputBuffer[startIndex],&pInputBuffer[startIndex],numSamples);
    	}
    	threadRunning[10] = false;
    }

    void FIRFilterCCF_MT::runThread12(long startIndex,long numSamples) {
    	if (decimating()) {
    		long decIndex = calcDecimationIndex(startIndex);

        	FIRFilterCCF::filterNdec(&pOutputBuffer[decIndex],&pInputBuffer[startIndex],numSamples,decimation);
    	}
    	else {
        	FIRFilterCCF::filterN(&pOutputBuffer[startIndex],&pInputBuffer[startIndex],numSamples);
    	}
    	threadRunning[11] = false;
    }

    void FIRFilterCCF_MT::runThread13(long startIndex,long numSamples) {
    	if (decimating()) {
    		long decIndex = calcDecimationIndex(startIndex);

        	FIRFilterCCF::filterNdec(&pOutputBuffer[decIndex],&pInputBuffer[startIndex],numSamples,decimation);
    	}
    	else {
        	FIRFilterCCF::filterN(&pOutputBuffer[startIndex],&pInputBuffer[startIndex],numSamples);
    	}
    	threadRunning[12] = false;
    }

    void FIRFilterCCF_MT::runThread14(long startIndex,long numSamples) {
    	if (decimating()) {
    		long decIndex = calcDecimationIndex(startIndex);

        	FIRFilterCCF::filterNdec(&pOutputBuffer[decIndex],&pInputBuffer[startIndex],numSamples,decimation);
    	}
    	else {
        	FIRFilterCCF::filterN(&pOutputBuffer[startIndex],&pInputBuffer[startIndex],numSamples);
    	}
    	threadRunning[13] = false;
    }

    void FIRFilterCCF_MT::runThread15(long startIndex,long numSamples) {
    	if (decimating()) {
    		long decIndex = calcDecimationIndex(startIndex);

        	FIRFilterCCF::filterNdec(&pOutputBuffer[decIndex],&pInputBuffer[startIndex],numSamples,decimation);
    	}
    	else {
        	FIRFilterCCF::filterN(&pOutputBuffer[startIndex],&pInputBuffer[startIndex],numSamples);
    	}
    	threadRunning[14] = false;
    }

    void FIRFilterCCF_MT::runThread16(long startIndex,long numSamples) {
    	if (decimating()) {
    		long decIndex = calcDecimationIndex(startIndex);

        	FIRFilterCCF::filterNdec(&pOutputBuffer[decIndex],&pInputBuffer[startIndex],numSamples,decimation);
    	}
    	else {
        	FIRFilterCCF::filterN(&pOutputBuffer[startIndex],&pInputBuffer[startIndex],numSamples);
    	}
    	threadRunning[15] = false;
    }

	// NOTE: This routine is expecting numSamples to be an integer multiple of the number of taps
	long FIRFilterCCF_MT::filterN(gr_complex *outputBuffer, const gr_complex *inputBuffer, long numSamples) {
		while (anyThreadRunning() || (threadReady == false))
			usleep(10);

		pInputBuffer = inputBuffer;
		pOutputBuffer = outputBuffer;

		long blockSize = numSamples / d_nthreads;
		long lastBlock = numSamples - (d_nthreads-1)*blockSize;
		threadReady = false;

		int curBlock;

		// std::cout << "Starting threads.  numSamples = " << numSamples << " Block size =" << blockSize << " last block=" << lastBlock << std::endl;

		for (int i=0;i<d_nthreads;i++) {
			threadRunning[i] = true;

			if (i<(d_nthreads-1))
				curBlock = blockSize;
			else
				curBlock = lastBlock;

			switch(i) {
			case 0:
	    		threads[i] = new boost::thread(boost::bind(&FIRFilterCCF_MT::runThread1, this,i*blockSize,curBlock));
				break;
			case 1:
	    		threads[i] = new boost::thread(boost::bind(&FIRFilterCCF_MT::runThread2, this,i*blockSize,curBlock));
				break;
			case 2:
	    		threads[i] = new boost::thread(boost::bind(&FIRFilterCCF_MT::runThread3, this,i*blockSize,curBlock));
				break;
			case 3:
	    		threads[i] = new boost::thread(boost::bind(&FIRFilterCCF_MT::runThread4, this,i*blockSize,curBlock));
				break;
			case 4:
	    		threads[i] = new boost::thread(boost::bind(&FIRFilterCCF_MT::runThread5, this,i*blockSize,curBlock));
				break;
			case 5:
	    		threads[i] = new boost::thread(boost::bind(&FIRFilterCCF_MT::runThread6, this,i*blockSize,curBlock));
				break;
			case 6:
	    		threads[i] = new boost::thread(boost::bind(&FIRFilterCCF_MT::runThread7, this,i*blockSize,curBlock));
				break;
			case 7:
	    		threads[i] = new boost::thread(boost::bind(&FIRFilterCCF_MT::runThread8, this,i*blockSize,curBlock));
				break;
			case 8:
	    		threads[i] = new boost::thread(boost::bind(&FIRFilterCCF_MT::runThread9, this,i*blockSize,curBlock));
				break;
			case 9:
	    		threads[i] = new boost::thread(boost::bind(&FIRFilterCCF_MT::runThread10, this,i*blockSize,curBlock));
				break;
			case 10:
	    		threads[i] = new boost::thread(boost::bind(&FIRFilterCCF_MT::runThread11, this,i*blockSize,curBlock));
				break;
			case 11:
	    		threads[i] = new boost::thread(boost::bind(&FIRFilterCCF_MT::runThread12, this,i*blockSize,curBlock));
				break;
			case 12:
	    		threads[i] = new boost::thread(boost::bind(&FIRFilterCCF_MT::runThread13, this,i*blockSize,curBlock));
				break;
			case 13:
	    		threads[i] = new boost::thread(boost::bind(&FIRFilterCCF_MT::runThread14, this,i*blockSize,curBlock));
				break;
			case 14:
	    		threads[i] = new boost::thread(boost::bind(&FIRFilterCCF_MT::runThread15, this,i*blockSize,curBlock));
				break;
			case 15:
	    		threads[i] = new boost::thread(boost::bind(&FIRFilterCCF_MT::runThread16, this,i*blockSize,curBlock));
				break;
			}
		}

		// std::cout << "Waiting for threads to finish." << std::endl;
		while (anyThreadRunning())
			usleep(10);

		// std::cout << "Threads finished." << std::endl;
		for (int i=0;i<d_nthreads;i++) {
			delete threads[i];
			threads[i] = NULL;
		}

		threadReady = true;

		return numSamples;
	}

	// OutputBuffer here should be at least numSamples/decimation in length
	long FIRFilterCCF_MT::filterNdec(gr_complex *outputBuffer, const gr_complex *inputBuffer, long numSamples, int decimation) {
		// For testing purposes
		// return FIRFilterCCF::filterNdec(outputBuffer,inputBuffer,numSamples*decimation,decimation);

		while (anyThreadRunning() || (threadReady == false))
			usleep(10);

		pInputBuffer = inputBuffer;
		pOutputBuffer = outputBuffer;

		// numSamples passed in here will be the decimated # (noutput_items).
		long inputSamples = numSamples * decimation;

		long blockSize = calcDecimationBlockSize(inputSamples);
		long lastBlock = inputSamples - (d_nthreads-1)*blockSize;
		//std::cout << "Input Samples: " << inputSamples << " Decimated Block Size: " << blockSize << " last block: " << lastBlock << std::endl;
		threadReady = false;

		long curBlock;

		// std::cout << "Starting threads.  numSamples = " << numSamples << " Block size =" << blockSize << " last block=" << lastBlock << std::endl;

		for (int i=0;i<d_nthreads;i++) {
			threadRunning[i] = true;

			if (i<(d_nthreads-1))
				curBlock = blockSize;
			else
				curBlock = lastBlock;

			switch(i) {
			case 0:
	    		threads[i] = new boost::thread(boost::bind(&FIRFilterCCF_MT::runThread1, this,i*blockSize,curBlock));
				break;
			case 1:
	    		threads[i] = new boost::thread(boost::bind(&FIRFilterCCF_MT::runThread2, this,i*blockSize,curBlock));
				break;
			case 2:
	    		threads[i] = new boost::thread(boost::bind(&FIRFilterCCF_MT::runThread3, this,i*blockSize,curBlock));
				break;
			case 3:
	    		threads[i] = new boost::thread(boost::bind(&FIRFilterCCF_MT::runThread4, this,i*blockSize,curBlock));
				break;
			case 4:
	    		threads[i] = new boost::thread(boost::bind(&FIRFilterCCF_MT::runThread5, this,i*blockSize,curBlock));
				break;
			case 5:
	    		threads[i] = new boost::thread(boost::bind(&FIRFilterCCF_MT::runThread6, this,i*blockSize,curBlock));
				break;
			case 6:
	    		threads[i] = new boost::thread(boost::bind(&FIRFilterCCF_MT::runThread7, this,i*blockSize,curBlock));
				break;
			case 7:
	    		threads[i] = new boost::thread(boost::bind(&FIRFilterCCF_MT::runThread8, this,i*blockSize,curBlock));
				break;
			case 8:
	    		threads[i] = new boost::thread(boost::bind(&FIRFilterCCF_MT::runThread9, this,i*blockSize,curBlock));
				break;
			case 9:
	    		threads[i] = new boost::thread(boost::bind(&FIRFilterCCF_MT::runThread10, this,i*blockSize,curBlock));
				break;
			case 10:
	    		threads[i] = new boost::thread(boost::bind(&FIRFilterCCF_MT::runThread11, this,i*blockSize,curBlock));
				break;
			case 11:
	    		threads[i] = new boost::thread(boost::bind(&FIRFilterCCF_MT::runThread12, this,i*blockSize,curBlock));
				break;
			case 12:
	    		threads[i] = new boost::thread(boost::bind(&FIRFilterCCF_MT::runThread13, this,i*blockSize,curBlock));
				break;
			case 13:
	    		threads[i] = new boost::thread(boost::bind(&FIRFilterCCF_MT::runThread14, this,i*blockSize,curBlock));
				break;
			case 14:
	    		threads[i] = new boost::thread(boost::bind(&FIRFilterCCF_MT::runThread15, this,i*blockSize,curBlock));
				break;
			case 15:
	    		threads[i] = new boost::thread(boost::bind(&FIRFilterCCF_MT::runThread16, this,i*blockSize,curBlock));
				break;
			}
		}

		// std::cout << "Waiting for threads to finish." << std::endl;
		while (anyThreadRunning())
			usleep(10);

		// std::cout << "Threads finished." << std::endl;
		for (int i=0;i<d_nthreads;i++) {
			delete threads[i];
			threads[i] = NULL;
		}

		threadReady = true;

		return numSamples;
	}

    // ------------------------------------------------
    // Multi-threaded filter, float data, float taps
    // ------------------------------------------------

    FIRFilterFFF_MT::FIRFilterFFF_MT(int nthreads):MTBase(nthreads),FIRFilterFFF() {

    }

    FIRFilterFFF_MT::FIRFilterFFF_MT(const std::vector<float>& newTaps, int nthreads):MTBase(nthreads),FIRFilterFFF(newTaps) {

	}

    FIRFilterFFF_MT::~FIRFilterFFF_MT() {

	}

    void FIRFilterFFF_MT::runThread1(long startIndex,long numSamples) {
    	if (decimating()) {
    		long decIndex = calcDecimationIndex(startIndex);

    		FIRFilterFFF::filterNdec(&pOutputBuffer[decIndex],&pInputBuffer[startIndex],numSamples,decimation);
    	}
    	else {
    		FIRFilterFFF::filterN(&pOutputBuffer[startIndex],&pInputBuffer[startIndex],numSamples);
    	}
    	threadRunning[0] = false;
    }

    void FIRFilterFFF_MT::runThread2(long startIndex,long numSamples) {
    	if (decimating()) {
    		long decIndex = calcDecimationIndex(startIndex);

    		FIRFilterFFF::filterNdec(&pOutputBuffer[decIndex],&pInputBuffer[startIndex],numSamples,decimation);
    	}
    	else {
    		FIRFilterFFF::filterN(&pOutputBuffer[startIndex],&pInputBuffer[startIndex],numSamples);
    	}
    	threadRunning[1] = false;
    }

    void FIRFilterFFF_MT::runThread3(long startIndex,long numSamples) {
    	if (decimating()) {
    		long decIndex = calcDecimationIndex(startIndex);

    		FIRFilterFFF::filterNdec(&pOutputBuffer[decIndex],&pInputBuffer[startIndex],numSamples,decimation);
    	}
    	else {
    		FIRFilterFFF::filterN(&pOutputBuffer[startIndex],&pInputBuffer[startIndex],numSamples);
    	}
    	threadRunning[2] = false;
    }

    void FIRFilterFFF_MT::runThread4(long startIndex,long numSamples) {
    	if (decimating()) {
    		long decIndex = calcDecimationIndex(startIndex);

    		FIRFilterFFF::filterNdec(&pOutputBuffer[decIndex],&pInputBuffer[startIndex],numSamples,decimation);
    	}
    	else {
    		FIRFilterFFF::filterN(&pOutputBuffer[startIndex],&pInputBuffer[startIndex],numSamples);
    	}
    	threadRunning[3] = false;
    }

    void FIRFilterFFF_MT::runThread5(long startIndex,long numSamples) {
    	if (decimating()) {
    		long decIndex = calcDecimationIndex(startIndex);

    		FIRFilterFFF::filterNdec(&pOutputBuffer[decIndex],&pInputBuffer[startIndex],numSamples,decimation);
    	}
    	else {
    		FIRFilterFFF::filterN(&pOutputBuffer[startIndex],&pInputBuffer[startIndex],numSamples);
    	}
    	threadRunning[4] = false;
    }

    void FIRFilterFFF_MT::runThread6(long startIndex,long numSamples) {
    	if (decimating()) {
    		long decIndex = calcDecimationIndex(startIndex);

    		FIRFilterFFF::filterNdec(&pOutputBuffer[decIndex],&pInputBuffer[startIndex],numSamples,decimation);
    	}
    	else {
    		FIRFilterFFF::filterN(&pOutputBuffer[startIndex],&pInputBuffer[startIndex],numSamples);
    	}
    	threadRunning[5] = false;
    }

    void FIRFilterFFF_MT::runThread7(long startIndex,long numSamples) {
    	if (decimating()) {
    		long decIndex = calcDecimationIndex(startIndex);

    		FIRFilterFFF::filterNdec(&pOutputBuffer[decIndex],&pInputBuffer[startIndex],numSamples,decimation);
    	}
    	else {
    		FIRFilterFFF::filterN(&pOutputBuffer[startIndex],&pInputBuffer[startIndex],numSamples);
    	}
    	threadRunning[6] = false;
    }

    void FIRFilterFFF_MT::runThread8(long startIndex,long numSamples) {
    	if (decimating()) {
    		long decIndex = calcDecimationIndex(startIndex);

    		FIRFilterFFF::filterNdec(&pOutputBuffer[decIndex],&pInputBuffer[startIndex],numSamples,decimation);
    	}
    	else {
    		FIRFilterFFF::filterN(&pOutputBuffer[startIndex],&pInputBuffer[startIndex],numSamples);
    	}
    	threadRunning[7] = false;
    }

    void FIRFilterFFF_MT::runThread9(long startIndex,long numSamples) {
    	if (decimating()) {
    		long decIndex = calcDecimationIndex(startIndex);

    		FIRFilterFFF::filterNdec(&pOutputBuffer[decIndex],&pInputBuffer[startIndex],numSamples,decimation);
    	}
    	else {
    		FIRFilterFFF::filterN(&pOutputBuffer[startIndex],&pInputBuffer[startIndex],numSamples);
    	}
    	threadRunning[8] = false;
    }

    void FIRFilterFFF_MT::runThread10(long startIndex,long numSamples) {
    	if (decimating()) {
    		long decIndex = calcDecimationIndex(startIndex);

    		FIRFilterFFF::filterNdec(&pOutputBuffer[decIndex],&pInputBuffer[startIndex],numSamples,decimation);
    	}
    	else {
    		FIRFilterFFF::filterN(&pOutputBuffer[startIndex],&pInputBuffer[startIndex],numSamples);
    	}
    	threadRunning[9] = false;
    }

    void FIRFilterFFF_MT::runThread11(long startIndex,long numSamples) {
    	if (decimating()) {
    		long decIndex = calcDecimationIndex(startIndex);

    		FIRFilterFFF::filterNdec(&pOutputBuffer[decIndex],&pInputBuffer[startIndex],numSamples,decimation);
    	}
    	else {
    		FIRFilterFFF::filterN(&pOutputBuffer[startIndex],&pInputBuffer[startIndex],numSamples);
    	}
    	threadRunning[10] = false;
    }

    void FIRFilterFFF_MT::runThread12(long startIndex,long numSamples) {
    	if (decimating()) {
    		long decIndex = calcDecimationIndex(startIndex);

    		FIRFilterFFF::filterNdec(&pOutputBuffer[decIndex],&pInputBuffer[startIndex],numSamples,decimation);
    	}
    	else {
    		FIRFilterFFF::filterN(&pOutputBuffer[startIndex],&pInputBuffer[startIndex],numSamples);
    	}
    	threadRunning[11] = false;
    }

    void FIRFilterFFF_MT::runThread13(long startIndex,long numSamples) {
    	if (decimating()) {
    		long decIndex = calcDecimationIndex(startIndex);

    		FIRFilterFFF::filterNdec(&pOutputBuffer[decIndex],&pInputBuffer[startIndex],numSamples,decimation);
    	}
    	else {
    		FIRFilterFFF::filterN(&pOutputBuffer[startIndex],&pInputBuffer[startIndex],numSamples);
    	}
    	threadRunning[12] = false;
    }

    void FIRFilterFFF_MT::runThread14(long startIndex,long numSamples) {
    	if (decimating()) {
    		long decIndex = calcDecimationIndex(startIndex);

    		FIRFilterFFF::filterNdec(&pOutputBuffer[decIndex],&pInputBuffer[startIndex],numSamples,decimation);
    	}
    	else {
    		FIRFilterFFF::filterN(&pOutputBuffer[startIndex],&pInputBuffer[startIndex],numSamples);
    	}
    	threadRunning[13] = false;
    }

    void FIRFilterFFF_MT::runThread15(long startIndex,long numSamples) {
    	if (decimating()) {
    		long decIndex = calcDecimationIndex(startIndex);

    		FIRFilterFFF::filterNdec(&pOutputBuffer[decIndex],&pInputBuffer[startIndex],numSamples,decimation);
    	}
    	else {
    		FIRFilterFFF::filterN(&pOutputBuffer[startIndex],&pInputBuffer[startIndex],numSamples);
    	}
    	threadRunning[14] = false;
    }

    void FIRFilterFFF_MT::runThread16(long startIndex,long numSamples) {
    	if (decimating()) {
    		long decIndex = calcDecimationIndex(startIndex);

    		FIRFilterFFF::filterNdec(&pOutputBuffer[decIndex],&pInputBuffer[startIndex],numSamples,decimation);
    	}
    	else {
    		FIRFilterFFF::filterN(&pOutputBuffer[startIndex],&pInputBuffer[startIndex],numSamples);
    	}
    	threadRunning[15] = false;
    }


	// NOTE: This routine is expecting numSamples to be an integer multiple of the number of taps
	long FIRFilterFFF_MT::filterN(float *outputBuffer, const float *inputBuffer, long numSamples) {
		while (anyThreadRunning() || (threadReady == false))
			usleep(10);

		pInputBuffer = inputBuffer;
		pOutputBuffer = outputBuffer;

		long blockSize = numSamples / d_nthreads;
		long lastBlock = numSamples - (d_nthreads-1)*blockSize;
		threadReady = false;

		int curBlock;

		// std::cout << "Starting threads.  noutput_items = " << noutput_items << " Block size =" << blockSize << " last block=" << lastBlock << std::endl;

		for (int i=0;i<d_nthreads;i++) {
			threadRunning[i] = true;

			if (i<(d_nthreads-1))
				curBlock = blockSize;
			else
				curBlock = lastBlock;

			switch(i) {
			case 0:
	    		threads[i] = new boost::thread(boost::bind(&FIRFilterFFF_MT::runThread1, this,i*blockSize,curBlock));
				break;
			case 1:
	    		threads[i] = new boost::thread(boost::bind(&FIRFilterFFF_MT::runThread2, this,i*blockSize,curBlock));
				break;
			case 2:
	    		threads[i] = new boost::thread(boost::bind(&FIRFilterFFF_MT::runThread3, this,i*blockSize,curBlock));
				break;
			case 3:
	    		threads[i] = new boost::thread(boost::bind(&FIRFilterFFF_MT::runThread4, this,i*blockSize,curBlock));
				break;
			case 4:
	    		threads[i] = new boost::thread(boost::bind(&FIRFilterFFF_MT::runThread5, this,i*blockSize,curBlock));
				break;
			case 5:
	    		threads[i] = new boost::thread(boost::bind(&FIRFilterFFF_MT::runThread6, this,i*blockSize,curBlock));
				break;
			case 6:
	    		threads[i] = new boost::thread(boost::bind(&FIRFilterFFF_MT::runThread7, this,i*blockSize,curBlock));
				break;
			case 7:
	    		threads[i] = new boost::thread(boost::bind(&FIRFilterFFF_MT::runThread8, this,i*blockSize,curBlock));
				break;
			case 8:
	    		threads[i] = new boost::thread(boost::bind(&FIRFilterFFF_MT::runThread9, this,i*blockSize,curBlock));
				break;
			case 9:
	    		threads[i] = new boost::thread(boost::bind(&FIRFilterFFF_MT::runThread10, this,i*blockSize,curBlock));
				break;
			case 10:
	    		threads[i] = new boost::thread(boost::bind(&FIRFilterFFF_MT::runThread11, this,i*blockSize,curBlock));
				break;
			case 11:
	    		threads[i] = new boost::thread(boost::bind(&FIRFilterFFF_MT::runThread12, this,i*blockSize,curBlock));
				break;
			case 12:
	    		threads[i] = new boost::thread(boost::bind(&FIRFilterFFF_MT::runThread13, this,i*blockSize,curBlock));
				break;
			case 13:
	    		threads[i] = new boost::thread(boost::bind(&FIRFilterFFF_MT::runThread14, this,i*blockSize,curBlock));
				break;
			case 14:
	    		threads[i] = new boost::thread(boost::bind(&FIRFilterFFF_MT::runThread15, this,i*blockSize,curBlock));
				break;
			case 15:
	    		threads[i] = new boost::thread(boost::bind(&FIRFilterFFF_MT::runThread16, this,i*blockSize,curBlock));
				break;
			}
		}

		// std::cout << "Waiting for threads to finish." << std::endl;
		while (anyThreadRunning())
			usleep(10);

		// std::cout << "Threads finished." << std::endl;
		for (int i=0;i<d_nthreads;i++) {
			delete threads[i];
			threads[i] = NULL;
		}

		threadReady = true;
		return numSamples;
	}

	// OutputBuffer here should be at least numSamples/decimation in length
	long FIRFilterFFF_MT::filterNdec(float *outputBuffer, const float *inputBuffer, long numSamples, int decimation) {
		while (anyThreadRunning() || (threadReady == false))
			usleep(10);

		pInputBuffer = inputBuffer;
		pOutputBuffer = outputBuffer;

		numSamples = numSamples * decimation;
		long blockSize = calcDecimationBlockSize(numSamples);

		long lastBlock = numSamples - (d_nthreads-1)*blockSize;
		threadReady = false;

		int curBlock;

		// std::cout << "Starting threads.  numSamples = " << numSamples << " Block size =" << blockSize << " last block=" << lastBlock << std::endl;

		for (int i=0;i<d_nthreads;i++) {
			threadRunning[i] = true;

			if (i<(d_nthreads-1))
				curBlock = blockSize;
			else
				curBlock = lastBlock;

			switch(i) {
			case 0:
	    		threads[i] = new boost::thread(boost::bind(&FIRFilterFFF_MT::runThread1, this,i*blockSize,curBlock));
				break;
			case 1:
	    		threads[i] = new boost::thread(boost::bind(&FIRFilterFFF_MT::runThread2, this,i*blockSize,curBlock));
				break;
			case 2:
	    		threads[i] = new boost::thread(boost::bind(&FIRFilterFFF_MT::runThread3, this,i*blockSize,curBlock));
				break;
			case 3:
	    		threads[i] = new boost::thread(boost::bind(&FIRFilterFFF_MT::runThread4, this,i*blockSize,curBlock));
				break;
			case 4:
	    		threads[i] = new boost::thread(boost::bind(&FIRFilterFFF_MT::runThread5, this,i*blockSize,curBlock));
				break;
			case 5:
	    		threads[i] = new boost::thread(boost::bind(&FIRFilterFFF_MT::runThread6, this,i*blockSize,curBlock));
				break;
			case 6:
	    		threads[i] = new boost::thread(boost::bind(&FIRFilterFFF_MT::runThread7, this,i*blockSize,curBlock));
				break;
			case 7:
	    		threads[i] = new boost::thread(boost::bind(&FIRFilterFFF_MT::runThread8, this,i*blockSize,curBlock));
				break;
			case 8:
	    		threads[i] = new boost::thread(boost::bind(&FIRFilterFFF_MT::runThread9, this,i*blockSize,curBlock));
				break;
			case 9:
	    		threads[i] = new boost::thread(boost::bind(&FIRFilterFFF_MT::runThread10, this,i*blockSize,curBlock));
				break;
			case 10:
	    		threads[i] = new boost::thread(boost::bind(&FIRFilterFFF_MT::runThread11, this,i*blockSize,curBlock));
				break;
			case 11:
	    		threads[i] = new boost::thread(boost::bind(&FIRFilterFFF_MT::runThread12, this,i*blockSize,curBlock));
				break;
			case 12:
	    		threads[i] = new boost::thread(boost::bind(&FIRFilterFFF_MT::runThread13, this,i*blockSize,curBlock));
				break;
			case 13:
	    		threads[i] = new boost::thread(boost::bind(&FIRFilterFFF_MT::runThread14, this,i*blockSize,curBlock));
				break;
			case 14:
	    		threads[i] = new boost::thread(boost::bind(&FIRFilterFFF_MT::runThread15, this,i*blockSize,curBlock));
				break;
			case 15:
	    		threads[i] = new boost::thread(boost::bind(&FIRFilterFFF_MT::runThread16, this,i*blockSize,curBlock));
				break;
			}
		}

		// std::cout << "Waiting for threads to finish." << std::endl;
		while (anyThreadRunning())
			usleep(10);

		// std::cout << "Threads finished." << std::endl;
		for (int i=0;i<d_nthreads;i++) {
			delete threads[i];
			threads[i] = NULL;
		}

		threadReady = true;

		return numSamples;
	}

    // ------------------------------------------------
    // Multi-threaded filter, complex data, float taps
    // ------------------------------------------------

    FIRFilterCCC_MT::FIRFilterCCC_MT(int nthreads):MTBase(nthreads),FIRFilterCCC() {

    }

    FIRFilterCCC_MT::FIRFilterCCC_MT(const std::vector<gr_complex>& newTaps, int nthreads):MTBase(nthreads),FIRFilterCCC(newTaps) {

	}

    FIRFilterCCC_MT::~FIRFilterCCC_MT() {

	}

    void FIRFilterCCC_MT::runThread1(long startIndex,long numSamples) {
    	if (decimating()) {
    		long decIndex = calcDecimationIndex(startIndex);

    		FIRFilterCCC::filterNdec(&pOutputBuffer[decIndex],&pInputBuffer[startIndex],numSamples,decimation);
    	}
    	else {
    		FIRFilterCCC::filterN(&pOutputBuffer[startIndex],&pInputBuffer[startIndex],numSamples);
    	}
    	threadRunning[0] = false;
    }

    void FIRFilterCCC_MT::runThread2(long startIndex,long numSamples) {
    	if (decimating()) {
    		long decIndex = calcDecimationIndex(startIndex);

    		FIRFilterCCC::filterNdec(&pOutputBuffer[decIndex],&pInputBuffer[startIndex],numSamples,decimation);
    	}
    	else {
    		FIRFilterCCC::filterN(&pOutputBuffer[startIndex],&pInputBuffer[startIndex],numSamples);
    	}
    	threadRunning[1] = false;
    }

    void FIRFilterCCC_MT::runThread3(long startIndex,long numSamples) {
    	if (decimating()) {
    		long decIndex = calcDecimationIndex(startIndex);

    		FIRFilterCCC::filterNdec(&pOutputBuffer[decIndex],&pInputBuffer[startIndex],numSamples,decimation);
    	}
    	else {
    		FIRFilterCCC::filterN(&pOutputBuffer[startIndex],&pInputBuffer[startIndex],numSamples);
    	}
    	threadRunning[2] = false;
    }

    void FIRFilterCCC_MT::runThread4(long startIndex,long numSamples) {
    	if (decimating()) {
    		long decIndex = calcDecimationIndex(startIndex);

    		FIRFilterCCC::filterNdec(&pOutputBuffer[decIndex],&pInputBuffer[startIndex],numSamples,decimation);
    	}
    	else {
    		FIRFilterCCC::filterN(&pOutputBuffer[startIndex],&pInputBuffer[startIndex],numSamples);
    	}
    	threadRunning[3] = false;
    }

    void FIRFilterCCC_MT::runThread5(long startIndex,long numSamples) {
    	if (decimating()) {
    		long decIndex = calcDecimationIndex(startIndex);

    		FIRFilterCCC::filterNdec(&pOutputBuffer[decIndex],&pInputBuffer[startIndex],numSamples,decimation);
    	}
    	else {
    		FIRFilterCCC::filterN(&pOutputBuffer[startIndex],&pInputBuffer[startIndex],numSamples);
    	}
    	threadRunning[4] = false;
    }

    void FIRFilterCCC_MT::runThread6(long startIndex,long numSamples) {
    	if (decimating()) {
    		long decIndex = calcDecimationIndex(startIndex);

    		FIRFilterCCC::filterNdec(&pOutputBuffer[decIndex],&pInputBuffer[startIndex],numSamples,decimation);
    	}
    	else {
    		FIRFilterCCC::filterN(&pOutputBuffer[startIndex],&pInputBuffer[startIndex],numSamples);
    	}
    	threadRunning[5] = false;
    }

    void FIRFilterCCC_MT::runThread7(long startIndex,long numSamples) {
    	if (decimating()) {
    		long decIndex = calcDecimationIndex(startIndex);

    		FIRFilterCCC::filterNdec(&pOutputBuffer[decIndex],&pInputBuffer[startIndex],numSamples,decimation);
    	}
    	else {
    		FIRFilterCCC::filterN(&pOutputBuffer[startIndex],&pInputBuffer[startIndex],numSamples);
    	}
    	threadRunning[6] = false;
    }

    void FIRFilterCCC_MT::runThread8(long startIndex,long numSamples) {
    	if (decimating()) {
    		long decIndex = calcDecimationIndex(startIndex);

    		FIRFilterCCC::filterNdec(&pOutputBuffer[decIndex],&pInputBuffer[startIndex],numSamples,decimation);
    	}
    	else {
    		FIRFilterCCC::filterN(&pOutputBuffer[startIndex],&pInputBuffer[startIndex],numSamples);
    	}
    	threadRunning[7] = false;
    }

    void FIRFilterCCC_MT::runThread9(long startIndex,long numSamples) {
    	if (decimating()) {
    		long decIndex = calcDecimationIndex(startIndex);

    		FIRFilterCCC::filterNdec(&pOutputBuffer[decIndex],&pInputBuffer[startIndex],numSamples,decimation);
    	}
    	else {
    		FIRFilterCCC::filterN(&pOutputBuffer[startIndex],&pInputBuffer[startIndex],numSamples);
    	}
    	threadRunning[8] = false;
    }

    void FIRFilterCCC_MT::runThread10(long startIndex,long numSamples) {
    	if (decimating()) {
    		long decIndex = calcDecimationIndex(startIndex);

    		FIRFilterCCC::filterNdec(&pOutputBuffer[decIndex],&pInputBuffer[startIndex],numSamples,decimation);
    	}
    	else {
    		FIRFilterCCC::filterN(&pOutputBuffer[startIndex],&pInputBuffer[startIndex],numSamples);
    	}
    	threadRunning[9] = false;
    }

    void FIRFilterCCC_MT::runThread11(long startIndex,long numSamples) {
    	if (decimating()) {
    		long decIndex = calcDecimationIndex(startIndex);

    		FIRFilterCCC::filterNdec(&pOutputBuffer[decIndex],&pInputBuffer[startIndex],numSamples,decimation);
    	}
    	else {
    		FIRFilterCCC::filterN(&pOutputBuffer[startIndex],&pInputBuffer[startIndex],numSamples);
    	}
    	threadRunning[10] = false;
    }

    void FIRFilterCCC_MT::runThread12(long startIndex,long numSamples) {
    	if (decimating()) {
    		long decIndex = calcDecimationIndex(startIndex);

    		FIRFilterCCC::filterNdec(&pOutputBuffer[decIndex],&pInputBuffer[startIndex],numSamples,decimation);
    	}
    	else {
    		FIRFilterCCC::filterN(&pOutputBuffer[startIndex],&pInputBuffer[startIndex],numSamples);
    	}
    	threadRunning[11] = false;
    }

    void FIRFilterCCC_MT::runThread13(long startIndex,long numSamples) {
    	if (decimating()) {
    		long decIndex = calcDecimationIndex(startIndex);

    		FIRFilterCCC::filterNdec(&pOutputBuffer[decIndex],&pInputBuffer[startIndex],numSamples,decimation);
    	}
    	else {
    		FIRFilterCCC::filterN(&pOutputBuffer[startIndex],&pInputBuffer[startIndex],numSamples);
    	}
    	threadRunning[12] = false;
    }

    void FIRFilterCCC_MT::runThread14(long startIndex,long numSamples) {
    	if (decimating()) {
    		long decIndex = calcDecimationIndex(startIndex);

    		FIRFilterCCC::filterNdec(&pOutputBuffer[decIndex],&pInputBuffer[startIndex],numSamples,decimation);
    	}
    	else {
    		FIRFilterCCC::filterN(&pOutputBuffer[startIndex],&pInputBuffer[startIndex],numSamples);
    	}
    	threadRunning[13] = false;
    }

    void FIRFilterCCC_MT::runThread15(long startIndex,long numSamples) {
    	if (decimating()) {
    		long decIndex = calcDecimationIndex(startIndex);

    		FIRFilterCCC::filterNdec(&pOutputBuffer[decIndex],&pInputBuffer[startIndex],numSamples,decimation);
    	}
    	else {
    		FIRFilterCCC::filterN(&pOutputBuffer[startIndex],&pInputBuffer[startIndex],numSamples);
    	}
    	threadRunning[14] = false;
    }

    void FIRFilterCCC_MT::runThread16(long startIndex,long numSamples) {
    	if (decimating()) {
    		long decIndex = calcDecimationIndex(startIndex);

    		FIRFilterCCC::filterNdec(&pOutputBuffer[decIndex],&pInputBuffer[startIndex],numSamples,decimation);
    	}
    	else {
    		FIRFilterCCC::filterN(&pOutputBuffer[startIndex],&pInputBuffer[startIndex],numSamples);
    	}
    	threadRunning[15] = false;
    }

	// NOTE: This routine is expecting numSamples to be an integer multiple of the number of taps
	long FIRFilterCCC_MT::filterN(gr_complex *outputBuffer, const gr_complex *inputBuffer, long numSamples) {
		while (anyThreadRunning() || (threadReady == false))
			usleep(10);

		pInputBuffer = inputBuffer;
		pOutputBuffer = outputBuffer;

		long blockSize = numSamples / d_nthreads;
		long lastBlock = numSamples - (d_nthreads-1)*blockSize;
		threadReady = false;

		int curBlock;

		// std::cout << "Starting threads.  numSamples = " << numSamples << " Block size =" << blockSize << " last block=" << lastBlock << std::endl;

		for (int i=0;i<d_nthreads;i++) {
			threadRunning[i] = true;

			if (i<(d_nthreads-1))
				curBlock = blockSize;
			else
				curBlock = lastBlock;

			switch(i) {
			case 0:
	    		threads[i] = new boost::thread(boost::bind(&FIRFilterCCC_MT::runThread1, this,i*blockSize,curBlock));
				break;
			case 1:
	    		threads[i] = new boost::thread(boost::bind(&FIRFilterCCC_MT::runThread2, this,i*blockSize,curBlock));
				break;
			case 2:
	    		threads[i] = new boost::thread(boost::bind(&FIRFilterCCC_MT::runThread3, this,i*blockSize,curBlock));
				break;
			case 3:
	    		threads[i] = new boost::thread(boost::bind(&FIRFilterCCC_MT::runThread4, this,i*blockSize,curBlock));
				break;
			case 4:
	    		threads[i] = new boost::thread(boost::bind(&FIRFilterCCC_MT::runThread5, this,i*blockSize,curBlock));
				break;
			case 5:
	    		threads[i] = new boost::thread(boost::bind(&FIRFilterCCC_MT::runThread6, this,i*blockSize,curBlock));
				break;
			case 6:
	    		threads[i] = new boost::thread(boost::bind(&FIRFilterCCC_MT::runThread7, this,i*blockSize,curBlock));
				break;
			case 7:
	    		threads[i] = new boost::thread(boost::bind(&FIRFilterCCC_MT::runThread8, this,i*blockSize,curBlock));
				break;
			case 8:
	    		threads[i] = new boost::thread(boost::bind(&FIRFilterCCC_MT::runThread9, this,i*blockSize,curBlock));
				break;
			case 9:
	    		threads[i] = new boost::thread(boost::bind(&FIRFilterCCC_MT::runThread10, this,i*blockSize,curBlock));
				break;
			case 10:
	    		threads[i] = new boost::thread(boost::bind(&FIRFilterCCC_MT::runThread11, this,i*blockSize,curBlock));
				break;
			case 11:
	    		threads[i] = new boost::thread(boost::bind(&FIRFilterCCC_MT::runThread12, this,i*blockSize,curBlock));
				break;
			case 12:
	    		threads[i] = new boost::thread(boost::bind(&FIRFilterCCC_MT::runThread13, this,i*blockSize,curBlock));
				break;
			case 13:
	    		threads[i] = new boost::thread(boost::bind(&FIRFilterCCC_MT::runThread14, this,i*blockSize,curBlock));
				break;
			case 14:
	    		threads[i] = new boost::thread(boost::bind(&FIRFilterCCC_MT::runThread15, this,i*blockSize,curBlock));
				break;
			case 15:
	    		threads[i] = new boost::thread(boost::bind(&FIRFilterCCC_MT::runThread16, this,i*blockSize,curBlock));
				break;
			}
		}

		// std::cout << "Waiting for threads to finish." << std::endl;
		while (anyThreadRunning())
			usleep(10);

		//std::cout << "Threads finished." << std::endl;
		for (int i=0;i<d_nthreads;i++) {
			delete threads[i];
			threads[i] = NULL;
		}

		threadReady = true;

		return numSamples;
	}

	// OutputBuffer here should be at least numSamples/decimation in length
	long FIRFilterCCC_MT::filterNdec(gr_complex *outputBuffer, const gr_complex *inputBuffer, long numSamples, int decimation) {
		while (anyThreadRunning() || (threadReady == false))
			usleep(10);

		pInputBuffer = inputBuffer;
		pOutputBuffer = outputBuffer;

		numSamples = numSamples * decimation;
		long blockSize = calcDecimationBlockSize(numSamples);

		long lastBlock = numSamples - (d_nthreads-1)*blockSize;
		threadReady = false;

		int curBlock;

		// std::cout << "Starting threads.  numSamples = " << numSamples << " Block size =" << blockSize << " last block=" << lastBlock << std::endl;

		for (int i=0;i<d_nthreads;i++) {
			threadRunning[i] = true;

			if (i<(d_nthreads-1))
				curBlock = blockSize;
			else
				curBlock = lastBlock;

			switch(i) {
			case 0:
	    		threads[i] = new boost::thread(boost::bind(&FIRFilterCCC_MT::runThread1, this,i*blockSize,curBlock));
				break;
			case 1:
	    		threads[i] = new boost::thread(boost::bind(&FIRFilterCCC_MT::runThread2, this,i*blockSize,curBlock));
				break;
			case 2:
	    		threads[i] = new boost::thread(boost::bind(&FIRFilterCCC_MT::runThread3, this,i*blockSize,curBlock));
				break;
			case 3:
	    		threads[i] = new boost::thread(boost::bind(&FIRFilterCCC_MT::runThread4, this,i*blockSize,curBlock));
				break;
			case 4:
	    		threads[i] = new boost::thread(boost::bind(&FIRFilterCCC_MT::runThread5, this,i*blockSize,curBlock));
				break;
			case 5:
	    		threads[i] = new boost::thread(boost::bind(&FIRFilterCCC_MT::runThread6, this,i*blockSize,curBlock));
				break;
			case 6:
	    		threads[i] = new boost::thread(boost::bind(&FIRFilterCCC_MT::runThread7, this,i*blockSize,curBlock));
				break;
			case 7:
	    		threads[i] = new boost::thread(boost::bind(&FIRFilterCCC_MT::runThread8, this,i*blockSize,curBlock));
				break;
			case 8:
	    		threads[i] = new boost::thread(boost::bind(&FIRFilterCCC_MT::runThread9, this,i*blockSize,curBlock));
				break;
			case 9:
	    		threads[i] = new boost::thread(boost::bind(&FIRFilterCCC_MT::runThread10, this,i*blockSize,curBlock));
				break;
			case 10:
	    		threads[i] = new boost::thread(boost::bind(&FIRFilterCCC_MT::runThread11, this,i*blockSize,curBlock));
				break;
			case 11:
	    		threads[i] = new boost::thread(boost::bind(&FIRFilterCCC_MT::runThread12, this,i*blockSize,curBlock));
				break;
			case 12:
	    		threads[i] = new boost::thread(boost::bind(&FIRFilterCCC_MT::runThread13, this,i*blockSize,curBlock));
				break;
			case 13:
	    		threads[i] = new boost::thread(boost::bind(&FIRFilterCCC_MT::runThread14, this,i*blockSize,curBlock));
				break;
			case 14:
	    		threads[i] = new boost::thread(boost::bind(&FIRFilterCCC_MT::runThread15, this,i*blockSize,curBlock));
				break;
			case 15:
	    		threads[i] = new boost::thread(boost::bind(&FIRFilterCCC_MT::runThread16, this,i*blockSize,curBlock));
				break;
			}
		}

		// std::cout << "Waiting for threads to finish." << std::endl;
		while (anyThreadRunning())
			usleep(10);

		// std::cout << "Threads finished." << std::endl;
		for (int i=0;i<d_nthreads;i++) {
			delete threads[i];
			threads[i] = NULL;
		}

		threadReady = true;

		return numSamples;
	}

  } // end lfast
} // end gr



