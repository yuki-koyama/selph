#include "imagewidget.h"

#include <QPainter>
#include <QResizeEvent>

ImageWidget::ImageWidget(std::shared_ptr<QImage> img, int height) : img(img)
{
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    setHeight(height);
}

void ImageWidget::setHeight(int height)
{
    setFixedHeight(height);
    setFixedWidth(height * img->width() / img->height());
}

void ImageWidget::paintEvent(QPaintEvent*)
{
    QPainter painter;
    painter.begin(this);

    if (!this->scaledImg->isNull())
    {
        painter.drawPixmap(0, 0, QPixmap::fromImage(*scaledImg, Qt::AutoColor));
    }

    painter.end();
}

void ImageWidget::resizeEvent(QResizeEvent* e)
{
    QWidget::resizeEvent(e);
    scaledImg = std::make_shared<QImage>(this->img->scaled(this->width(), this->height(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
}
