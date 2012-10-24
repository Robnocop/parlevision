#ifndef TOUCHBLOBANDSTRINGCONVERTER_H
#define TOUCHBLOBANDSTRINGCONVERTER_H

#include <plvcore/PipelineProcessor.h>
#include <plvcore/CvMatDataPin.h>
#include "Blob.h"

namespace plvblobtracker
{

class TouchBlobAndStringConverter : public plv::PipelineProcessor
{
    Q_OBJECT
    Q_DISABLE_COPY( TouchBlobAndStringConverter )

    Q_CLASSINFO("author", "Robby van Delden using VPToString from Richard Loos")
    Q_CLASSINFO("name", "AIRScreen to String Converter")
    Q_CLASSINFO("description", "(A processor which converts a closest blob (XYZ) to a Virtual Playground compatible string. In order to create touchscreen. The depth sesor should be in the middle of the screen, the image should be crop to the screen, the incoming image should be symmetric around the middle of the screen as well. The image should have been threholded zo no depth values except the finger will be in a single blob. The Processor expects both the blobs and the depthimage input. Source file is TouchBlobAndStringConverter.cpp. ")

	Q_PROPERTY( int minZ READ getMinZ WRITE setMinZ NOTIFY minZChanged )
	Q_PROPERTY( int maxZ READ getMaxZ WRITE setMaxZ NOTIFY maxZChanged )


	public:
		TouchBlobAndStringConverter();
		virtual ~TouchBlobAndStringConverter();

		int getMinZ() const;
		int getMaxZ() const;

		/** required standard method declaration for plv::PipelineProcessor */
		PLV_PIPELINE_PROCESSOR

	public slots: 
		void setMinZ( int z );
		void setMaxZ( int z );

	signals:
		void minZChanged( int z );
		void maxZChanged( int z );

	private:
		plv::InputPin< QList<plvblobtracker::Blob> >* m_inputBlobs;
		plv::CvMatDataInputPin* m_inputImage;
		plv::OutputPin<QString>* m_outputPin;
		plv::CvMatDataOutputPin* m_outputImage2;
		plv::CvMatDataOutputPin* m_outputImage3;
		int m_minZ;
		int m_maxZ;
		//vector<int> orderSkels;
};

}

#endif // VPBLOBTOSTRINGCONVERTER_H
