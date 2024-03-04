#include "MicrowaveOpenGLWidget.h"
#include <QOpenGLShaderProgram>

MicrowaveOpenGLWidget::MicrowaveOpenGLWidget(QWidget *parent) :
    QOpenGLWidget(parent),
    color(Qt::white),
    rotationAngle(0.0f)
{
    rotationTimer.setInterval(20);
    connect(&rotationTimer, &QTimer::timeout, this, &MicrowaveOpenGLWidget::updateRotation); //We update the rotation angle every 20ms
}

MicrowaveOpenGLWidget::~MicrowaveOpenGLWidget()
{
    makeCurrent();
    m_vertex.destroy();
    m_object.destroy();
    delete shaderProgram;
    rotationTimer.stop();
    doneCurrent();
}

QVector<GLfloat> MicrowaveOpenGLWidget::generateCylinderVertices(int radius, int resolution){
    QVector<GLfloat> vertices;

    // Center point
    vertices.append(0.0f); // x
    vertices.append(0.0f); // y
    vertices.append(0.0f); // z

    // Perimeter points
    for (int i = 0; i <= resolution; ++i) {
        float angle = 2.0f * M_PI * float(i) / float(resolution);
        float x = radius * qCos(angle);
        float y = radius * qSin(angle);

        vertices.append(x); // x
        vertices.append(y); // y
        vertices.append(0.0f); // z (flat on the XZ plane)
    }

    return vertices;
}

static const GLfloat vertices[] = {
    // Front face
    -1.0f, -1.0f,  1.0f, // Bottom-left
    1.0f, -1.0f,  1.0f, // Bottom-right
    1.0f,  1.0f,  1.0f, // Top-right
    1.0f,  1.0f,  1.0f, // Top-right
    -1.0f,  1.0f,  1.0f, // Top-left
    -1.0f, -1.0f,  1.0f, // Bottom-left

    // Right face
    1.0f, -1.0f,  1.0f, // Bottom-left
    1.0f, -1.0f, -1.0f, // Bottom-right
    1.0f,  1.0f, -1.0f, // Top-right
    1.0f,  1.0f, -1.0f, // Top-right
    1.0f,  1.0f,  1.0f, // Top-left
    1.0f, -1.0f,  1.0f, // Bottom-left

    // Back face
    1.0f, -1.0f, -1.0f, // Bottom-left
    -1.0f, -1.0f, -1.0f, // Bottom-right
    -1.0f,  1.0f, -1.0f, // Top-right
    -1.0f,  1.0f, -1.0f, // Top-right
    1.0f,  1.0f, -1.0f, // Top-left
    1.0f, -1.0f, -1.0f, // Bottom-left

    // Left face
    -1.0f, -1.0f, -1.0f, // Bottom-left
    -1.0f, -1.0f,  1.0f, // Bottom-right
    -1.0f,  1.0f,  1.0f, // Top-right
    -1.0f,  1.0f,  1.0f, // Top-right
    -1.0f,  1.0f, -1.0f, // Top-left
    -1.0f, -1.0f, -1.0f, // Bottom-left

    // Top face
    -1.0f,  1.0f,  1.0f, // Bottom-left
    1.0f,  1.0f,  1.0f, // Bottom-right
    1.0f,  1.0f, -1.0f, // Top-right
    1.0f,  1.0f, -1.0f, // Top-right
    -1.0f,  1.0f, -1.0f, // Top-left
    -1.0f,  1.0f,  1.0f, // Bottom-left

    // Bottom face
    -1.0f, -1.0f, -1.0f, // Bottom-left
    1.0f, -1.0f, -1.0f, // Bottom-right
    1.0f, -1.0f,  1.0f, // Top-right
    1.0f, -1.0f,  1.0f, // Top-right
    -1.0f, -1.0f,  1.0f, // Top-left
    -1.0f, -1.0f, -1.0f, // Bottom-left
};

void MicrowaveOpenGLWidget::initializeGL()
{
    initializeOpenGLFunctions(); //OpenGL backend apparently

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    /////////////////////////SHADERS///////////////////////////
    std::string vertexShader = R"glsl(
    #version 330 core
    layout (location = 0) in vec3 aPos;
    uniform mat4 mvp;
    out vec3 vertexColor;
    out vec3 vertexPos;

    void main()
    {
        gl_Position = mvp * vec4(aPos, 1.0);
        vertexColor = vec3(1.0, 1.0, 1.0);
        vertexPos = aPos;
    }
    )glsl";

    std::string fragmentShader = R"glsl(
    #version 330 core
    out vec4 FragColor;
    in vec3 vertexColor;
    in vec3 vertexPos;
    uniform vec3 objectColor;
    uniform int isCylinder;

    void main()
    {
        vec4 colorToRender;
        if(isCylinder==1){
            float greyScale = (vertexPos.y + 1.0) / 2.0;
            colorToRender = vec4(0.2, greyScale*0.5, 0.2, 1.0);
        } else {
            colorToRender = vec4(objectColor, 1.0);
        }

        FragColor = colorToRender;
    }

    )glsl";
    ////////////////////////////////////////////////////////////


    shaderProgram = new QOpenGLShaderProgram();
    shaderProgram->addShaderFromSourceCode(QOpenGLShader::Vertex, vertexShader.c_str());
    shaderProgram->addShaderFromSourceCode(QOpenGLShader::Fragment, fragmentShader.c_str());
    shaderProgram->link();
    shaderProgram->bind();

    m_vertex.create();
    m_vertex.bind();
    m_vertex.setUsagePattern(QOpenGLBuffer::StaticDraw);
    m_vertex.allocate(vertices, sizeof(vertices));

    m_object.create();
    m_object.bind();
    shaderProgram->enableAttributeArray(0);
    shaderProgram->setAttributeBuffer(0, GL_FLOAT, 0, 3, 3 * sizeof(GLfloat));

    cylinderVertices = generateCylinderVertices(2, 100);
    cylinder_vertex.create();
    cylinder_vertex.bind();
    cylinder_vertex.setUsagePattern(QOpenGLBuffer::StaticDraw);
    cylinder_vertex.allocate(cylinderVertices.constData(), cylinderVertices.size() * sizeof(GLfloat));

    cylinder_object.create();
    cylinder_object.bind();
    shaderProgram->enableAttributeArray(0);
    shaderProgram->setAttributeBuffer(0, GL_FLOAT, 0, 3, 3 * sizeof(GLfloat));

    m_object.release();
    m_vertex.release();
    cylinder_object.release();
    cylinder_vertex.release();
    shaderProgram->release();
}

void MicrowaveOpenGLWidget::resizeGL(int w, int h)
{
    projection.setToIdentity();
    projection.perspective(45.0f, (float)w / h, 0.1f, 100.0f);
}

void MicrowaveOpenGLWidget::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Clear color and depth buffer
    glEnable(GL_DEPTH_TEST); // Enable depth testing

    shaderProgram->bind();

    //We draw the cube
    QMatrix4x4 mvpMatrix;
    mvpMatrix.setToIdentity();
    mvpMatrix.perspective(45.0f, GLfloat(width()) / height(), 0.1f, 100.0f);
    mvpMatrix.translate(0.0f, 0.0f, -5.0f);
    mvpMatrix.rotate(rotationAngle, 0.0f, 1.0f, 0.0f);

    shaderProgram->setUniformValue("mvp", mvpMatrix); // Pass the MVP matrix to the shader
    QVector3D colorVector(color.redF()*0.6, color.greenF()*0.6, color.blueF()*0.6);
    shaderProgram->setUniformValue("objectColor", colorVector);
    shaderProgram->setUniformValue("isCylinder", 0);

    m_object.bind();
    glDrawArrays(GL_TRIANGLES, 0, 36); // Draw the object
    m_object.release();

    //We draw the cylinder
    mvpMatrix.setToIdentity();
    mvpMatrix.perspective(45.0f, GLfloat(width()) / height(), 0.1f, 100.0f);
    mvpMatrix.translate(0.0f, -1.0f, -5.0f);
    mvpMatrix.rotate(90.f, 1.0f, 0.0f, 0.0f);
    mvpMatrix.rotate(-rotationAngle, 0.0f, 0.0f, 1.0f);

    shaderProgram->setUniformValue("mvp", mvpMatrix); // Pass the MVP matrix to the shader
    QVector3D cylinderColorVector(0.0f, 0.0f, 1.0f);
    shaderProgram->setUniformValue("objectColor", cylinderColorVector);
    shaderProgram->setUniformValue("isCylinder", 1);

    cylinder_object.bind();
    glDrawArrays(GL_TRIANGLE_FAN, 0, cylinderVertices.size() / 3);
    cylinder_object.release();

    shaderProgram->release();
}

void MicrowaveOpenGLWidget::setColor(const QColor &color)
{
    this->color = color;
    update();
}

void MicrowaveOpenGLWidget::startRotation()
{
    rotationTimer.start();
}

void MicrowaveOpenGLWidget::stopRotation()
{
    rotationTimer.stop();
}

void MicrowaveOpenGLWidget::updateRotation()
{
    rotationAngle += 1.0f;
    if (rotationAngle >= 360.0f)
        rotationAngle -= 360.0f;
    update();
}
