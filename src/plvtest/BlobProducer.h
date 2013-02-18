#ifndef BLOBPRODUCER_H
#define BLOBPRODUCER_H

#include <plvcore/PipelineProducer.h>
#include <plvcore/CvMatDataPin.h>

class BlobProducer : public plv::PipelineProducer
{
public:
    Q_OBJECT
    Q_DISABLE_COPY( BlobProducer )

    Q_CLASSINFO("author", "Richard Loos")
    Q_CLASSINFO("name", "BlobProducer")
    Q_CLASSINFO("description", "A producer which generates an image with blobs.")
    Q_PROPERTY( int maxStep READ getMaxStep WRITE setMaxStep NOTIFY maxStepChanged )
    Q_PROPERTY( int numBlobs READ getNumBlobs WRITE setNumBlobs NOTIFY numBlobsChanged )
	Q_PROPERTY( int pixelShift READ getPixelShift WRITE setPixelShift NOTIFY pixelShiftChanged 
	Q_PROPERTY( int blobSize READ getBlobSize WRITE setBlobSize NOTIFY blobSizeChanged )

    /** required standard method declaration for plv::PipelineProcessor */
    PLV_PIPELINE_PRODUCER

    BlobProducer();
    virtual ~BlobProducer();

    bool init();

    int getMaxStep() const;
    int getNumBlobs() const;
	int getBlobSize() const;
	int getPixelShift() const;
	int getColor(int blobid, int elapsedtime);



public slots:
    void setMaxStep(int step);
    void setNumBlobs(int num);
	void setPixelShift(int pixel);
	void setBlobSize(int pixels);

signals:
    void maxStepChanged(int s);
    void numBlobsChanged(int n);
	void pixelShiftChanged(int p);
	void blobSizeChanged(int p);

private:
    int m_width;
    int m_height;
    int m_maxStep;
	int m_blobSize;
    int m_numBlobs;
	int m_pixelShift;
	int m_colorp;
    double m_factor;
    QVector<cv::Point> m_positions;
    QVector<cv::Point> m_targets;
    plv::CvMatDataOutputPin* m_outputPin;
	//TEST FPS callback from one plugin
	QTime m_timeSinceLastFPSCalculation;
    int m_numFramesSinceLastFPSCalculation;
    float m_fps; /** running avg of fps */
};

#endif // BLOBPRODUCER_H
