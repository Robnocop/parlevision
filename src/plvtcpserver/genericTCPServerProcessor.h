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

#include <opencv/cv.h>

//#include <conio.h>
//#include <iostream>

//#include "Server.h"

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
    Q_CLASSINFO("name", "UDP Server")
    Q_CLASSINFO("description", "UDP server, non QT-TCP IP server for use with 3rd party software not supporting QT")
    Q_PROPERTY( int port READ getPort WRITE setPort NOTIFY portChanged  )

    /** required standard method declaration for plv::PipelineProcessor */
    PLV_PIPELINE_PROCESSOR

	public slots:
		void setPort(int port, bool doEmit=false );

	signals:
		void portChanged(int port);
	
	public:
		genericTCPServerProcessor();
		virtual ~genericTCPServerProcessor();

		virtual bool init();
		virtual bool deinit() throw();
		virtual bool start();
		virtual bool stop();

		/** propery methods */
		int getPort() const;
		//int timedout; 


	private:
		void acceptConfigurationRequest();

		unsigned int m_port;
		//unsigned int m_movx;
		//unsigned int m_movy;
	
		bool m_waiting;
		bool m_convertCvMatToQImage;
		bool m_bool;
		int m_cvMatDataTypeId;
		sockaddr_in dest;
		sockaddr_in local;
		SOCKET socketname;
};

#endif // TCPSERVERPROCESSOR_H
