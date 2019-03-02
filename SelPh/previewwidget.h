#ifndef PREVIEWWIDGET_H
#define PREVIEWWIDGET_H

#include <QOpenGLWidget>
#include <QImage>

class PreviewWidget : public QOpenGLWidget
{
public:
    explicit PreviewWidget(QWidget *parent = 0);

    void setCurrentImage(const QImage &image);

    QSize sizeHint() const override;

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
