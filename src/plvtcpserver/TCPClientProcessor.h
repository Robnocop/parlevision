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

#ifndef TCPCLIENTPROCESSOR_H
#define TCPCLIENTPROCESSOR_H

//#include <plvcore/PipelineProducer.h>
#include <plvcore/PipelineProcessor.h>
#include <plvcore/Pin.h>
#include <plvcore/Types.h>
#include <plvcore/CvMatDataPin.h>


#include <QAbstractSocket>

QT_FORWARD_DECLARE_CLASS( QTcpSocket )
QT_FORWARD_DECLARE_CLASS( QNetworkSession )

class TCPClientProcessor : public plv::PipelineProcessor
{
    Q_OBJECT
    //PLV_PIPELINE_PRODUCER
	Q_DISABLE_COPY( TCPClientProcessor )

    Q_CLASSINFO("author", "Robby van Delden")
    Q_CLASSINFO("name", "Active TCP Client")
    Q_CLASSINFO("description", "TCP client sending messages"
	"Currently on my PC it will not work on cable based TCP connection"
	"it is used for a stable serer at which this client has to connect to and send the data it gets"
	"the other client, receives data in order to process it."
	"This actively sends information to the server. Specifically designed for an Arduino-like activated Blox Ball")
	
    Q_PROPERTY( int port READ getPort WRITE setPort NOTIFY portChanged  )
    Q_PROPERTY( QString serverIP READ getServerIP WRITE setServerIP NOTIFY serverIPChanged )
    Q_PROPERTY( bool autoReconnect READ getAutoReconnect WRITE setAutoReconnect NOTIFY autoReconnectChanged  )
	
	Q_PROPERTY( bool overrideString READ getOverrideString WRITE setOverrideString NOTIFY overrideStringChanged  )
	Q_PROPERTY( QString someString READ getSomeString WRITE setSomeString NOTIFY someStringChanged )


	/** required standard method declaration for plv::PipelineProcessor */
    PLV_PIPELINE_PROCESSOR

public:
    TCPClientProcessor();
    virtual ~TCPClientProcessor();

    virtual bool init();
    virtual bool deinit() throw();
    virtual bool start();
    virtual bool stop();

   // bool parseFrame(QDataStream& in);
    //bool parseConfig(QDataStream& in);

    int getPort() const;
    QString getServerIP() const;
    bool getAutoReconnect() const;

	bool  getOverrideString() {return m_overrideString;}
	QString getSomeString() { return m_someString; }
	

signals:
    void portChanged(int port);
    void serverIPChanged(QString ip);
    void autoReconnectChanged(bool ar);
	void overrideStringChanged(bool newVAlue);
    void someStringChanged(QString newValue);

public slots:
    void setPort(int port, bool doEmit=false );
    void setServerIP( const QString& ip, bool doEmit=false );
    //void readData();
    void sessionOpened();
    void displayError(QAbstractSocket::SocketError socketError, bool signal=true);
    void connected();
    void disconnected();
    void setAutoReconnect(bool ar, bool doEmit=false );
	void setOverrideString(bool b);
    void setSomeString(QString s) {m_someString = s; emit(someStringChanged(s));}

private:
    //void ackFrame(quint32 frameNumber);
	bool readyToProduceOnInit() const;
	void sendString(std::string msgCommando);

    QTcpSocket* m_tcpSocket;
    QString m_ipAddress;
    int m_port;
    int m_blockSize;
    QNetworkSession* m_networkSession;
    bool m_configured;
    QVector<QVariant::Type> m_types;
    QList<QVariantList> m_frameList;
    QMutex m_frameListMutex;
    bool m_autoReconnect;
	bool m_failedAtLoading;
	
	//be able to manualy type a command and send this command at the boolean click whatever input there is 
	bool m_overrideString;
    QString m_someString;

    plv::OutputPin<int>* m_intOut;
    plv::OutputPin<QString>* m_stringOut;
	plv::OutputPin<float>* m_floatOut;
    plv::OutputPin<double>* m_doubleOut;
    plv::OutputPin<cv::Scalar>* m_cvScalarOut;
    plv::CvMatDataOutputPin* m_imageOut1;
    plv::CvMatDataOutputPin* m_imageOut2;

	plv::InputPin<QString>* m_inputPin;
	plv::OutputPin<QString>* m_stringOut2;
	bool m_stopped;
};

#endif // TCPClientProcessor_H
