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
#include <QImageWriter>

//CHINESE http://net.pku.edu.cn/~course/cs501/2011/code/BSD_Socket.t/sockets.pdf
//#include <sys/types.h>
//#include <sys/socket.h>


//#include <winsock.h> 

//requires ws2_32.lib allthough i can find the header file the lib linking to the dll needs to be added in properties linker/ or the tcpserver pri file danger is that it can be msvc20210 depended
//needed for amongst other htons functions convert an IP port number in host byte order to the IP port number in network byte order
//#include <conio.h>

//#include <iostream>
//#include <QNetworkInterface>

#include <QDebug>

genericTCPServerProcessor::genericTCPServerProcessor() :
    m_port(20248),
	m_bool(true)
	//m_movx(0),
	//m_movy(0),
  //  m_waiting(false),
//    m_convertCvMatToQImage(false)

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
	qDebug() << "init";
	//if (m_bool)	
	//{
	//	WSAData wsaDataName;
	//	unsigned short vr;
	//	vr = MAKEWORD( 2, 2 );
	//	WSAStartup(vr, &wsaDataName);

	//	char* IPADRESIN = "127.0.0.1";
	//	local.sin_family = AF_INET;
	//	local.sin_addr.s_addr = inet_addr(IPADRESIN);
	//	local.sin_port = 1080; // choose any

	//	char* IPADRESUIT = "127.0.0.1";
	//	dest.sin_family = AF_INET;
	//	dest.sin_addr.s_addr = inet_addr( IPADRESUIT);
	//	dest.sin_port = htons(7777);

	//	qDebug() << "socketsetup";
	//	// create the socket
	//	socketname = socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP );

	//
	//	// bind to the local address
	//	qDebug() << "binding";
	//	int errorcode; 
	//	errorcode = bind( socketname, (sockaddr *)&local, sizeof(local) );
	//		//qDebug() << "bound " << errorcode;
	//}
	//else 
	//{
	//	qDebug() << "not bound";
	//}

	m_bool = false;
	
	return true;
}

bool genericTCPServerProcessor::deinit() throw ()
{
	return true;
}

bool genericTCPServerProcessor::start()
{
	qDebug() << "start";
	return true;
	
}

bool genericTCPServerProcessor::stop()
{
	return true;
}


bool genericTCPServerProcessor::process()
{
	qDebug() << "process step";

	//temp to be dependent of fps of incoming frames
	//qDebug() << "enter process";
	//QVariantList frameData;
	
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

	qDebug() << "sending to" ;
	/*char pkt['bla'];
	size_t pkt_length = 3;
	int ret = sendto( socketname, pkt, pkt_length, 0 , (sockaddr *)&dest, sizeof(dest) );
	qDebug() << "ended send to" ;
*/
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
//	m_local.sin_port=htons((u_short)getPort());
//    lock.unlock();
    if( doEmit ) emit( portChanged(port) );
}

//bool genericTCPServerProcessor::getConvertCvMatDataToQImage() const
//{
//    QMutexLocker lock(m_propertyMutex);
//    return m_convertCvMatToQImage;
//}
//
//void genericTCPServerProcessor::setConvertCvMatDataToQImage(bool doConvert, bool doEmit)
//{
//    QMutexLocker lock(m_propertyMutex);
//    m_convertCvMatToQImage = doConvert;
//    lock.unlock();
//    if( doEmit ) emit convertCvMatDataToQImageChanged(doConvert);
//}

//bool genericTCPServerProcessor::isReadyForProcessing() const
//{
//    QMutexLocker lock( m_propertyMutex );
//    return !m_waiting;
//}

//void genericTCPServerProcessor::serverError(PlvErrorType type, const QString& msg)
//{
//    // propagate to pipeline element error handling
//    setError(type,msg);
//    emit onError(type,this);
//}




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

//
//int genericTCPServerProcessor::getMovX() const
//{
//    QMutexLocker lock(m_propertyMutex);
//    return m_movx;
//}
//
//void genericTCPServerProcessor::setMovx(int num)
//{
//	QMutexLocker lock(m_propertyMutex);
//	//if( movx >= -100 && movx <= 100)
//    //{
//		m_movx = num;
//	//}
//	emit movXChanged(m_movx);
//}
//
//int genericTCPServerProcessor::getMovY() const
//{
//    QMutexLocker lock(m_propertyMutex);
//    return m_movy;
//}
//
//void genericTCPServerProcessor::setMovy(int movy)
//{
//	QMutexLocker lock(m_propertyMutex);
//	if( movy >= -100 && movy <= 100)
//    {
//        m_movy = movy;
//	}
//	emit movYChanged(m_movy);
//}


//
//void genericTCPServerProcessor::acceptConfigurationRequest()
//{
//}
//
//void genericTCPServerProcessor::stalled(ServerConnection* connection)
//{
//    Q_UNUSED(connection)
//    QMutexLocker lock( m_propertyMutex );
//    //qDebug() << "TCPServerProcessor: a connection stalled";
//    m_waiting = true;
//}

// TODO: do a connection check, we cannot be lossless with
// more than one connection now
//void genericTCPServerProcessor::unstalled(ServerConnection* connection)
//{
//    Q_UNUSED(connection)
//    QMutexLocker lock( m_propertyMutex );
//    //qDebug() << "TCPServerProcessor: a connection unstalled";
//    m_waiting = false;
//}
