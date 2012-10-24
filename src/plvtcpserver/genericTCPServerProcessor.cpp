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

#include "genericTCPServerProcessor.h"
#include "Proto.h"

#include <plvcore/CvMatData.h>
#include <plvcore/CvMatDataPin.h>
#include <plvgui/ImageConverter.h>
#include <winsock2.h> 
//requires ws2_32.lib allthough i can find the header file the lib linking to the dll needs to be added in properties linker/ or the tcpserver pri file danger is that it can be msvc20210 depended
//needed for amongst other htons functions convert an IP port number in host byte order to the IP port number in network byte order
#include <conio.h>
#include <iostream>
//#include <QNetworkInterface>
#include <QImageWriter>


genericTCPServerProcessor::genericTCPServerProcessor() :
    m_port(20248),
	m_movx(0),
	m_movy(0),
    m_waiting(false),
    m_convertCvMatToQImage(false)

{
    plv::createDynamicInputPin( "generic pin", this, plv::IInputPin::CONNECTION_OPTIONAL );
    m_cvMatDataTypeId = QMetaType::type("plv::CvMatData");
    
	//m_server = new Server(this);
}

genericTCPServerProcessor::~genericTCPServerProcessor()
{
}

bool genericTCPServerProcessor::init()
{
	//m_server=socket(AF_INET,SOCK_STREAM,0);
    return true;
}

bool genericTCPServerProcessor::deinit() throw ()
{
   /* m_server->disconnectAll();
    m_server->close();
    disconnect(m_server, SIGNAL(onError(PlvErrorType, const QString&)),
               this, SLOT(serverError(PlvErrorType, const QString&)));*/
    //TODO
	//closesocket(m_server);
	 //closesocket() closes the socket and releases the socket descriptor
	closesocket(m_client);
    closesocket(m_server);

	//closesocket(m_client);
    //originally this function probably had some use
    //currently this is just for backward compatibility
    //but it is safer to call it as I still believe some
    //implementations use this to terminate use of WS2_32.DLL 
    WSACleanup();

	return true;
}

bool genericTCPServerProcessor::start()
{
	timedout = 0;
	//this is init stuff btw
	m_local.sin_family=AF_INET; //Address family
    m_local.sin_addr.s_addr=INADDR_ANY; //Wild card IP address
   // m_local.sin_port=htons((u_short)20248); //port to use
	m_local.sin_port=htons((u_short)getPort()); //port to use	
	
	//WSADATA is a struct that is filled up by the call 
    //to WSAStartup
    //WSADATA wsaData;

	//WSAStartup initializes the program for calling WinSock.
    //The first parameter specifies the highest version of the 
    //WinSock specification, the program is allowed to use.
    int wsaret=WSAStartup(0x101,&wsaData);

    //WSAStartup returns zero on success.
    //If it fails we exit.
    if(wsaret!=0)
    {
        qDebug() << "wsaret failed";
		return 0;
    }

	//the socket function creates our SOCKET
	m_server=socket(AF_INET,SOCK_STREAM,0);

	//If the socket() function fails we exit
	if(m_server==INVALID_SOCKET)
	{
		qDebug() << "invalid socket";
		return 0;
	}
	
    //bind links the socket we just created with the sockaddr_in 
    //structure. Basically it connects the socket with 
    //the local address and a specified port.
    //If it returns non-zero quit, as this indicates error
    if(bind(m_server,(sockaddr*)&m_local,sizeof(m_local))!=0)
    {
		qDebug() << "size localhost 0";
        return 0;
    }


	return true;
	
}

bool genericTCPServerProcessor::stop()
{
	//cant be called somehow qDebug() << "STOP!!!";

	WSAEVENT loopthet = WSACreateEvent();
	if (loopthet == WSA_INVALID_EVENT)
	{
		qDebug() << "somethingwent wrong";
	}else
	{
		qDebug() << "not the WSA event went wrong";
	}


	//closesocket() closes the socket and releases the socket descriptor
    WSACloseEvent(WSAAccept);
	closesocket(m_client);
	closesocket(m_server);
	//

    //originally this function probably had some use
    //currently this is just for backward compatibility
    //but it is safer to call it as I still believe some
    //implementations use this to terminate use of WS2_32.DLL 
    WSACleanup();
    return true;
}


bool genericTCPServerProcessor::process()
{
	//temp to be dependent of fps of incoming frames
	//qDebug() << "enter process";
	QVariantList frameData;
	

    PipelineElement::InputPinMap::iterator itr = m_inputPins.begin();
    for( ; itr != m_inputPins.end(); ++itr )
    {
        plv::IInputPin* pin = itr->second.getPtr();
        if( pin->isConnected() )
        {

            if( pin->getTypeId() == m_cvMatDataTypeId &&
                m_convertCvMatToQImage )
            {
                QVariant v;
                pin->getVariant(v);
                plv::CvMatData data = v.value<plv::CvMatData>();
                QImage img = plvgui::ImageConverter::cvMatToQImageNoThrow(data);
                QVariant v2;
                v2.setValue(img);
                frameData.append(v2);
            }
            else
            {
                QVariant v;
                pin->getVariant(v);
                frameData.append(v);
            }
        }
    }
    quint32 frameNumber = (quint32)getProcessingSerial();

	//Here our server part starts:
	
	//from http://www.programmersheaven.com/mb/CandCPP/245610/245610/timeout-on-listenaccept-functions----winsock/
	fd_set readfds;
	FD_ZERO(&readfds);
	FD_SET(m_server, &readfds);
	
	// Timeout parameter
	timeval tv = { 0 };
	//tv.tv_sec = 2;
	//something < last fps without taking this process into account.
	tv.tv_usec = 1000/30;
	int ret = select(0, &readfds, NULL, NULL, &tv);


	//from main from http://www.codeproject.com/Articles/1891/Beginning-Winsock-Programming-Simple-TCP-server
	int nRetCode = 0;	
	
	//listen instructs the socket to listen for incoming 
    //connections from clients. The second arg is the backlog
    if(listen(m_server,10)!=0)
    {
		qDebug() << "listens from server returned 0";
        return 0;
    }

    //we will need variables to hold the client socket.
    //thus we declare them here.
    //SOCKET m_client; //declared globally
    sockaddr_in from;
    int fromlen=sizeof(from);

	//TODO change
	
   // while(_getch()!=27)//we are looping endlessly with true
   // {
    
	char temp[512];
	//no clue
	char tekst[20];
	//accept() will accept an incoming client connection
	//seems as if this accept function waits to get a connection.
	//this is unacceptable
   // 
	//m_client=accept(m_server, (struct sockaddr*)&from,&fromlen);
	//use wsa instead as this might be stopped: not
	//m_client=WSAAccept(m_server,(struct sockaddr*)&from,&fromlen, NULL, NULL);
	if (ret > 0) 
	{
		m_client = accept(m_server, (struct sockaddr*)&from,&fromlen);
		if (m_client == INVALID_SOCKET) 
		{
			qDebug() << "accept failed with error: " << WSAGetLastError();
			//closesocket(ListenSocket);
			//WSACleanup();
			return 0;
		} else
		{
			//sprintf_s(temp,"Your IP is %s\r\n",inet_ntoa(from.sin_addr));
			//sprintf_s(temp,"x=", getMovX(), "to", timedout);
			_itoa_s(getMovX(),tekst,10);
			sprintf_s(temp,"x= %s en timedout: %i \r\n",tekst, timedout);
			//we simply send this string to the client
			send(m_client,temp,strlen(temp),0);
			qDebug() << temp << "x=" << getMovX() << "txt" << tekst;
			//qDebug() << "Connection from " << inet_ntoa(from.sin_addr) <<"\r\n";
			//remooved to keep socket alive
			closesocket(m_client);  
		}
		timedout = 0;
	}
	else 
	{
		timedout++;
		//qDebug() << "timedout";
	}	
    //close the client socket
    //closesocket() closes the socket and releases the socket descriptor
	//closesocket(m_client);   
	
    //originally this function probably had some use
    //currently this is just for backward compatibility
    //but it is safer to call it as I still believe some
    //implementations use this to terminate use of WS2_32.DLL 
    //i call it now in stop and deinit
	//WSACleanup();
	
    return 1;
}

/** propery methods */

int genericTCPServerProcessor::getPort() const
{
    QMutexLocker lock(m_propertyMutex);
    return m_port;
}

void genericTCPServerProcessor::setPort(int port, bool doEmit )
{
    QMutexLocker lock(m_propertyMutex);
    m_port = port;
	//here?? changing the port
	m_local.sin_port=htons((u_short)getPort());
    lock.unlock();
    if( doEmit ) emit( portChanged(port) );
}

int genericTCPServerProcessor::getMovX() const
{
    QMutexLocker lock(m_propertyMutex);
    return m_movx;
}

void genericTCPServerProcessor::setMovx(int num)
{
	QMutexLocker lock(m_propertyMutex);
	//if( movx >= -100 && movx <= 100)
    //{
        m_movx = num;
	//}
	emit movXChanged(m_movx);
}

int genericTCPServerProcessor::getMovY() const
{
    QMutexLocker lock(m_propertyMutex);
    return m_movy;
}

void genericTCPServerProcessor::setMovy(int movy)
{
	QMutexLocker lock(m_propertyMutex);
	if( movy >= -100 && movy <= 100)
    {
        m_movy = movy;
	}
	emit movYChanged(m_movy);
}

bool genericTCPServerProcessor::getConvertCvMatDataToQImage() const
{
    QMutexLocker lock(m_propertyMutex);
    return m_convertCvMatToQImage;
}

void genericTCPServerProcessor::setConvertCvMatDataToQImage(bool doConvert, bool doEmit)
{
    QMutexLocker lock(m_propertyMutex);
    m_convertCvMatToQImage = doConvert;
    lock.unlock();
    if( doEmit ) emit convertCvMatDataToQImageChanged(doConvert);
}



bool genericTCPServerProcessor::isReadyForProcessing() const
{
    QMutexLocker lock( m_propertyMutex );
    return !m_waiting;
}

void genericTCPServerProcessor::acceptConfigurationRequest()
{
}

void genericTCPServerProcessor::stalled(ServerConnection* connection)
{
    Q_UNUSED(connection)
    QMutexLocker lock( m_propertyMutex );
    //qDebug() << "TCPServerProcessor: a connection stalled";
    m_waiting = true;
}

// TODO: do a connection check, we cannot be lossless with
// more than one connection now
void genericTCPServerProcessor::unstalled(ServerConnection* connection)
{
    Q_UNUSED(connection)
    QMutexLocker lock( m_propertyMutex );
    //qDebug() << "TCPServerProcessor: a connection unstalled";
    m_waiting = false;
}

void genericTCPServerProcessor::serverError(PlvErrorType type, const QString& msg)
{
    // propagate to pipeline element error handling
    setError(type,msg);
    emit onError(type,this);
}




//removed from org process:

//    QVariantList frameData;
//
//    PipelineElement::InputPinMap::iterator itr = m_inputPins.begin();
//    for( ; itr != m_inputPins.end(); ++itr )
//    {
//        plv::IInputPin* pin = itr->second.getPtr();
//        if( pin->isConnected() )
//        {
//
//            if( pin->getTypeId() == m_cvMatDataTypeId &&
//                m_convertCvMatToQImage )
//            {
//                QVariant v;
//                pin->getVariant(v);
//                plv::CvMatData data = v.value<plv::CvMatData>();
//                QImage img = plvgui::ImageConverter::cvMatToQImageNoThrow(data);
//                QVariant v2;
//                v2.setValue(img);
//                frameData.append(v2);
//            }
//            else
//            {
//                QVariant v;
//                pin->getVariant(v);
//                frameData.append(v);
//            }
//        }
//    }
//    quint32 frameNumber = (quint32)getProcessingSerial();
//
//    QByteArray block;
//    QDataStream out(&block, QIODevice::WriteOnly);
//    out.setVersion(QDataStream::Qt_4_0);
//
//    // write the header
//    //out << (quint32)0; // reserved for number of bytes later
//    //out << (quint32)PROTO_FRAME;    // msg type
//    //out << (quint32)frameNumber;    // frame number / serial number
//    //out << (quint32)frameData.size(); // number of arguments
//
//
//	out << (unsigned int)0; // reserved for number of bytes later
//    out << (unsigned int)PROTO_FRAME;    // msg type
//    out << (unsigned int)frameNumber;    // frame number / serial number
//    out << (unsigned int)frameData.size(); // number of arguments	
//
//	  // write all data to the stream
//    foreach( const QVariant& v, frameData )
//    {
//#if 0
//        // we do not want QImage to convert
//        // to the default PNG because that
//        // is way too slow
//        if( v.type() == QVariant::Image )
//        {
//            out << QVariant::UserType;
//
//            if(out.version() >= QDataStream::Qt_4_2)
//                out << qint8(v.isNull());
//
//            QByteArray a("plv::QImageWrapper");
//            out << a;
//
//            if( !v.isValid() )
//            {
//                out << QString();
//            }
//            else
//            {
//                QImage img = v.value<QImage>();
//                QImageWriter writer(out.device(), "png");
//                if(!writer.write(img))
//                {
//                    QString err = writer.errorString();
//                    qWarning() << "Failed to write image to stream because " << err;
//                }
//            }
//        }
//        else
//        {
//            out << v;
//        }
//#else
//        out << v;
//#endif
//    }
//
//    // calculate size of total data and write it as first 4 bytes
//    out.device()->seek(0);
//    out << (unsigned int)(block.size() - sizeof(unsigned int));
//
//    m_server->sendFrame(frameNumber, block);
//    return true;


//void genericTCPServerProcessor::inputConnectionSet(plv::IInputPin* pin, plv::PinConnection* connection)
//{
//   /* QString name = pin->getName();
//    QString otherName = connection->fromPin()->getName();
//
//    QString msg = tr("TCPServerProcessor input pin %1 connection set with %2")
//                  .arg(name).arg(otherName);
//
//    qDebug() << msg;
//
//    QString newName = QString("%1: %2").arg(pin->getId()).arg(pin->getTypeName());
//    pin->setName(newName);
//    plv::createDynamicInputPin( "generic pin", this, plv::IInputPin::CONNECTION_OPTIONAL );*/
//}
//
//void genericTCPServerProcessor::inputConnectionRemoved(plv::IInputPin* pin, plv::PinConnection* connection)
//{
//    /*QString name = pin->getName();
//    QString otherName = connection->fromPin()->getName();
//
//    QString msg = tr("TCPServerProcessor input pin %1 connection removed with %2")
//                  .arg(name).arg(otherName);
//
//    qDebug() << msg;
//    removeInputPin(pin->getId());*/
//}

//void genericTCPServerProcessor::outputConnectionAdded(plv::IOutputPin* pin, plv::PinConnection* connection)
//{
//    /*QString name = pin->getName();
//    QString otherName = connection->toPin()->getName();
//
//    QString msg = tr("TCPServerProcessor output pin %1 connection removed with %2")
//                  .arg(name).arg(otherName);
//
//    qDebug() << msg;*/
//}
//
//void genericTCPServerProcessor::outputConnectionRemoved(plv::IOutputPin* pin, plv::PinConnection* connection)
//{
//   /* QString name = pin->getName();
//    QString otherName = connection->toPin()->getName();
//
//    QString msg = tr("TCPServerProcessor output pin %1 connection removed with %2")
//                  .arg(name).arg(otherName);
//
//    qDebug() << msg;*/
//}


//int genericTCPServerProcessor::getMaxFramesInFlight() const
//{
//    Q_ASSERT( m_server != 0 );
//    //return m_server->getMaxFramesInFlight();
//}

//int genericTCPServerProcessor::getMaxFramesInQueue() const
//{
//    Q_ASSERT( m_server != 0 );
//    return m_server->getMaxFramesInQueue();
//}

//bool genericTCPServerProcessor::getLossless() const
//{
//    Q_ASSERT( m_server != 0 );
//    return m_server->getLossless();
//}

//void genericTCPServerProcessor::setMaxFramesInQueue(int max)
//{
//    QMutexLocker lock(m_propertyMutex);
//    if( max > 0 )
//    {
//        m_server->setMaxFramesInQueue(max);
//        lock.unlock();
//        emit maxFramesInQueueChanged(max);
//    }
//}
//
//void genericTCPServerProcessor::setMaxFramesInFlight(int max)
//{
//    QMutexLocker lock(m_propertyMutex);
//    if( max > 0 )
//    {
//        m_server->setMaxFramesInFlight(max);
//        lock.unlock();
//        emit maxFramesInFlightChanged(max);
//    }
//}
//
//void genericTCPServerProcessor::setLossless(bool lossless)
//{
//    QMutexLocker lock(m_propertyMutex);
//    m_server->setLossless(lossless);
//    lock.unlock();
//    emit losslessChanged(lossless);
//}