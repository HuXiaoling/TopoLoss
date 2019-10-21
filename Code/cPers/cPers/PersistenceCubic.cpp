// Persistent Homology code by Hubert Wagner based on an implementation by Chao Chen
#include <cmath>
#include <vector>
#include <map>
#include <set>
#include <list>
#include <cassert>
#include <algorithm>
#include <iostream>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <deque>
#include <cstring>
#include <ctime>
#include <blitz/array.h>
#include <blitz/tinyvec-et.h>

using namespace std;

const int BIG_INT = 0x7FFFFFFF;	//be careful, could be too small compared to # of cubes
typedef int CellNrType; // it could be long for really large inputs...

#include "PersistenceIO.h"
#include "Debugging.h"
#include "GeneralFiltration.h"

#include "InputRunner.h"

#include "PersistentPair.h"
#include "DataReaders.h"

#include "PersistenceCalculator.h"
#include "PersistenceCalcRunner.h"


int main(int argc, const char* argv[]){

	time_t startTime1, startTime2, endTime;		

	if (argc < 2)
	{
		std::cout << "usage: " << argv[0] << " input_file.ext" << std::endl;		
		return 1;
	}		

	string input_file = argv[1];
	double pers_thd = 0.0;	

	string lfile = "log.txt";	
	string efile = "error.txt";	
	DebuggerClass::init( false, lfile, efile );
	
	InputFileInfo input_file_info(input_file);
	
	int max_pers_pts = BIG_INT; // the maximal number of persistence pairs recorded

	fstream filestr;
	filestr.open (lfile.c_str(), fstream::out | fstream::trunc);
	//	filestr.open (LOG_FILE, fstream::out | fstream::ate);
	filestr << "################################################" << endl;
	filestr << "start computing persistence" << endl;
	filestr << "Input file = \'" << input_file << "\'" << endl;
	// filestr << "Output file = \'" << output_file << "\'" << endl;	
	filestr << "Maximal number of persistence points recorded = " << max_pers_pts << endl;
	filestr.close();	

	time(&startTime1);		

	switch(input_file_info.dimension)
	{
	case 1:
		InputRunner<1>::run(input_file_info, pers_thd);
		break;
	case 2:
		InputRunner<2>::run(input_file_info, pers_thd);
		break;
	case 3:
		InputRunner<3>::run(input_file_info, pers_thd);
		break;
	case 4:
		InputRunner<4>::run(input_file_info, pers_thd);
		break;
	case 5:
		InputRunner<5>::run(input_file_info, pers_thd);
		break;
	case 6:
		InputRunner<6>::run(input_file_info, pers_thd);
		break;	
	case 7:
		InputRunner<7>::run(input_file_info, pers_thd);
		break;	
	case 8:
		InputRunner<8>::run(input_file_info, pers_thd);
		break;	
	}

	time(&endTime);
	double ellapsed1 = difftime (endTime,startTime1);	

	cout << "All calculations computed in " << setprecision(2) << ellapsed1/60.0 << endl;
	DebuggerClass::finish();
	
	return 0;
}
