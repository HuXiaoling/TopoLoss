#include "Debugging.h"

using namespace std;

time_t DebuggerClass::startTime;
time_t DebuggerClass::endTime;
int DebuggerClass::num_error;
bool DebuggerClass::quiet;
string DebuggerClass::LOG_FNAME;
string DebuggerClass::ERR_FNAME;

void DebuggerClass::init ( bool qt, string lfname, string efname ){
	DebuggerClass::quiet = qt;
  	DebuggerClass::num_error = 0;
	DebuggerClass::LOG_FNAME = lfname;
	DebuggerClass::ERR_FNAME = efname;

    // initialize the two files, DebuggerClass::LOG and DebuggerClass::ERROR.	    
        fstream filestr;
        filestr.open (DebuggerClass::LOG_FNAME.c_str(), fstream::in | fstream::out | fstream::trunc);
        filestr << "Debuging Persistence Computation -- Log File" << endl;
        filestr.close();

	filestr.open (DebuggerClass::ERR_FNAME.c_str(), fstream::in | fstream::out | fstream::trunc);
        filestr << "Debuging Persistence Computation -- Error File" << endl;
        filestr.close();

	if( ! DebuggerClass::quiet ){
//	        mexPrintf("log file: %s\n",DebuggerClass::LOG_FNAME);
//	        mexPrintf("error file: %s\n",DebuggerClass::ERR_FNAME);
	        cout << "log file: " << DebuggerClass::LOG_FNAME << endl;
	        cout << "error file: " << DebuggerClass::ERR_FNAME << endl;
	}

	//start counting time
	time (& DebuggerClass::startTime);

    };

void DebuggerClass::myMessage (const string msg, bool showtime){
    //      mexWarnMsgTxt(msg.c_str());
    //      mexPrintf("%s\n",msg.c_str());
	    
 	    time_t now;
 	    time(&now);
 
 	    fstream filestr;
 	    filestr.open (DebuggerClass::LOG_FNAME.c_str(), fstream::in | fstream::out | fstream::ate);
 	    
 	    if (showtime){
 	      filestr << ctime(&now) << "-------" << msg.c_str();
 	    }else{
 	      filestr << msg.c_str();
 	    }
 	    filestr.close();
     };
 
void DebuggerClass::finish(){
	    // print timing
	    time(&DebuggerClass::endTime);
	    double ellapsed = difftime (DebuggerClass::endTime,DebuggerClass::startTime);       

	    OUTPUT_NOTIME_MSG( "All calculations computed in " <<  (int)ellapsed << " seconds. " << endl );
	 
   	    if( ! DebuggerClass::quiet )	    
	    	cout << "Persistence computed in " << (int) ellapsed << " seconds\n";
//	    	mexPrintf("Persistence computed in %d seconds\n", (int)ellapsed);

   	    if (DebuggerClass::num_error > 0){
	         cout << "Number of errors = " << DebuggerClass::num_error << endl;
		 cout << "If error happens, please contact the author.\n" ;
//	          mexPrintf("Number of errors = %d\n", DebuggerClass::num_error);
//		 mexPrintf("If error happens, please contact the author.\n");
 	    };
    

    };
	
void DebuggerClass::myErrMessage (const string msg,bool showtime){
    //      mexWarnMsgTxt(msg.c_str());
    //      mexPrintf("%s\n",msg.c_str());
    	    DebuggerClass::num_error = DebuggerClass::num_error + 1;

	    time_t now;
	    time(&now);

	    fstream filestr;
	    filestr.open (DebuggerClass::ERR_FNAME.c_str(), fstream::in | fstream::out | fstream::ate);
	    
	    if (showtime){
	      filestr << ctime(&now) << "-------" << msg.c_str();
	    }else{
	      filestr << msg.c_str();
	    }
	    filestr.close();
    };


