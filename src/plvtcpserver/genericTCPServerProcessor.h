/**
  * Copyright (C)2011 by Richard Loos
  * All rights reserved.
  *
  * This file is part of the plvtcpserver plugin of ParleVision.
  *
  * ParleVision is free software: you can redistribute it and/or modify
  * it under the terms of the GNU General Public License as published by
  * the Free Software Foundation, either version 3 of the License, or
  * (at your option) any later version.
  *
  * ParleVision is distributed in the hope that it will be useful,
  * but WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  * GNU General Public License for more details.
  *
  * A copy of the GNU General Public License can be found in the root
  * of this software package directory in the file LICENSE.LGPL.
  * If not, see <http://www.gnu.org/licenses/>.
  */

#ifndef GENERICTCPSERVERPROCESSOR_H
#define GENERICTCPSERVERPROCESSOR_H

#include <QObject>
#include <plvcore/PipelineProcessor.h>
#include <plvcore/RefPtr.h>
#include <plvcore/Pin.h>

#define _WINSOCKAPI_    // stops windows.h including winsock.h
#include <winsock2.h>
#include <opencv/cv.h>


//#ifndef WIN32_LEAN_AND_MEAN
//#define WIN32_LEAN_AND_MEAN
//#endif


//#include <windows.h>
//#include <ws2tcpip.h>
//#include <windows.h>

#include <conio.h>
#include <iostream>

#include "Server.h"

#pragma comment(lib, "Ws2_32.lib")

namespace plv
{
    class CvMatDataInputPin;
    class CvMatDataOutputPin;
    class IInputPin;
    class IOutputPin;
    class PinConnection;
}

class genericTCPServerProcessor : public plv::PipelineProcessor
{
    Q_OBJECT
    Q_DISABLE_COPY( genericTCPServerProcessor )

    Q_CLASSINFO("author", "Robby van Delden") //based on Qt TCP server of richard loos
    Q_CLASSINFO("name", "TCP Server v2")
    Q_CLASSINFO("description", "TCP server v2, non QT-TCP IP server for use with 3rd party software not supporting QT")
	//apparently somewhere i used set movx with a small x as well:(
    Q_PROPERTY( int port READ getPort WRITE setPort NOTIFY portChanged  )
	Q_PROPERTY( int movx READ getMovX WRITE setMovx NOTIFY movXChanged  )
	Q_PROPERTY( int movy READ getMovY WRITE setMovx NOTIFY movYChanged  )
    Q_PROPERTY( bool convertCvMatDataToQImage READ getConvertCvMatDataToQImage WRITE setConvertCvMatDataToQImage NOTIFY convertCvMatDataToQImageChanged )
   // Q_PROPERTY( bool lossless READ getLossless WRITE setLossless NOTIFY losslessChanged )
   // Q_PROPERTY( int maxFramesInQueue  READ getMaxFramesInQueue  WRITE setMaxFramesInQueue NOTIFY maxFramesInQueueChanged )
   // Q_PROPERTY( int maxFramesInFlight READ getMaxFramesInFlight WRITE setMaxFramesInFlight NOTIFY maxFramesInFlightChanged )

    /** required standard method declaration for plv::PipelineProcessor */
    PLV_PIPELINE_PROCESSOR
	int getMovX() const;
	int getMovY() const;

	public slots:
		void setPort(int port, bool doEmit=false );
		void setMovx(int movx);
		void setMovy(int movy);
		void setConvertCvMatDataToQImage(bool doConvert, bool doEmit=false);
		void stalled(ServerConnection* connection);
		void unstalled(ServerConnection* connection);
		//void setMaxFramesInQueue(int max);
		//void setMaxFramesInFlight(int max);
	   // void setLossless(bool lossless);
		void serverError(PlvErrorType type, const QString& msg);

	signals:
		void portChanged(int port);
		void movXChanged(int movx);
		void movYChanged(int movy);
		void convertCvMatDataToQImageChanged(bool b);
	   // void maxFramesInQueueChanged(int max);
	   // void maxFramesInFlightChanged(int max);
	   // void losslessChanged(bool lossless);

	
	public:
		genericTCPServerProcessor();
		virtual ~genericTCPServerProcessor();

		virtual bool init();
		virtual bool deinit() throw();
		virtual bool start();
		virtual bool stop();

		/** propery methods */
		int getPort() const;
	
		int timedout; 

		bool getConvertCvMatDataToQImage() const;
	   // int getMaxFramesInFlight() const;
	   // int getMaxFramesInQueue() const;
		bool getLossless() const;

		virtual bool isReadyForProcessing() const;

	   // virtual void inputConnectionSet(plv::IInputPin* pin, plv::PinConnection* connection);
		// virtual void inputConnectionRemoved(plv::IInputPin* pin, plv::PinConnection* connection);

	//    virtual void outputConnectionAdded(plv::IOutputPin* pin, plv::PinConnection* connection);
	 //   virtual void outputConnectionRemoved(plv::IOutputPin* pin, plv::PinConnection* connection);


	private:
		void acceptConfigurationRequest();

		unsigned int m_port;
		unsigned int m_movx;
		unsigned int m_movy;
		SOCKET m_server;//Server* m_server;
		SOCKET m_client;
		//WSADATA is a struct that is filled up by the call 
		//to WSAStartup
		WSADATA wsaData;

		bool m_waiting;
		struct sockaddr_in m_local;
		bool m_convertCvMatToQImage;
		int m_cvMatDataTypeId;
};

#endif // TCPSERVERPROCESSOR_H
