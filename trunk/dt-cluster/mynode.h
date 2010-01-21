/* -*-c++-*-
* MyNode - MyNode (.h & .cpp) - Using 'The MIT License'
* Copyright (C) 2006-2008, MOVES Institute
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*/
#ifndef DELTA_MyNode
#define DELTA_MyNode

#include <dtABC/application.h>
#include <dtCore/walkmotionmodel.h>
#include <dtCore/refptr.h>
#include <dtNet/dtnet.h>
#include <dtCore/inputmapper.h>         //For joystick functionality
#include <dtInputPLIB/joystick.h>       //For joystick functionality 

#include "Mynet.h"
#include "isense.h"

class MyNode : public dtABC::Application
{
   public:
      MyNode(	const std::string &fileName,
				const std::string &hostIP, 
				const std::string &slaveCam,
				const std::string &configFilename );
   protected:
      virtual ~MyNode() {}
   public:
      virtual void Config();

      bool KeyPressed(const dtCore::Keyboard* keyboard, int key);

      virtual void PreFrame( const double deltaFrameTime );
      virtual void Frame( const double deltaFrameTime );
      virtual void Quit();

   private:
         
		dtCore::RefPtr<MyNet>					mNet; // Reference the NetMgr derived class
		std::string								mFileName; // full path to scene's file
		std::string								mhostIP; // if client or slave: The host's IP f.ex. "168.212.0.1"
		std::string								mSlaveCam; // if slave: "front", "left", "right", "up", "down"  or "back" 
		dtCore::RefPtr<dtCore::Object>			mScene; // Ground
		dtCore::RefPtr<dtCore::WalkMotionModel>	mMotion; // Motion model
		dtCore::RefPtr<dtCore::Camera>			mCam;
		bool									mIamHost;
		osg::Vec3								mHeadTrackPosition;
		ISD_TRACKER_HANDLE						mIsdTrackerHandle;
		ISD_TRACKER_INFO_TYPE					mIsdTrackerInfo;
		ISD_TRACKER_DATA_TYPE					mIsdTrackerData;

		void SendPosition();
		void CreateSlaveCam();
		void updateHeadTracking();
   
};

#endif // DELTA_MyNode
