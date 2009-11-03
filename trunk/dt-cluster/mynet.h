/* -*-c++-*-
* MyNode - MyNet (.h & .cpp) - Using 'The MIT License'
* Copyright (C) 2006-2008, MOVES Institute
*
*/
#ifndef MyNet_INCLUDE
#define MyNet_INCLUDE

#include <dtNet/netmgr.h>
#include <dtCore/refptr.h>
#include <dtCore/object.h>
#include <dtCore/scene.h>
#include <dtCore/camera.h>

class MyNode; // forward declaration

const float eyeheight = 1.7f;

/** Deriving from NetMgr will allow use to overwrite some virtual methods.
  * We'll use these methods for controlling our network connections.
  */
class MyNet : public dtNet::NetMgr
{
public:
   MyNet( MyNode* mynode, dtCore::Camera* camera );
   virtual ~MyNet() {}

   virtual void OnReceive( GNE::Connection &conn );
   virtual void OnExit( GNE::Connection &conn );
   virtual void OnDisconnect( GNE::Connection &conn );

   void PreFrame( const double deltaFrameTime );

private:
	//dtCore::RefPtr< dtCore::Scene >						mScene;
	MyNode*												mNode;
	dtCore::RefPtr< dtCore::Camera >					mCamera;
	//bool												mIsSlave;
	//std::queue< dtCore::RefPtr<dtCore::Object> >		mObjectsToAdd;
	//std::queue< std::string >							mIDsToRemove;

	///a map of player ID strings and their corresponding Object
	//typedef std::map<std::string, dtCore::RefPtr<dtCore::Object> > StringObjectMap;
	//StringObjectMap mOtherPlayerMap;

	//void MakePlayer(const std::string& ownerID);
	GNE::Mutex mMutex;
};

#endif // _DEBUG
