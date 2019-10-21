#ifndef DEBUG_H
#define DEBUG_H
#include <algorithm>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <string>
//#include "mex.h"
#include <ctime>

using namespace std;

//#define LOG_FNAME "PERS_DEBUGGER_LOG.txt"
//#define ERR_FNAME "PERS_DEBUGGER_ERR.txt"


class DebuggerClass
{
public:
	static time_t startTime, endTime;
	static int num_error;
	static bool quiet;
	static string LOG_FNAME;
	static string ERR_FNAME;
public:
    static void init ( bool qt, string lfname, string efname );

    static void myMessage (const string , bool );
    
    static void finish();	

    static void myErrMessage (const string ,bool );

};

// OUTPUT_MSG print message (in a stream formation) to log file, together with the time stamp
#define  OUTPUT_MSG(MSG)  {stringstream * ss=new stringstream(stringstream::in | stringstream::out); *ss << MSG << endl << ends; DebuggerClass::myMessage(ss->str(), true); delete ss; }
// OUTPUT_MSG print message without time stamp
#define  OUTPUT_NOTIME_MSG(MSG)  {stringstream * ss=new stringstream(stringstream::in | stringstream::out); *ss << MSG << endl << ends; DebuggerClass::myMessage(ss->str(),false); delete ss; }

// MY_MEXPRINTF print string into LOG file
#define  MY_MEXPRINTF(...)  {{char buffer[100]; sprintf(buffer, __VA_ARGS__); stringstream * ss=new stringstream(stringstream::in | stringstream::out); *ss << buffer << ends; DebuggerClass::myMessage(ss->str(),false); delete ss; }}

#define expo(...) __VA_ARGS__
// MY_ASSERT( a, ... ) print error message into the ERROR file, together with the line and file where the error happens
#define WHERESTR  "[file %s, line %d]: "
#define WHEREARG  __FILE__, __LINE__
#define MY_MEXERRF(...)  {{char buffer[100]; sprintf(buffer, __VA_ARGS__); stringstream * ss=new stringstream(stringstream::in | stringstream::out); *ss << buffer << ends; DebuggerClass::myErrMessage(ss->str(),true); delete ss; }}
#define DEBUGPRINT(_fmt, ...)  expo(MY_MEXERRF(WHERESTR _fmt, WHEREARG, __VA_ARGS__))
#define MY_ASSERT_MORE( a, ... ) expo( if (!(a)){ DEBUGPRINT( __VA_ARGS__ )} )
#define MY_ASSERT( a ) expo( MY_ASSERT_MORE( a, "%s", "ERROR" ) )

#endif
