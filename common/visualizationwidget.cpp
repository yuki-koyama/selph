#include "visualizationwidget.h"

#include <iostream>
#include <QPainter>
#include <QPaintEvent>
#include <QVector3D>
#include "core.h"
#include "colorutility.h"
#include "eigenutility.h"

using namespace std;
using namespace Eigen;

namespace {
Core& core = Core::getInstance();
}

VisualizationWidget::VisualizationWidget(QWidget *parent, int index) :
    QWidget(parent)
{
    this->index = index;
    this->setMinimumHeight(core.getSizeOfVisualizationHeight());
    this->setMaximumHeight(core.getSizeOfVisualizationHeight());
}

void VisualizationWidget::paintEvent(QPaintEvent *event)
{
    QPainter painter;

    // start & initialize
    painter.begin(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::HighQualityAntialiasing);
    painter.setRenderHint(QPainter::TextAntialiasing);

    QRect wholeRect = event->rect();
    int w = wholeRect.width();
    int h = wholeRect.height();

    // background
    QColor background(0xff, 0xff, 0xff, 0x00);
    painter.fillRect(event->rect(), background);

    // if not visualization mode
    if (!core.useVisualization) {
        painter.end();
        return;
    }

    // gradation
    int res = core.gradationResolution;
    int wid = w / res;
    vector<double> x = core.parameters;
    for (int i = wid / 2; i < w; i += wid) {
        x[index] = static_cast<double>(i) / static_cast<double>(w - 1);
        double value = core.goodnessFunction.getValue(EigenUtility::std2eigen(x), core.getCurrentFeatureVector());

        // regularize the value into [0, 1]
        value = (value - core.localMin) / (core.localMax - core.localMin);
        value = std::isnan(value) ? 0.5 : value;

        // get heatmap color
        Vector3d colorVec = ColorUtility::getHeatmapColor(value);
        QColor color(colorVec(0) * 255.0, colorVec(1) * 255.0, colorVec(2) * 255.0);

        // control saturation using the confidence value
        qreal _h, _s, _l;
        color.getHslF(&_h, &_s, &_l);
        color.setHslF(_h, core.confidence * _s, _l);

        // draw
        painter.fillRect(i - wid / 2, 0, wid * 2, h, color);
    }

    // current position
    double curPos   = core.parameters[index] * static_cast<double>(w);
    double curWidth = 10.0;
    QRectF curRect(curPos - curWidth / 2.0, 0.0, curWidth, static_cast<double>(h));
    QPen   curPen(QColor(0x00, 0x00, 0x00));
    curPen.setWidth(2);
    painter.setPen(curPen);
    painter.drawRect(curRect);

    // boundary
    QPen   boundaryPen(QColor(0x00, 0x00, 0x00));
    boundaryPen.setWidth(4);
    painter.setPen(boundaryPen);
    painter.drawRect(event->rect());

    // end
    painter.end();
}
