#ifndef TRACKTOSTRINGCONVERTER_H
#define TRACKTOSTRINGCONVERTER_H

#include <plvcore/PipelineProcessor.h>
#include <plvcore/CvMatDataPin.h>
//#include "Blob.h"
#include "BlobTrack.h"

namespace plvblobtracker
{

class TrackToStringConverter : public plv::PipelineProcessor
{
    Q_OBJECT
    Q_DISABLE_COPY( TrackToStringConverter )

    Q_CLASSINFO("author", "Richard Loos- edited by Robby van Delden")
    Q_CLASSINFO("name", "Track to String Converter")
    Q_CLASSINFO("description", "A processor which converts a track list to a string with it ids,position,direction and velocity.")

	Q_PROPERTY( bool saveToFile READ getSaveToFile WRITE setSaveToFile NOTIFY saveToFileChanged )

public:
    TrackToStringConverter();
    virtual ~TrackToStringConverter();
	virtual bool init();
	bool getSaveToFile() {return m_saveToFile;}

    /** required standard method declaration for plv::PipelineProcessor */
    PLV_PIPELINE_PROCESSOR
signals:
	void saveToFileChanged (bool b);

public slots:
	void setSaveToFile(bool b);

private:
    plv::InputPin< QList<plvblobtracker::BlobTrack> >* m_inputBlobs;
    plv::OutputPin<QString>* m_outputPin;
	bool m_saveToFile;
	unsigned short halfhour; 
	//bool m_timenotset;
	QString m_filename;
};

}

#endif // VPBLOBTOSTRINGCONVERTER_H
