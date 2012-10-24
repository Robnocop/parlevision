#include "TrackToStringConverter.h"

using namespace plv;
using namespace plvblobtracker;

#include <opencv/cv.h>

TrackToStringConverter::TrackToStringConverter()
{
    m_inputBlobs = createInputPin< QList<BlobTrack> >( "input", this );
  

    m_outputPin = createOutputPin<QString>( "output", this );
}

TrackToStringConverter::~TrackToStringConverter()
{
}

bool TrackToStringConverter::process()
{
    QList<plvblobtracker::BlobTrack> tracks = m_inputBlobs->get();

    QString out = QString("FRAME:%1#").arg( this->getProcessingSerial() );

    // convert coordinates to what the virtual playground expects
    foreach( BlobTrack t , tracks )
    {
        cv::Point p = t.getLastMeasurement().getCenterOfGravity();
        //p.x = image.width() - p.x;
        //p.y = image.height() - p.y;
		//playgroundQString blobString = QString("BEGIN_MARKER#MARKER_ID:%1#MARKER_CENTER_X:%2#MARKER_CENTER_Y:%3#").arg(t.getID()).arg(p.x).arg(p.y);
		QString blobString = QString("BEGIN_MARKER#MARKER_ID:%1#MARKER_CENTER_X:%2#MARKER_CENTER_Y:%3#DIRECTION:%4#SPEED:%5#AVERAGEZ:%6").arg(t.getId()).arg(p.x).arg(p.y).arg(t.getAvgDirection()).arg(t.getAvgVelocity()).arg(t.getAveragePixel());
		out.append(blobString);
		out.append("\n");
    }

    // end TCP frame with newline
    out.append("\n");

    m_outputPin->put(out);
    return true;
}
