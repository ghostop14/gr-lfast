/* -*- c++ -*- */
/*
 * Copyright 2012 Free Software Foundation, Inc.
 *
 * This file is part of GNU Radio
 *
 * GNU Radio is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * GNU Radio is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNU Radio; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <iostream>
#include <fstream>
#include <iomanip>
#include <boost/algorithm/string/replace.hpp>

#include <chrono>
#include "costas2_impl.h"
#include "costas4_impl.h"
#include "agc_fast_impl.h"
#include "CC2F2ByteVector_impl.h"
#include "nlog10volk_impl.h"
#include "quad_demod_volk_impl.h"
#include "MTFIRFilterCCF_impl.h"

#include "fir_filter_lfast.h"

int largeBlockSize=8192;
int ntaps=241;
int maxThreads=8;

using namespace gr::lfast;

class comma_numpunct : public std::numpunct<char>
{
  protected:
    virtual char do_thousands_sep() const
    {
        return ',';
    }

    virtual std::string do_grouping() const
    {
        return "\03";
    }
};

void timeFilter() {
	std::cout << "----------------------------------------------------------" << std::endl;

	int localblocksize=largeBlockSize;

	int decimation = 1;

	std::cout << "Testing FIR filter with complex data and float taps with " << ntaps << " taps, " <<
			localblocksize << " samples..." << std::endl;

	std::vector<float> filtertaps;

	for (int i=0;i<ntaps;i++) {
		filtertaps.push_back(((float)i)/1000.0);
	}

	MTFIRFilterCCF_impl *test;
	test = new MTFIRFilterCCF_impl(decimation,filtertaps,1);

	int i;
	std::chrono::time_point<std::chrono::steady_clock> start, end;
	std::chrono::duration<double> elapsed_seconds = end-start;
	std::vector<int> ninitems;


	std::vector<gr_complex> inputItems;
	std::vector<gr_complex> outputItems;
	std::vector<const void *> inputPointers;
	std::vector<void *> outputPointers;

	gr_complex grZero(0.0,0.0);
	gr_complex newComplex(1.0,0.5);

	// Need to give it the tail buffer
	int requiredSize=localblocksize + ntaps; // have to include history.

	for (i=0;i<requiredSize;i++) {
		inputItems.push_back(gr_complex(0.5f,0.25f));
		outputItems.push_back(grZero);
	}

	inputPointers.push_back((const void *)&inputItems[0]);
	outputPointers.push_back((void *)&outputItems[0]);

	ninitems.push_back(localblocksize);

	int noutputitems;
	int iterations = 100;
	float elapsed_time,throughput_original,throughput;

	noutputitems = test->work_original(localblocksize,inputPointers,outputPointers);
/*
	// --------------------
	// Debug Code
	test->setDecimation(4);
	test->setThreads(2);
	noutputitems = test->work_test(localblocksize/4,inputPointers,outputPointers);

	test->setDecimation(1);
	return;
	// --------------------
*/
	start = std::chrono::steady_clock::now();
	// make iterations calls to get average.
	for (i=0;i<iterations;i++) {
		noutputitems = test->work_original(localblocksize,inputPointers,outputPointers);
	}
	end = std::chrono::steady_clock::now();

	elapsed_seconds = end-start;

	elapsed_time = elapsed_seconds.count()/(float)iterations;
	throughput_original = localblocksize / elapsed_time;

	std::cout << "Original Code Run Time:   " << std::fixed << std::setw(11)
    << std::setprecision(6) << elapsed_time << " s  (" << throughput_original << " sps)" << std::endl;

	// -----------------------------------------------------------------------------
	// New Code
	// -----------------------------------------------------------------------------

	int fastestThread=1;
	float fastestTime = -99999999.0;
	float floatOriginalSPS = throughput_original;
	float fastestSPS = 0.0;

	for (int curThread=1;curThread<=maxThreads;curThread++) {
		test->setThreads(curThread);
		// Get first call out of the way.
		noutputitems = test->work_test(localblocksize,inputPointers,outputPointers);

		start = std::chrono::steady_clock::now();
		// make iterations calls to get average.
		for (i=0;i<iterations;i++) {
			noutputitems = test->work_test(localblocksize,inputPointers,outputPointers);
		}
		end = std::chrono::steady_clock::now();

		elapsed_seconds = end-start;

		elapsed_time = elapsed_seconds.count()/(float)iterations;
		throughput = localblocksize / elapsed_time;

		std::cout << "LFAST Code Run Time [" << curThread << " threads]:   " << std::fixed << std::setw(11)
	    << std::setprecision(6) << elapsed_time << " s  (" << throughput << " sps)" << std::endl;

		float faster = (throughput / throughput_original - 1) * 100.0;
		std::cout << curThread << "-thread Speedup:   " << std::fixed << std::setw(11)
	    << std::setprecision(2) << faster << "% faster" << std::endl;

		if (faster > fastestTime) {
			fastestTime = faster;
			fastestThread = curThread;
			fastestSPS = throughput;
		}
	}

	std::cout << std::endl << "Fastest filter thread performance:" << std::endl <<
			"Number of Samples in test case (can be varied with command-line parameter): " << largeBlockSize << std::endl <<
			"Number of taps in filter (can be varied with command-line parameter): " << ntaps << std::endl <<
			"Fastest Thread Count: " << fastestThread << std::endl <<
			"Original SPS: " << floatOriginalSPS << std::endl <<
			"Fastest SPS: " << fastestSPS << std::endl <<
			"Speedup: " << fastestTime << "% faster" << std::endl;

	delete test;
}

void timeCC2Vector() {
	std::cout << "----------------------------------------------------------" << std::endl;

	int localblocksize=largeBlockSize;

	std::cout << "Testing Complex->Real->Char->Vector with " << localblocksize << " samples..." << std::endl;

	CC2F2ByteVector_impl *test;
	test = new CC2F2ByteVector_impl(127,64,1);
	test->setBufferLength(localblocksize);

	int i;
	std::chrono::time_point<std::chrono::steady_clock> start, end;
	std::chrono::duration<double> elapsed_seconds = end-start;
	std::vector<int> ninitems;


	std::vector<gr_complex> inputItems;
	std::vector<char> outputItems;
	std::vector<char> outputItems2;
	std::vector<const void *> inputPointers;
	std::vector<void *> outputPointers;
	std::vector<void *> outputPointers2;

	gr_complex grZero(0.0,0.0);
	gr_complex newComplex(1.0,0.5);

	for (i=0;i<localblocksize;i++) {
		inputItems.push_back(gr_complex(0.5f,0.25f));
		outputItems.push_back(0);
		outputItems2.push_back(0);
	}

	inputPointers.push_back((const void *)&inputItems[0]);
	outputPointers.push_back((void *)&outputItems[0]);
	outputPointers2.push_back((void *)&outputItems2[0]);

	ninitems.push_back(localblocksize);

	int noutputitems;
	int iterations = 100;
	float elapsed_time,throughput_original,throughput;

	noutputitems = test->work_original(localblocksize,inputPointers,outputPointers);

	start = std::chrono::steady_clock::now();
	// make iterations calls to get average.
	for (i=0;i<iterations;i++) {
		noutputitems = test->work_original(localblocksize,inputPointers,outputPointers);
	}
	end = std::chrono::steady_clock::now();

	elapsed_seconds = end-start;

	elapsed_time = elapsed_seconds.count()/(float)iterations;
	throughput_original = localblocksize / elapsed_time;

	std::cout << "Original Code Run Time:   " << std::fixed << std::setw(11)
    << std::setprecision(6) << elapsed_time << " s  (" << throughput_original << " sps)" << std::endl;

	// New Code
	noutputitems = test->work_test(localblocksize,inputPointers,outputPointers);
	// -----------------------------
	start = std::chrono::steady_clock::now();
	// make iterations calls to get average.
	for (i=0;i<iterations;i++) {
		noutputitems = test->work_test(localblocksize,inputPointers,outputPointers);
	}
	end = std::chrono::steady_clock::now();

	elapsed_seconds = end-start;

	elapsed_time = elapsed_seconds.count()/(float)iterations;
	throughput = localblocksize / elapsed_time;

	std::cout << "LFAST Code Run Time:   " << std::fixed << std::setw(11)
    << std::setprecision(6) << elapsed_time << " s  (" << throughput << " sps)" << std::endl;

	float faster = (throughput / throughput_original - 1) * 100.0;
	std::cout << "Speedup:   " << std::fixed << std::setw(11)
    << std::setprecision(2) << faster << "% faster" << std::endl << std::endl;

	delete test;
}


void timeAGC() {
	std::cout << "----------------------------------------------------------" << std::endl;

	int localblocksize=largeBlockSize;

	std::cout << "Testing AGC with " << localblocksize << " samples..." << std::endl;

	agc_fast_impl *test;
	test = new agc_fast_impl(0.01,0.5,0.5);
	test->set_max_gain(4000.0);


	int i;
	std::chrono::time_point<std::chrono::steady_clock> start, end;
	std::chrono::duration<double> elapsed_seconds = end-start;
	std::vector<int> ninitems;


	std::vector<gr_complex> inputItems;
	std::vector<gr_complex> outputItems;
	std::vector<gr_complex> outputItems2;
	std::vector<const void *> inputPointers;
	std::vector<void *> outputPointers;
	std::vector<void *> outputPointers2;

	gr_complex grZero(0.0,0.0);
	gr_complex newComplex(1.0,0.5);

	for (i=0;i<localblocksize;i++) {
		inputItems.push_back(gr_complex(1.0f,0.5f));
		outputItems.push_back(grZero);
		outputItems2.push_back(grZero);
	}

	inputPointers.push_back((const void *)&inputItems[0]);
	outputPointers.push_back((void *)&outputItems[0]);
	outputPointers2.push_back((void *)&outputItems2[0]);

	ninitems.push_back(localblocksize);

	int noutputitems;
	int iterations = 100;
	float elapsed_time,throughput_original,throughput;

	noutputitems = test->work_original(localblocksize,inputPointers,outputPointers);

	start = std::chrono::steady_clock::now();
	// make iterations calls to get average.
	for (i=0;i<iterations;i++) {
		noutputitems = test->work_original(localblocksize,inputPointers,outputPointers);
	}
	end = std::chrono::steady_clock::now();

	elapsed_seconds = end-start;

	elapsed_time = elapsed_seconds.count()/(float)iterations;
	throughput_original = localblocksize / elapsed_time;

	std::cout << "Original Code Run Time:   " << std::fixed << std::setw(11)
    << std::setprecision(6) << elapsed_time << " s  (" << throughput_original << " sps)" << std::endl;

	// New Code
	// -----------------------------
	start = std::chrono::steady_clock::now();
	// make iterations calls to get average.
	for (i=0;i<iterations;i++) {
		noutputitems = test->work_test(localblocksize,inputPointers,outputPointers);
	}
	end = std::chrono::steady_clock::now();

	elapsed_seconds = end-start;

	elapsed_time = elapsed_seconds.count()/(float)iterations;
	throughput = localblocksize / elapsed_time;

	std::cout << "LFAST Code Run Time:   " << std::fixed << std::setw(11)
    << std::setprecision(6) << elapsed_time << " s  (" << throughput << " sps)" << std::endl;

	float faster = (throughput / throughput_original - 1) * 100.0;
	std::cout << "Speedup:   " << std::fixed << std::setw(11)
    << std::setprecision(2) << faster << "% faster" << std::endl << std::endl;

	delete test;
}

void timeLog10() {
	std::cout << "----------------------------------------------------------" << std::endl;

	int localblocksize=largeBlockSize;

	std::cout << "Testing volk nlog10+k with " << localblocksize << " samples..." << std::endl;

	nlog10volk_impl *test;
	test = new nlog10volk_impl(10.0,1,2);


	int i;
	std::chrono::time_point<std::chrono::steady_clock> start, end;
	std::chrono::duration<double> elapsed_seconds = end-start;
	std::vector<int> ninitems;


	std::vector<gr_complex> inputItems;
	std::vector<gr_complex> outputItems;
	std::vector<gr_complex> outputItems2;
	std::vector<const void *> inputPointers;
	std::vector<void *> outputPointers;
	std::vector<void *> outputPointers2;

	gr_complex grZero(0.0,0.0);
	gr_complex newComplex(1.0,0.5);

	for (i=0;i<localblocksize;i++) {
		inputItems.push_back(gr_complex(1.0f,0.5f));
		outputItems.push_back(grZero);
		outputItems2.push_back(grZero);
	}

	inputPointers.push_back((const void *)&inputItems[0]);
	outputPointers.push_back((void *)&outputItems[0]);
	outputPointers2.push_back((void *)&outputItems2[0]);

	ninitems.push_back(localblocksize);

	int noutputitems;
	int iterations = 100;
	float elapsed_time,throughput_original,throughput;

	noutputitems = test->work_original(localblocksize,inputPointers,outputPointers);

	start = std::chrono::steady_clock::now();
	// make iterations calls to get average.
	for (i=0;i<iterations;i++) {
		noutputitems = test->work_original(localblocksize,inputPointers,outputPointers);
	}
	end = std::chrono::steady_clock::now();

	elapsed_seconds = end-start;

	elapsed_time = elapsed_seconds.count()/(float)iterations;
	throughput_original = localblocksize / elapsed_time;

	std::cout << "Original Code Run Time:   " << std::fixed << std::setw(11)
    << std::setprecision(6) << elapsed_time << " s  (" << throughput_original << " sps)" << std::endl;

	// New Code
	// -----------------------------
	start = std::chrono::steady_clock::now();
	// make iterations calls to get average.
	for (i=0;i<iterations;i++) {
		noutputitems = test->work_test(localblocksize,inputPointers,outputPointers);
	}
	end = std::chrono::steady_clock::now();

	elapsed_seconds = end-start;

	elapsed_time = elapsed_seconds.count()/(float)iterations;
	throughput = localblocksize / elapsed_time;

	std::cout << "LFAST Code Run Time:   " << std::fixed << std::setw(11)
    << std::setprecision(6) << elapsed_time << " s  (" << throughput << " sps)" << std::endl;

	float faster = (throughput / throughput_original - 1) * 100.0;
	std::cout << "Speedup:   " << std::fixed << std::setw(11)
    << std::setprecision(2) << faster << "% faster" << std::endl << std::endl;

	delete test;
}

void timeQuadDemod() {
	std::cout << "----------------------------------------------------------" << std::endl;

	int localblocksize=largeBlockSize;

	std::cout << "Testing volk quad demod with " << localblocksize << " samples..." << std::endl;

	quad_demod_volk_impl *test;
	test = new quad_demod_volk_impl(10.0);


	int i;
	std::chrono::time_point<std::chrono::steady_clock> start, end;
	std::chrono::duration<double> elapsed_seconds = end-start;
	std::vector<int> ninitems;


	std::vector<gr_complex> inputItems;
	std::vector<gr_complex> outputItems;
	std::vector<gr_complex> outputItems2;
	std::vector<const void *> inputPointers;
	std::vector<void *> outputPointers;
	std::vector<void *> outputPointers2;

	gr_complex grZero(0.0,0.0);
	gr_complex newComplex(1.0,0.5);

	for (i=0;i<localblocksize;i++) {
		inputItems.push_back(gr_complex(1.0f,0.5f));
		outputItems.push_back(grZero);
		outputItems2.push_back(grZero);
	}

	inputPointers.push_back((const void *)&inputItems[0]);
	outputPointers.push_back((void *)&outputItems[0]);
	outputPointers2.push_back((void *)&outputItems2[0]);

	ninitems.push_back(localblocksize);

	int noutputitems;
	int iterations = 100;
	float elapsed_time,throughput_original,throughput;

	noutputitems = test->work_original(localblocksize,inputPointers,outputPointers);

	start = std::chrono::steady_clock::now();
	// make iterations calls to get average.
	for (i=0;i<iterations;i++) {
		noutputitems = test->work_original(localblocksize,inputPointers,outputPointers);
	}
	end = std::chrono::steady_clock::now();

	elapsed_seconds = end-start;

	elapsed_time = elapsed_seconds.count()/(float)iterations;
	throughput_original = localblocksize / elapsed_time;

	std::cout << "Original Code Run Time:   " << std::fixed << std::setw(11)
    << std::setprecision(6) << elapsed_time << " s  (" << throughput_original << " sps)" << std::endl;

	// New Code
	// -----------------------------
	start = std::chrono::steady_clock::now();
	// make iterations calls to get average.
	for (i=0;i<iterations;i++) {
		noutputitems = test->work_test(localblocksize,inputPointers,outputPointers);
	}
	end = std::chrono::steady_clock::now();

	elapsed_seconds = end-start;

	elapsed_time = elapsed_seconds.count()/(float)iterations;
	throughput = localblocksize / elapsed_time;

	std::cout << "LFAST Code Run Time:   " << std::fixed << std::setw(11)
    << std::setprecision(6) << elapsed_time << " s  (" << throughput << " sps)" << std::endl;

	float faster = (throughput / throughput_original - 1) * 100.0;
	std::cout << "Speedup:   " << std::fixed << std::setw(11)
    << std::setprecision(2) << faster << "% faster" << std::endl << std::endl;

	delete test;
}
void timeCostasLoop2() {
	int localblocksize=largeBlockSize;

	std::cout << "Testing 2nd order Costas Loop with " << localblocksize << " samples..." << std::endl;

	costas2_impl *test;
	test = new costas2_impl(0.00199,2,false);

	int i;
	std::chrono::time_point<std::chrono::steady_clock> start, end;
	std::chrono::duration<double> elapsed_seconds = end-start;
	std::vector<int> ninitems;


	std::vector<gr_complex> inputItems;
	std::vector<gr_complex> outputItems;
	std::vector<gr_complex> outputItems2;
	std::vector<const void *> inputPointers;
	std::vector<void *> outputPointers;
	std::vector<void *> outputPointers2;

	gr_complex grZero(0.0,0.0);
	gr_complex newComplex(1.0,0.5);

	for (i=0;i<localblocksize;i++) {
		inputItems.push_back(gr_complex(1.0f,0.5f));
		outputItems.push_back(grZero);
		outputItems2.push_back(grZero);
	}

	inputPointers.push_back((const void *)&inputItems[0]);
	outputPointers.push_back((void *)&outputItems[0]);
	outputPointers2.push_back((void *)&outputItems2[0]);

	ninitems.push_back(localblocksize);

	int noutputitems;
	int iterations = 100;
	float elapsed_time,throughput_original,throughput;

	noutputitems = test->work_original(localblocksize,inputPointers,outputPointers);

	start = std::chrono::steady_clock::now();
	// make iterations calls to get average.
	for (i=0;i<iterations;i++) {
		noutputitems = test->work_original(localblocksize,inputPointers,outputPointers);
	}
	end = std::chrono::steady_clock::now();

	elapsed_seconds = end-start;

	elapsed_time = elapsed_seconds.count()/(float)iterations;
	throughput_original = localblocksize / elapsed_time;

	std::cout << "Original Code Run Time:   " << std::fixed << std::setw(11)
    << std::setprecision(6) << elapsed_time << " s  (" << throughput_original << " sps)" << std::endl;

	// New Code
	// -----------------------------
	start = std::chrono::steady_clock::now();
	// make iterations calls to get average.
	for (i=0;i<iterations;i++) {
		noutputitems = test->work_test(localblocksize,inputPointers,outputPointers);
	}
	end = std::chrono::steady_clock::now();

	elapsed_seconds = end-start;

	elapsed_time = elapsed_seconds.count()/(float)iterations;
	throughput = localblocksize / elapsed_time;

	std::cout << "LFAST Code Run Time:   " << std::fixed << std::setw(11)
    << std::setprecision(6) << elapsed_time << " s  (" << throughput << " sps)" << std::endl;

	float faster = (throughput / throughput_original - 1) * 100.0;
	std::cout << "Speedup:   " << std::fixed << std::setw(11)
    << std::setprecision(2) << faster << "% faster" << std::endl << std::endl;

	delete test;
}

void timeCostasLoop4() {
	int localblocksize=largeBlockSize;

	std::cout << "Testing 4th order Costas Loop with " << localblocksize << " samples..." << std::endl;

	costas4_impl *test;
	test = new costas4_impl(0.00199,2,false);

	int i;
	std::chrono::time_point<std::chrono::steady_clock> start, end;
	std::chrono::duration<double> elapsed_seconds = end-start;
	std::vector<int> ninitems;


	std::vector<gr_complex> inputItems;
	std::vector<gr_complex> outputItems;
	std::vector<gr_complex> outputItems2;
	std::vector<const void *> inputPointers;
	std::vector<void *> outputPointers;
	std::vector<void *> outputPointers2;

	gr_complex grZero(0.0,0.0);
	gr_complex newComplex(1.0,0.5);

	for (i=0;i<localblocksize;i++) {
		inputItems.push_back(gr_complex(1.0f,0.5f));
		outputItems.push_back(grZero);
		outputItems2.push_back(grZero);
	}

	inputPointers.push_back((const void *)&inputItems[0]);
	outputPointers.push_back((void *)&outputItems[0]);
	outputPointers2.push_back((void *)&outputItems2[0]);

	ninitems.push_back(localblocksize);

	int noutputitems;
	int iterations = 100;
	float elapsed_time,throughput_original,throughput;

	noutputitems = test->work_original(localblocksize,inputPointers,outputPointers);

	start = std::chrono::steady_clock::now();
	// make iterations calls to get average.
	for (i=0;i<iterations;i++) {
		noutputitems = test->work_original(localblocksize,inputPointers,outputPointers);
	}
	end = std::chrono::steady_clock::now();

	elapsed_seconds = end-start;

	elapsed_time = elapsed_seconds.count()/(float)iterations;
	throughput_original = localblocksize / elapsed_time;

	std::cout << "Original Code Run Time:   " << std::fixed << std::setw(11)
    << std::setprecision(6) << elapsed_time << " s  (" << throughput_original << " sps)" << std::endl;

	// New Code
	// -----------------------------
	start = std::chrono::steady_clock::now();
	// make iterations calls to get average.
	for (i=0;i<iterations;i++) {
		noutputitems = test->work_test(localblocksize,inputPointers,outputPointers);
	}
	end = std::chrono::steady_clock::now();

	elapsed_seconds = end-start;

	elapsed_time = elapsed_seconds.count()/(float)iterations;
	throughput = localblocksize / elapsed_time;

	std::cout << "LFAST Code Run Time:   " << std::fixed << std::setw(11)
    << std::setprecision(6) << elapsed_time << " s  (" << throughput << " sps)" << std::endl;

	float faster = (throughput / throughput_original - 1) * 100.0;
	std::cout << "Speedup:   " << std::fixed << std::setw(11)
    << std::setprecision(2) << faster << "% faster" << std::endl << std::endl;

	delete test;
}

void printHelp() {
	std::cout << std::endl;
//			std::cout << "Usage: [<test buffer size>] [--gpu] [--cpu] [--accel] [--any]" << std::endl;
	std::cout << "Usage: [--ntaps=<# of filter taps>] [number of samples (default is 8192)]" << std::endl;
	std::cout << std::endl;
	std::cout << "You can create a filter by hand and see how many taps it would create from an interactive python command-line like this:" << std::endl;
	std::cout << std::endl;
	std::cout << "python" << std::endl;
	std::cout << "from gnuradio.filter import firdes" << std::endl;
	std::cout << "# parameters are gain, sample rate, cutoff freq, transition width for this low_pass filter." << std::endl;
	std::cout << "taps=firdes.low_pass(1, 10e6, 500e3, 0.2*500e3)" << std::endl;
	std::cout << "len(taps)" << std::endl;
	std::cout << std::endl << "For this example 241 taps were created." << std::endl;
	std::cout << std::endl;
}

int
main (int argc, char **argv)
{
	/*
  CppUnit::TextTestRunner runner;
  std::ofstream xmlfile(get_unittest_path("lfast.xml").c_str());
  CppUnit::XmlOutputter *xmlout = new CppUnit::XmlOutputter(&runner.result(), xmlfile);

  runner.addTest(qa_lfast::suite());
  runner.setOutputter(xmlout);

  bool was_successful = runner.run("", false);

  return was_successful ? 0 : 1;
  */

	// Add comma's to numbers
	std::locale comma_locale(std::locale(), new comma_numpunct());

	// tell cout to use our new locale.
	std::cout.imbue(comma_locale);

	if (argc > 1) {
		// 1 is the file name
		if (strcmp(argv[1],"--help")==0) {
			printHelp();
			exit(0);
		}

		for (int i=1;i<argc;i++) {
			std::string param = argv[i];

			if (param.find("--ntaps") != std::string::npos) {
				boost::replace_all(param,"--ntaps=","");
				ntaps=atoi(param.c_str());
			}
			else if (atoi(argv[i]) > 0) {
				int newVal=atoi(argv[i]);

				largeBlockSize=newVal;
				std::cout << "Running with user-defined test buffer size of " << largeBlockSize << std::endl;
			}
			else {
				std::cout << "ERROR: Unknown parameter." << std::endl;
				exit(1);

			}
		}
	}

	if (ntaps == 0) {
		std::cout << "ERROR: please specify --ntaps=<# of taps>." << std::endl << std::endl;
		printHelp();
		exit(1);
	}

	timeCostasLoop2();
	timeCostasLoop4();
	timeAGC();
	timeCC2Vector();
	timeLog10();
	timeFilter();

	// This just turned out to be slower even specifying other architectures in .volk/volk_profile
	// timeQuadDemod();

	return 0;

}
