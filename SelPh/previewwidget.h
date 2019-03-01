#ifndef PREVIEWWIDGET_H
#define PREVIEWWIDGET_H

#include <QOpenGLWidget>
#include <QImage>

class PreviewWidget : public QOpenGLWidget
{
    Q_OBJECT
public:
    explicit PreviewWidget(QWidget *parent = 0);

    void setCurrentImage(const QImage &image);

    QSize sizeHint() const override;

public slots:
    void initializeGL() override;
    void paintGL() override;
    void resizeGL(int w, int h) override {}

private:
    QImage image;

    GLuint shaderProgram;
    GLuint texture;
    GLint texLocation;
    GLint p1Location;
    GLint p2Location;
};

#endif // PREVIEWWIDGET_H
