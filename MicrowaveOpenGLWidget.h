#ifndef MICROWAVEOPENGLWIDGET_H
#define MICROWAVEOPENGLWIDGET_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QMatrix4x4>
#include <QColor>
#include <QTimer>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>

class MicrowaveOpenGLWidget : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT
public:
    explicit MicrowaveOpenGLWidget(QWidget *parent = nullptr);
    ~MicrowaveOpenGLWidget();

    void setColor(const QColor &color);
    void startRotation();
    void stopRotation();

protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;

private:
    QMatrix4x4 projection;
    QMatrix4x4 model;
    QColor color;
    QTimer rotationTimer;
    float rotationAngle;

    QOpenGLBuffer m_vertex;
    QOpenGLVertexArrayObject m_object;
    QOpenGLBuffer cylinder_vertex;
    QOpenGLVertexArrayObject cylinder_object;

    QOpenGLShaderProgram* shaderProgram;
    QVector<GLfloat> generateCylinderVertices(int radius, int resolution);
    QVector<GLfloat> cylinderVertices;




private slots:
    void updateRotation();
};
#endif // MICROWAVEOPENGLWIDGET_H
