#include <QApplication>
#include <QPushButton>
#include <QPen>
#include <QPainter>
#include <QEvent>
#include <QPaintEvent>
#include <QTimer>
#include <QOpenGLWindow>
#include <QLabel>
#include <QGridLayout>
#include <QtOpenGLWidgets/QOpenGLWidget>
#include <QMainWindow>

#include <QMouseEvent>
#include <QTimer>
#include <iostream>


struct glView : QOpenGLWidget
{
    glView(QWidget *parent)
        : QOpenGLWidget(parent)
    {
        connect(&mpTimer, &QTimer::timeout, this, QOverload<>::of(&glView::repaint));
        mpTimer.start(33);
    }

    void initializeGL() override
    {
        glMatrixMode(GL_PROJECTION);

        init();
    }

    void resizeGL(int w, int h) override
    {
        size_.setWidth(w);
        size_.setHeight(h);
        glLoadIdentity();
        glOrtho(0, w, h, 0, 0, 1);
        glViewport(0, 0, w, h);
    }

    void paintGL() override
    {
        glClearColor(0.4f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, backgroundimage);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        QPointF point = (mPosition);
        float a = point.x();
        float b = point.y();
        float w = image_size.width();
        float h = image_size.height();

        glBegin(GL_QUADS);
        glTexCoord2f(0, 0);
        glVertex2f(a, b);

        glTexCoord2f(1, 0);
        glVertex2f(a + w, b);

        glTexCoord2f(1, 1);
        glVertex2f(a + w, b + h);

        glTexCoord2f(0, 1);
        glVertex2f(a, b + h);

        glEnd();

        glDisable(GL_TEXTURE_2D);
    }

    void mousePressEvent(QMouseEvent *apEvent) override
    {
        if (apEvent->button() == Qt::LeftButton)
        {
            mPosition = apEvent->pos();
        }
        else if (apEvent->button() == Qt::RightButton)
        {
            glBindTexture(GL_TEXTURE_2D, backgroundimage);
            QImage im(image_path);
            QImage tex = im.convertToFormat(QImage::Format_RGBA8888);
            image_size.setWidth(tex.width());
            image_size.setHeight(tex.height());
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tex.width(), tex.height(), 0, GL_RGBA,
                GL_UNSIGNED_BYTE, tex.bits());
            // mPosition = apEvent->pos();
        }
    }


    void init()
    {
        loadTexture2(image_path, backgroundimage);
    }

    QImage loadTexture2(const char *filename, GLuint &textureID)
    {
        glEnable(GL_TEXTURE_2D); // Enable texturing

        glGenTextures(1, &textureID);            // Obtain an id for the texture
        glBindTexture(GL_TEXTURE_2D, textureID); // Set as the current texture

        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);

        QImage im(filename);
        // QImage tex = QOpenGLWidget::convertToGLFormat(im);
        QImage tex = im.convertToFormat(QImage::Format_RGBA8888);
        image_size.setWidth(tex.width());
        image_size.setHeight(tex.height());

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tex.width(), tex.height(), 0, GL_RGBA,
            GL_UNSIGNED_BYTE, tex.bits());

        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);

        glDisable(GL_TEXTURE_2D);

        return tex;
    }

private:
    QSize size_{};
    QSize image_size{};

    GLuint backgroundimage{};

    QPoint mPosition{};
    QTimer mpTimer{};
    const char *image_path = "C:\\Users\\nekita\\CLionProjects\\paint\\data\\spam.png";
};

class MainWindow : public QMainWindow
{
public:
    MainWindow()
    {
        view_ = new glView(this);
        setCentralWidget(view_);
    }

private:
    glView *view_;
};

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    MainWindow win;
    win.resize(640, 480);
    win.show();
    // win.sho\wFullScreen();

    return a.exec();
}

#include "main.moc"