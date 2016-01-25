#ifndef VISUALIZATIONWIDGET_H
#define VISUALIZATIONWIDGET_H

#include <QWidget>

class VisualizationWidget : public QWidget
{
    Q_OBJECT
public:
    explicit VisualizationWidget(QWidget *parent = 0, int index = -1);

signals:
    
public slots:

protected:
    void paintEvent(QPaintEvent *event);

private:
    int index;
    
};

#endif // VISUALIZATIONWIDGET_H
