#ifndef FrameWidget_H
#define FrameWidget_H

#include <opencv/cv.h>

class QLabel;
class QPixmap;
class QLabel;
class QVBoxLayout;
class QImage;

#include <QWidget>

namespace plvgui {

class FrameWidget : public QWidget {
    Q_OBJECT

    private:
        QLabel*         m_imagelabel;
        QVBoxLayout*    m_layout;
        QImage          m_image;
        
    public:
        FrameWidget( QWidget *parent );
        ~FrameWidget();

        void putImage( const IplImage* );

    public slots:
        void setFrame( const IplImage* frame );
}; 

}

#endif