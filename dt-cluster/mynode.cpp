#include "mynode.h"
#include "packets.h"
#include <dtCore/deltawin.h>
#include <dtCore/transform.h> //Fix for D3D_2.4.0-RC2
#include <osgViewer/Viewer> //Fix for D3D_2.4.0-RC2
//#include <conio.h>

using namespace dtCore;
using namespace dtABC;
using namespace dtNet;

MyNode::MyNode( const std::string &fileName,
				const std::string &hostIP, 
				const std::string &slaveCam,
                const std::string &configFilename ): Application( configFilename ),
				mhostIP(hostIP),
				mSlaveCam(slaveCam),
				mFileName(fileName)
{
	std::string logFilename;

	if (mhostIP.empty())//if no hostIP was supplied, create a host, otherwise create a client or slave
	{
		mIamHost = true;
		logFilename = std::string("server.log");
		mIsdTrackerHandle = 0;
	}
	else
	{
		mIamHost = false;
		logFilename = std::string("client.log");
	}
	
	mNet = new MyNet( this, GetCamera() );
	
	//initialize the game name, version number, and a networking log file
	mNet->InitializeGame("Delta3D Cluster", 1, logFilename);

	//register our custom packet with GNE. Must come *after* NetMgr::InitializeGame()
	GNE::PacketParser::defaultRegisterPacket<PositionPacket>();
	GNE::PacketParser::defaultRegisterPacket<PlayerQuitPacket>();

	if ( mIamHost )
	{
		mNet->SetupServer( 4444 );
		GetWindow()->SetWindowTitle("Host" );
	}
	else
	{
		mNet->SetupClient( hostIP, 4444 );
		GetWindow()->SetWindowTitle("Slave to: " + hostIP);
	 
	}
}
   
void MyNode::Config()
{  
	Application::Config(); 

	/**
	*  Set up Stereo Rendering
	**/
	osg::DisplaySettings *display = osg::DisplaySettings::instance();
	display->setStereo(true);
	display->setStereoMode(osg::DisplaySettings::HORIZONTAL_SPLIT);
	display->setSplitStereoHorizontalEyeMapping(osg::DisplaySettings::LEFT_EYE_RIGHT_VIEWPORT);

	//See issue 1, comment 5
	//Units are in meters
	display->setEyeSeparation(0.06);
	display->setScreenHeight(4.0);
	display->setScreenWidth(4.0);
	display->setScreenDistance(2.0);
	
	//display->setSplitStereoVerticalSeparation(400);


	/**
	*  If a joystick is present, the maximumWalkSpeed should be negative
	*  If no joystick is present, it should be positive
	*  Default: no joystick is present.
	**/
	float invertJoystick = 1.0f;
	
	mScene = new Object("Scene");// setup scene here
	mScene->LoadFile( mFileName );
	AddDrawable( mScene.get() );     
	
	
	double zNear = 0.1;// works where FOVx = 90 and left=-.1, right=.1. 
	double zFar =1000.0;
	GetCamera()->SetFrustum(-0.1, 0.1, -0.1, 0.05, zNear, zFar); // off-axis	

	if ( mIamHost )
	{	
		if ( !GetWindow()->GetFullScreenMode() )
		{
			int width = GetWindow()->GetPosition().mWidth; //    426;
			int height = GetWindow()->GetPosition().mHeight; //  341;
			//Commented out. Doesn't work in 2.4.0_RC2
			//GetWindow()->GetOsgViewerGraphicsWindow()->setWindowDecoration(false);
			GetWindow()->SetPosition(width, height, width, height);

		} else
		{
			GetWindow()->SetFullScreenMode(true);
			//2800x1050
			GetWindow()->ChangeScreenResolution(2800, 1050, 24, 59);

		}
		mMotion = new WalkMotionModel(GetKeyboard(), GetMouse());

		/**
		*  Sets up the joystick controllers if there is one present.
		*  In the CUBE-environment, instance 0 is the correct joystick.
		**/
		dtInputPLIB::Joystick::CreateInstances();

		//Do this if there are one or more joysticks connected.
		if(dtInputPLIB::Joystick::GetInstanceCount() > 0)
		{
			dtInputPLIB::Joystick* mJoystick = dtInputPLIB::Joystick::GetInstance(0);
			mMotion->SetTurnLeftRightAxis(mJoystick->GetAxis(0));
			mMotion->SetWalkForwardBackwardAxis(mJoystick->GetAxis(1));
			invertJoystick = -1.0f;
		}

		mMotion->SetScene( GetScene() );
		mMotion->SetTarget( GetCamera() );
		mMotion->SetHeightAboveTerrain( eyeheight );   
		mMotion->SetMaximumWalkSpeed(invertJoystick * 3.0f); // we need a negative value for joystick
		mMotion->SetMaximumTurnSpeed(70.0f);

		mIsdTrackerHandle = ISD_OpenTracker(NULL, 0, false, false);
		if(mIsdTrackerHandle > 0)
		{
			printf( "\n xPos yPos zPos\n" );
		}
		else
		{
			printf("Tracker not found");
		}

		mHeadTrackPosition[0] = 0.0f;
		mHeadTrackPosition[1] = 0.0f;
		mHeadTrackPosition[2] = 0.0f;
	}	
	else
		CreateSlaveCam();
}

void MyNode::CreateSlaveCam()
{
	// add a slave camera to the default camera, with correct frustum
	mCam = new Camera("Slave");
	int hq =1; // horizontal quadrant position
	int vq =1; // vertical quadrant position
	mCam->SetWindow( GetWindow() );
	mCam->SetFrustum(-0.1, 0.1, -0.1, 0.05, 0.1, 1000.0 ); // frustum off-axis

	if (mSlaveCam == "left")
	{
		GetView()->GetOsgViewerView()->addSlave( mCam->GetOSGCamera(), osg::Matrixd(), osg::Matrixd::rotate(osg::inDegrees(-90.0f), 0.0,1.0,0.0 ) );
		hq =0;
		vq =1;
		goto windef;
	}
	if (mSlaveCam == "right")
	{
		GetView()->GetOsgViewerView()->addSlave( mCam->GetOSGCamera(), osg::Matrixd(), osg::Matrixd::rotate(osg::inDegrees(90.0f), 0.0,1.0,0.0 ) );
		hq =2;
		vq =1;
		goto windef;
	}
	if (mSlaveCam == "down")
	{
		GetView()->GetOsgViewerView()->addSlave( mCam->GetOSGCamera(), osg::Matrixd::translate(0.0, -0.66666666, 0.0),  osg::Matrixd::rotate(osg::inDegrees(90.0f), 1.0,0.0,0.0 ) );
		hq =1;
		vq =2;
		goto windef;
	}
	if (mSlaveCam == "up")
	{
		GetView()->GetOsgViewerView()->addSlave( mCam->GetOSGCamera(), osg::Matrixd::translate(0.0, -0.66666666, 0.0),  osg::Matrixd::rotate(osg::inDegrees(-90.0f), 1.0,0.0,0.0 ) );
		hq =1;
		vq =0;
		goto windef;
	}
	if (mSlaveCam == "back")
	{
		GetView()->GetOsgViewerView()->addSlave( mCam->GetOSGCamera(), osg::Matrixd(), osg::Matrixd::rotate(osg::inDegrees(180.0f), 0.0,1.0,0.0 ) );
		hq =2;
		vq =2;
		goto windef;
	}

	//default: front view
	GetView()->GetOsgViewerView()->addSlave( mCam->GetOSGCamera(), osg::Matrixd(), osg::Matrixd() );
	
windef:
	if ( !GetWindow()->GetFullScreenMode()  )
		{
			int width = GetWindow()->GetPosition().mWidth; //
			int height = GetWindow()->GetPosition().mHeight; //
			GetWindow()->GetOsgViewerGraphicsWindow()->setWindowDecoration(false);
			GetWindow()->SetPosition(width*hq, height*vq, width, height);
		}
}


void MyNode::PreFrame( const double deltaFrameTime )
{
	dtInputPLIB::Joystick::GetInstance(0)->Poll();
	//seems as if the below lines was the one that caused BSoD (bug id #4).
	//dtInputPLIB::Joystick::PollInstances();
	mNet->PreFrame( deltaFrameTime );
}

void MyNode::Frame( const double deltaFrameTime )
{
	Application::Frame(deltaFrameTime);

	if (mIamHost)// if host: send position packet.
	{
		updateHeadTracking();
		SendPosition(); 
		//mCam->SetFrustum(-0.1-mHeadTrackPosition[0], 0.1-mHeadTrackPosition[0], -0.1, 0.05, 0.1, 1000.0 ); // frustum off-axis
	}
}

void MyNode::SendPosition()
{
   //get our new position
   Transform xform;
   GetCamera()->GetTransform( xform );
   osg::Vec3 xyz;
   osg::Vec3 hpr;
   xform.GetTranslation(xyz);
   xform.GetRotation(hpr);

   PositionPacket packet( xyz, hpr, GetUniqueId().ToString() );
   mNet->SendPacket( "all", packet ); //send our position out to all connections
}

void MyNode::Quit()
{
	if (mIamHost)// if am Host:
	{
		//notify everyone else we are quitting
		PlayerQuitPacket packet( GetUniqueId().ToString() );
		mNet->SendPacket( "all", packet );

		//Close the head tracking
		ISD_CloseTracker(mIsdTrackerHandle);
		
	}
	//shutdown the networking
	//mNet->Shutdown();
	
	Application::Quit();
}

void MyNode::updateHeadTracking()
{
	if(mIsdTrackerHandle > 0)
	{
		ISD_GetData(mIsdTrackerHandle, &mIsdTrackerData);
		printf( "%7.2f %7.2f %7.2f ", mIsdTrackerData.Station[0].Position[0], mIsdTrackerData.Station[0].Position[1], mIsdTrackerData.Station[0].Position[2] );
		/*data.Station[0].Orientation[0],
		data.Station[0].Orientation[1],
		data.Station[0].Orientation[2] );*/
		mHeadTrackPosition[0] = mIsdTrackerData.Station[0].Position[1] *0.05f;
		mHeadTrackPosition[1] = mIsdTrackerData.Station[0].Position[0] *0.05f;
		mHeadTrackPosition[2] = mIsdTrackerData.Station[0].Position[2] *0.05f;

		ISD_GetCommInfo(mIsdTrackerHandle, &mIsdTrackerInfo);

		printf( "%5.2fKbps %d Records/s \r",
			mIsdTrackerInfo.KBitsPerSec, mIsdTrackerInfo.RecordsPerSec );
	}
}


//bool MyNode::KeyPressed(const dtCore::Keyboard* keyboard, int key)
//{
//   bool verdict(false);
//   switch( key )
//   {
//   case osgGA::GUIEventAdapter::KEY_Escape:
//      {
//         Quit();
//         verdict = true;
//      } break;
//
//   case 'P':
//      {
//         //send a "ping" packet for latency info
//         GNE::PingPacket ping;
//         mNet->SendPacket("all", ping);
//         verdict = true;
//      } break;
//
//   default:
//      {
//         verdict = false;
//      } break;
//   }
//
//   return verdict;
//}