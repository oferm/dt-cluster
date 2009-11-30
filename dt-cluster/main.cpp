/* -*-c++-*-
* MyNode - main (.h & .cpp) - Using 'The MIT License'
* Copyright (C) 2006-2008, MOVES Institute
*/

// main.cpp : defines the entry point for the console application.

#include "mynode.h"
#include <gnelib.h>
#include <dtCore/globals.h>
#include "isense.h"

using namespace dtCore;
using namespace dtUtil;

// Supplying a host name as the first argument on the command line will create
// a client and try to connect to that server.
// Only one parameter on the command line will create a server.
int main(int argc, char *argv[] )
{
	//set data search path to parent directory and delta3d/data
	SetDataFilePathList( GetDeltaRootPath() + "/examples/data" + ";");

	//Log::GetInstance().LogMessage(Log::LOG_ALWAYS, "", "Usage: testNetwork.exe [filename] [hostIP] [slaveCam]");

	std::string fileName;
	std::string hostIP;
	std::string slaveCam;

	if (argc>1)
	{
	  fileName = std::string(argv[1]);
	}else 
	   return 0; // filename missing

	if (argc>2)
	{
		hostIP = std::string(argv[2]); // this is a client or slave. Set host's IP-adress
		if (argc>3)
		{
			slaveCam = std::string(argv[3]); // this is a slave. "front", "left", "right", "up", "down"  or "back"
		}
	}
	
	dtCore::RefPtr<MyNode> app = new MyNode( fileName, hostIP, slaveCam, "dt_cluster_config.xml" );

	app->Config(); //configuring the application
	app->Run(); // running the simulation loop

	return 0;
}

