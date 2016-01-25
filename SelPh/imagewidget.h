#ifndef IMAGEWIDGET_H
#define IMAGEWIDGET_H

#include <memory>
#include <QWidget>

class QImage;

class ImageWidget : public QWidget
{
public:
    ImageWidget(std::shared_ptr<QImage> img, int height);

    void setHeight(int height);
    void paintEvent(QPaintEvent*);

protected:
    void resizeEvent(QResizeEvent* e);

private:
    std::shared_ptr<QImage> img;
    std::shared_ptr<QImage> scaledImg;
};

#endif // IMAGEWIDGET_H
