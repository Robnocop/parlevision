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

public:
    TrackToStringConverter();
    virtual ~TrackToStringConverter();

    /** required standard method declaration for plv::PipelineProcessor */
    PLV_PIPELINE_PROCESSOR

private:
    plv::InputPin< QList<plvblobtracker::BlobTrack> >* m_inputBlobs;
    plv::OutputPin<QString>* m_outputPin;
};

}

#endif // VPBLOBTOSTRINGCONVERTER_H
