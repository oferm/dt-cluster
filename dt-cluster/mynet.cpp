#include "mynet.h"
#include "packets.h"
#include <osg/io_utils>
#include <dtUtil/stringutils.h>
#include "mynode.h"
#include <dtCore/transform.h> //Fix for D3D_2.4.0-RC2

using namespace dtUtil;
using namespace dtCore;

MyNet::MyNet( MyNode* mynode, dtCore::Camera* camera  )
   : mNode(mynode), mCamera(camera)
{
}

///One or more GNE::Packets was received, let's do something with them
void MyNet::OnReceive( GNE::Connection &conn )
{
	mMutex.acquire();

   GNE::Packet* next = conn.stream().getNextPacket();

   while( next != 0 )
   {
      int type = next->getType();

	  if( type == PositionPacket::ID )
      {
         if ( !GetIsServer() )
		 {
			 //aha, this is one of our custom packets.  Decompose it and update our slave.
			 PositionPacket *pos = static_cast<PositionPacket*>(next);
			 osg::Vec3 newXYZ = pos->mXYZ;
			 osg::Vec3 newHPR = pos->mHPR;
			 std::string ownerID = pos->mOwnerID;

			 Transform xform;
			 xform.SetTranslation(newXYZ);
			 xform.SetRotation(newHPR);
			  
			 mCamera->SetTransform(xform, Transformable::REL_CS);
			 
		 }else		 
			LOG_INFO("Host sending position to itself\n");
      }
      else if( type == PlayerQuitPacket::ID )
      {
         PlayerQuitPacket* playerQuitPacket = static_cast<PlayerQuitPacket*>(next);
         //mIDsToRemove.push( playerQuitPacket->mPlayerID );
		 if ( !GetIsServer() )
		 {
			 LOG_INFO("Host sending Quit-packet\n");
			 //Shutdown();
			 mNode->Quit();
		 }else
			 LOG_ERROR("Host sending Quit-packet to itself\n");
      }

      delete next;
      next = conn.stream().getNextPacket();
   }

   mMutex.release();
}


void MyNet::OnExit( GNE::Connection &conn )
{
	Log::GetInstance().LogMessage(Log::LOG_ALWAYS, "", "MyNet::OnExit");
   
	//do the default NetMgr behavior too
   NetMgr::OnExit(conn);
}

void MyNet::OnDisconnect( GNE::Connection &conn )
{
	Log::GetInstance().LogMessage(Log::LOG_ALWAYS, "", "MyNet::OnDisconnect");
	RemoveConnection(&conn);
}

void MyNet::PreFrame( const double deltaFrameTime )
{

}
