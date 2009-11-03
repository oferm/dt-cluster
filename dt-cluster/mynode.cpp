#include "mynode.h"
#include "packets.h"
#include <dtCore/deltawin.h>
#include <dtCore/transform.h> //Fix for D3D_2.4.0-RC2
#include <osgViewer/Viewer> //Fix for D3D_2.4.0-RC2

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

		}

		/* 
		** Sets up the controlling scheme
		*/
		mMotion = new WalkMotionModel(GetKeyboard(), GetMouse());
		mMotion->SetScene( GetScene() );
		mMotion->SetTarget( GetCamera() );
		mMotion->SetHeightAboveTerrain( eyeheight );   
		mMotion->SetMaximumWalkSpeed(3.0f); // we need a negative value for joystick(!!)
		mMotion->SetMaximumTurnSpeed(70.0f);

		/*
		** Sets up the joystick controller
		*/
		mInputMapper = new InputMapper;

		dtInputPLIB::Joystick::CreateInstances();

		for (int i = 0; i < dtInputPLIB::Joystick::GetInstanceCount(); i++)
		{
		 mInputMapper->AddDevice(dtInputPLIB::Joystick::GetInstance(i));
		}
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
   mNet->PreFrame( deltaFrameTime );
}

void MyNode::Frame( const double deltaFrameTime )
{
	Application::Frame(deltaFrameTime);

	if (mIamHost)// if host: send position packet.
		SendPosition(); 
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
		
	}
	//shutdown the networking
	//mNet->Shutdown();
	
	Application::Quit();
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