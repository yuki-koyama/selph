#include "previewwidget.h"

#include <string>
#include <QOpenGLTexture>
#include <QPainter>
#include "drawutility.h"
#include "core.h"
#include "utility.h"

using namespace std;

namespace
{
    Core& core = Core::getInstance();
}

PreviewWidget::PreviewWidget(QWidget *parent) :
QOpenGLWidget(parent)
{
}

QSize PreviewWidget::sizeHint() const
{
    return QSize(1280, 720);
}

void PreviewWidget::setCurrentImage(const QImage &image)
{
    this->image = image;
}

void PreviewWidget::initializeGL()
{
    glEnable(GL_MULTISAMPLE);

    static const string mainShaderName("enhancer");

    const string bundlePath             = Utility::getResourceDirectory() + "/shaders";
    const string mainVertexShaderPath   = bundlePath + "/" + mainShaderName + ".vs";
    const string mainFragmentShaderPath = bundlePath + "/" + mainShaderName + ".fs";

    std::cout << "Load shaders: " << mainVertexShaderPath << " & " << mainFragmentShaderPath << std::endl;

    // set shader program
    const int success = DrawUtility::loadShader(mainVertexShaderPath, mainFragmentShaderPath, &shaderProgram);
    if (success < 0)
    {
        std::cerr << "Shader load error." << std::endl;
        exit(1);
    }

    texLocation = glGetUniformLocation(shaderProgram, "texture");
    p1Location  = glGetUniformLocation(shaderProgram, "first");
    p2Location  = glGetUniformLocation(shaderProgram, "second");

    glClearColor(0.0, 0.0, 0.0, 1.0);
}

void PreviewWidget::paintGL()
{
    // Compute the viewport settings
    const GLint wRate = this->image.width();
    const GLint hRate = this->image.height();
    const GLint w = width() * devicePixelRatio();
    const GLint h = height() * devicePixelRatio();
    if (w * hRate == h * wRate)
    {
        glViewport(0, 0, w, h);
    }
    else if (w * hRate > h * wRate)
    {
        const int w_corrected = h * wRate / hRate;
        glViewport((w - w_corrected) / 2, 0, w_corrected, h);
    }
    else if (w * hRate < h * wRate)
    {
        const int h_corrected = w * hRate / wRate;
        glViewport(0, (h - h_corrected) / 2, w, h_corrected);
    }

    // Draw background and image
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    glUseProgram(shaderProgram);

    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glEnable(GL_TEXTURE_2D);

    QOpenGLTexture* texture = new QOpenGLTexture(this->image.mirrored());
    texture->bind();
    glUniform1ui(texLocation, 0);

    glUniform3f(p1Location, core.parameters[0], core.parameters[1], core.parameters[2]);
    glUniform3f(p2Location, core.parameters[3], core.parameters[4], core.parameters[5]);

    glBegin(GL_TRIANGLE_FAN);
    glTexCoord2d(0.0, 0.0); glVertex2d(-1.0, -1.0);
    glTexCoord2d(1.0, 0.0); glVertex2d(+1.0, -1.0);
    glTexCoord2d(1.0, 1.0); glVertex2d(+1.0, +1.0);
    glTexCoord2d(0.0, 1.0); glVertex2d(-1.0, +1.0);
    glEnd();

    glDisable(GL_TEXTURE_2D);
    glUseProgram(0);
}
