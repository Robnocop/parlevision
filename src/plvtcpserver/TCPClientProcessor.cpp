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

#include "TCPClientProcessor.h"
#include <QtNetwork>
#include "Proto.h"
#include <limits>

TCPClientProcessor::TCPClientProcessor() :
    m_tcpSocket(new QTcpSocket(this)),
    m_ipAddress(QHostAddress(QHostAddress::LocalHost).toString()),
    m_port(1337),
    m_blockSize(0),
    m_networkSession(0),
    m_configured( true ),
    m_autoReconnect( false )
{
    m_intOut      = plv::createOutputPin<int>("int", this);
    m_stringOut   = plv::createOutputPin<QString>("QString", this);
    m_doubleOut   = plv::createOutputPin<double>("double", this);
    m_cvScalarOut = plv::createOutputPin< cv::Scalar >("cv::Scalar", this);
    m_imageOut1    = plv::createCvMatDataOutputPin("CvMatData1", this);
    m_imageOut2    = plv::createCvMatDataOutputPin("CvMatData2", this);

	m_inputPin = plv::createInputPin<QString>("input String", this);
	m_stringOut2 = plv::createOutputPin<QString>( "output String", this );

    // Try to optimize the socket for low latency.
    // For a QTcpSocket this would set the TCP_NODELAY option
    // and disable Nagle's algorithm.
    //m_tcpSocket->setSocketOption(QAbstractSocket::LowDelayOption, 1);

	//fix the fucking problem of error on stop and crash on error
    // connect(m_tcpSocket, SIGNAL(readyRead()), this, SLOT(readData()));
    connect(m_tcpSocket, SIGNAL(error(QAbstractSocket::SocketError)),
            this, SLOT(displayError(QAbstractSocket::SocketError)));

    connect(m_tcpSocket, SIGNAL(disconnected()), this, SLOT(disconnected()));
    connect(m_tcpSocket, SIGNAL(connected()), this, SLOT(connected()));
}

TCPClientProcessor::~TCPClientProcessor()
{
    m_tcpSocket->disconnect();
    delete m_tcpSocket;
}



bool TCPClientProcessor::init()
{
    assert( m_networkSession == 0 );
    assert( m_tcpSocket != 0 );

    QNetworkConfigurationManager manager;
    if (manager.capabilities() & QNetworkConfigurationManager::NetworkSessionRequired)
    {
        // Get saved network configuration
        QSettings settings(QSettings::UserScope, QLatin1String("Trolltech"));
        settings.beginGroup(QLatin1String("QtNetwork"));
        const QString id = settings.value(QLatin1String("DefaultNetworkConfiguration")).toString();
        settings.endGroup();

        // If the saved network configuration is not currently discovered use the system default
        QNetworkConfiguration config = manager.configurationFromIdentifier(id);
        if ((config.state() & QNetworkConfiguration::Discovered) !=
            QNetworkConfiguration::Discovered)
        {
            config = manager.defaultConfiguration();
        }

        m_networkSession = new QNetworkSession(config);
        connect(m_networkSession, SIGNAL(opened()), this, SLOT(sessionOpened()));

        qDebug() << "Opening network session.";
        m_networkSession->open();
    }
    return true;
}

bool TCPClientProcessor::deinit() throw ()
{
    if( m_networkSession )
    {
        m_networkSession->close();
        delete m_networkSession;
        m_networkSession = 0;
    }

    return true;
}

bool TCPClientProcessor::start()
{
	m_stopped = false;
	m_failedAtLoading = false;
    // connect to server
	////seems identical to readyToProduceOnInit();

    // if we did not find one, use IPv4 localhost
    if( m_ipAddress.isEmpty() )
    {
        m_ipAddress = QHostAddress(QHostAddress::LocalHost).toString();
        qWarning() << "No valid IP address given, using localhost";
    }

    QHostAddress address( m_ipAddress );

    qDebug() << "Connecting to " << address.toString() << ":" << m_port;

    // skip all data left in socket
    qint64 bytes = m_tcpSocket->bytesAvailable();
    if( bytes > 0 )
    {
        QDataStream in(m_tcpSocket);
        in.setVersion(QDataStream::Qt_4_0);

        // can skip 2^32 bytes at a time, skip until no bytes left
        //while( bytes > (qint64) std::numeric_limits<int>::max() )
		//bugfix attempt seems more reasonable:
		while( bytes < (qint64) std::numeric_limits<int>::max() )
        {
            in.skipRawData( std::numeric_limits<int>::max() );
            bytes -= (qint64) std::numeric_limits<int>::max();
        }
        in.skipRawData( bytes );
        
        assert( m_tcpSocket->bytesAvailable() == 0 );
        m_blockSize = 0;
    }

    // if this fails, error is automatically called
    // by signal slots connection
    m_tcpSocket->connectToHost( address, m_port );

    int timeout = 5*1000;
    if(m_tcpSocket->waitForConnected(timeout))
    {
		//when succesfull:
		//qDebug() << "not tcp socket timeout TRUE";
		return true;
	}
	else
	{
		//if false this one is reached:
		//qDebug() << "not tcp socket timeout FALSE";
	}

    if( m_autoReconnect )
    {
        qWarning() << "TCPClientProcessor failed to connect at startup, will keep trying to reconnect because autoreconnect is enabled.";
        return true;
    }

	//last minute CHANGED!!
    //displayError(m_tcpSocket->error(), false);
	m_failedAtLoading = true;
    //return false;
	return true;
}

bool TCPClientProcessor::stop()
{
    // disconnect from server
    qDebug() << "Disconnecting ... ";
	if(!m_failedAtLoading)
	{
		m_tcpSocket->disconnectFromHost();

		// if not immediately disconnected wait 5 seconds
		int timeout = 5*1000;
		if( m_tcpSocket->state() == QAbstractSocket::UnconnectedState || m_tcpSocket->waitForDisconnected(timeout) )
		{
			m_tcpSocket->abort();
			return true;
		}
		displayError(m_tcpSocket->error(), false);
		m_tcpSocket->abort();
		m_blockSize = 0;
	}
    //last min changed 
	//return false;
	return true;
}

//processor has no ready to produce instead  do something clever:
bool TCPClientProcessor::readyToProduceOnInit() const
{
    // check if we have a connection
	if (!m_stopped)
	{
		QAbstractSocket::SocketState state = m_tcpSocket->state(); 
		//qDebug() << "state"<< state << "unconne" << QAbstractSocket::UnconnectedState; 
		if( state == QAbstractSocket::UnconnectedState )
		{
			QHostAddress address( m_ipAddress );

			qWarning() << "Reconnecting to " << address.toString() << ":" << m_port;

			// if this fails, error is automatically called
			// by signal slots connection
			m_tcpSocket->connectToHost( address, m_port );

			//set the timeout on a really short interval instead of 5s
			int timeout = 4*100;
			if(!m_tcpSocket->waitForConnected(timeout))
			{
				qWarning() << "TCPClientProcessor failed to reconnect";
				return false;
			}
			else
				return true;
		}
		else if(state == QAbstractSocket::ConnectedState)
		{
			//qDebug() << "connected";
			return true;
		}
		else
		{
			return false;
		}
	}
	else 
		return false;
	//"connected" << QAbstractSocket::ConnectedState  << 3
	//conecting ==2 ==hostnotfound 
	//connection refused == 0 
	//QAbstractSocket::
	// check if there is data available, no longer needed
    //return !m_frameList.isEmpty();
	
	//unreachable
}

//no produce for a processor instead a process is used, 
bool TCPClientProcessor::process()
{
	//UGLY last minute changed solution:
	//stop will throw a less invasive error than a return false start error, (leading to a major memory leak )
	//mandatory get function is not called after all
	if 	(m_failedAtLoading)
		stop();
	else
	{
		bool debugging = false;
		if (debugging)
			qDebug() << "enter tcpclient" <<this->getProcessingSerial();
		QString src = m_inputPin->get();
		if (debugging)
			qDebug() << "got inputpin tcpclient, checking ready" <<this->getProcessingSerial();
		if (readyToProduceOnInit() )
		{
			if (debugging)
				qDebug() << "enter ready";
		
			if(getOverrideString())
			{
				sendString(getSomeString().toStdString());
			}
			else
			{
				//compare the incoming tring to "-" and send the string in stad string format otherwise
				if(src != "-")
					sendString(src.toStdString());

			}
			if (debugging)
				qDebug() << "exit ready";
		}
		else
		{
			qDebug() <<"not ready to produce on Init"<<this->getProcessingSerial();
		}

		//show the used string
		if(getOverrideString())
		{
			m_stringOut2->put(getSomeString());
		}
		else
		{
			m_stringOut2->put(src);
		}

		if (debugging)
			qDebug() << "exit tcpclient"<<this->getProcessingSerial();
	}
    return true;
}


void TCPClientProcessor::sendString(std::string msgCommando)
{
	//FOR THE BLOX BALL send as 8localbit
	QByteArray bytes2;
	QDataStream out2(&bytes2, QIODevice::WriteOnly);
	out2.setVersion(QDataStream::Qt_4_0);
	//makes little sense but this does work for both unity and the blox ball
	QString sendmsg = QString::fromUtf8(msgCommando.c_str())+ "\n";
	QAbstractSocket::SocketState state = m_tcpSocket->state();  
	if (state == QAbstractSocket::ConnectedState && m_tcpSocket->write(sendmsg.toLocal8Bit()) == -1)
	{
		qDebug() << "tried to send and it did not work";
	}
	else
	{
		//m_tcpSocket->flush();
	}

}



void TCPClientProcessor::sessionOpened()
{
    // Save the used configuration
    QNetworkConfiguration config = m_networkSession->configuration();
    QString id;
    if (config.type() == QNetworkConfiguration::UserChoice)
        id = m_networkSession->sessionProperty(QLatin1String("UserChoiceConfiguration")).toString();
    else
        id = config.identifier();

    QSettings settings(QSettings::UserScope, QLatin1String("Trolltech"));
    settings.beginGroup(QLatin1String("QtNetwork"));
    settings.setValue(QLatin1String("DefaultNetworkConfiguration"), id);
    settings.endGroup();
}

void TCPClientProcessor::connected()
{
    qDebug() << "TCPClientProcessor connected";
}

void TCPClientProcessor::disconnected()
{
    qDebug() << "TCPClientProcessor disconnected";
    m_stopped = true;
    qint64 bytes = m_tcpSocket->bytesAvailable();
    if( bytes > 0 )
    {
        QDataStream in(m_tcpSocket);
        in.setVersion(QDataStream::Qt_4_0);

        while( bytes > (qint64) std::numeric_limits<int>::max() )
        {
            in.skipRawData( std::numeric_limits<int>::max() );
            bytes -= (qint64) std::numeric_limits<int>::max();
        }
        in.skipRawData( bytes );
        
        qint64 bytes = m_tcpSocket->bytesAvailable();
        if( bytes > 0 )
        {
            qDebug() << QString("%1 bytes left in socket after reconnect").arg(bytes);
        }
        m_blockSize = 0;
    }
    //setError( PlvPipelineRuntimeError, tr("The remote host closed the connection."));
}

void TCPClientProcessor::displayError(QAbstractSocket::SocketError socketError, bool signal)
{
    QString msg;
    PlvErrorType type = PlvNoError;

    switch (socketError)
    {
    case QAbstractSocket::RemoteHostClosedError:
        type = PlvPipelineRuntimeError;
        msg = tr("The remote host closed the connection.");
        break;
    case QAbstractSocket::HostNotFoundError:
        type = PlvPipelineRuntimeError;
        msg = tr("The host was not found. Please check the "
                 "host name and port settings.");
        break;
    case QAbstractSocket::ConnectionRefusedError:
        type = PlvPipelineRuntimeError;
        msg = tr("The connection was refused by the peer. "
                 "Make sure the Parlevision server is running, "
                 "and check that the host name and port "
                 "settings are correct.");
        break;
    default:
       type = PlvPipelineRuntimeError;
       msg = tr("The following error occurred: %1.").arg(m_tcpSocket->errorString());
    }

    if( !m_autoReconnect )
    {
        setError(type,msg);
        if(signal) emit onError(type,this);
    }
    else
    {
        //emit sendMessageToPipeline( PlvNotifyMessage, msg );
        qWarning() << msg;
    }
}

/** propery methods */


void TCPClientProcessor::setOverrideString(bool b)
{
	if(b) 
	{
		m_overrideString = true;

	}
	else
	{
		m_overrideString = false;
	}

	emit(overrideStringChanged(b));
}


int TCPClientProcessor::getPort() const
{
    QMutexLocker lock(m_propertyMutex);
    return m_port;
}

void TCPClientProcessor::setPort(int port, bool doEmit )
{
    QMutexLocker lock(m_propertyMutex);
    m_port = port;
    if( doEmit ) emit( portChanged(port) );
}

/** propery methods */
QString TCPClientProcessor::getServerIP() const
{
    QMutexLocker lock(m_propertyMutex);
    return m_ipAddress;
}

void TCPClientProcessor::setServerIP(const QString& ip, bool doEmit )
{
    QMutexLocker lock(m_propertyMutex);
    m_ipAddress = ip;
    if( doEmit ) emit( serverIPChanged(ip) );
}

bool TCPClientProcessor::getAutoReconnect() const
{
    QMutexLocker lock(m_propertyMutex);
    return m_autoReconnect;
}

void TCPClientProcessor::setAutoReconnect(bool ar, bool doEmit )
{
    QMutexLocker lock(m_propertyMutex);
    m_autoReconnect = ar;
    if( doEmit ) emit( autoReconnectChanged(ar) );
}
