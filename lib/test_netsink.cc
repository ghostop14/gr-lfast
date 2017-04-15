/*
 * test_netsink.cc
 *
 *  Created on: Apr 7, 2017
 *      Author: ghostop14
 */


#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <cppunit/TextTestRunner.h>
#include <cppunit/XmlOutputter.h>

#include <gnuradio/unittests.h>
#include "qa_lfast.h"
#include <iostream>
#include <fstream>

#include <chrono>
#include "dt_datatypes.h"
#include "net_sink_impl.h"

using namespace gr::lfast;

int
main (int argc, char **argv)
{
	std::string host = "127.0.0.1";
	int port=2000;
	std::cout << "Testing netsink.  Sending test data to " << host << ":" << port << std::endl;

	net_sink_impl *test;
//	test = new net_sink_impl(int vecLen,int datatype,int dataSize,const std::string &host, int port,bool isTCP, bool autoreconnect);
	test = new net_sink_impl(1,DTYPE_CHAR,sizeof(char),host, port,true, false);

	int i;
	int iterations=100;
	std::chrono::time_point<std::chrono::steady_clock> start, end;
	std::chrono::duration<double> elapsed_seconds;
	float elapsed_time,throughput_original,throughput;
	std::vector<const void *> inputPointers;
	std::vector<void *> outputPointers;

	std::string testString="";
	int counter=0;
	int localblocksize=8192;

	for (int i=0;i<localblocksize;i++) {
		testString += std::to_string(counter);
		counter++;

		if (counter > 9)
			counter = 0;
	}

	inputPointers.push_back(testString.c_str());

	start = std::chrono::steady_clock::now();
	for (i=0;i<iterations;i++) {
		test->work_test(localblocksize,inputPointers,outputPointers);
	}

	end = std::chrono::steady_clock::now();

	elapsed_seconds = end-start;

	elapsed_time = elapsed_seconds.count() / iterations;
	throughput_original = localblocksize / elapsed_time;

	while (test->isThreadRunning())
		sleep(1);

	std::cout << "Code Run Time:   " << std::fixed << std::setw(11)
    << std::setprecision(6) << elapsed_time << " s  (" << throughput_original << " sps)" << std::endl;

	delete test;

	return 0;

}

