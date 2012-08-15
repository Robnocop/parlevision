#ifndef NAOBLOB_H
#define NAOBLOB_H

#include <plvcore/PipelineProcessor.h>
#include <plvcore/CvMatDataPin.h>
#include "Blob.h"

namespace plvblobtracker
{

class NaoBlob : public plv::PipelineProcessor
{
    Q_OBJECT
    Q_DISABLE_COPY( NaoBlob )

    Q_CLASSINFO("author", "Robby van Delden using VPToString from Richard Loos")
    Q_CLASSINFO("name", "NaoBlobDetector")
    Q_CLASSINFO("description", "A processor which converts a blob (XYZ) to a Virtual Playground compatible string. In order to create an interactive nao-robot.")

public:
    NaoBlob();
    virtual ~NaoBlob();

    /** required standard method declaration for plv::PipelineProcessor */
    PLV_PIPELINE_PROCESSOR

private:
    plv::InputPin< QList<plvblobtracker::Blob> >* m_inputBlobs;
    plv::CvMatDataInputPin* m_inputImage;
    plv::OutputPin<QString>* m_outputPin;
	plv::CvMatDataOutputPin* m_outputImage2;
	plv::CvMatDataOutputPin* m_outputImage3;
};

}

#endif // VPBLOBTOSTRINGCONVERTER_H
