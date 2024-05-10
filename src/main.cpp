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


struct glView : QOpenGLWidget
{
    glView()
    {
        connect(&mpTimer, &QTimer::timeout, this, QOverload<>::of(&glView::repaint));
        mpTimer.start(33);
    }

    void initializeGL() override
    {
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(0, 800, 600, 0, 0, 1);

        init();
    }

    void resizeGL(int w, int h) override
    {
        glViewport(0, 0, w, h);

        mScaleFactorX = 800 / (float)w;
        mScaleFactorY = 600 / (float)h;
    }

    void paintGL() override
    {
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glColor4f(1.0f, 0.2f, 0.2f, 1.0f);
        // glBegin(GL_TRIANGLES);
        // glVertex2i(mPosition.x() * mScaleFactorX, mPosition.y() * mScaleFactorY);
        // glVertex2i(mPosition.x() * mScaleFactorX + 100, mPosition.y() * mScaleFactorY + 100);
        // glVertex2i(mPosition.x() * mScaleFactorX, mPosition.y() * mScaleFactorY + 100);
        // glEnd();

        renderSceneGL2();

    }

    void mousePressEvent(QMouseEvent *apEvent) override
    {
        mPosition = apEvent->pos();
    }


    void init()
    {
        loadTexture2("C:\\Users\\nekita\\CLionProjects\\paint\\data\\spam.png", backgroundimage);
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

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tex.width(), tex.height(), 0, GL_RGBA,
            GL_UNSIGNED_BYTE, tex.bits());

        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);

        glDisable(GL_TEXTURE_2D);

        return tex;
    }

    void renderSceneGL2()
    {
        glClearColor(0.4f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, backgroundimage);

        const auto s = 100;

        // glBegin(GL_QUADS);
        // glTexCoord2f(0, 0);
        // glVertex3f(-s, -s, -s);
        // glTexCoord2f(1, 0);
        // glVertex3f(s, -s, -s);
        // glTexCoord2f(1, 1);
        // glVertex3f(s, s, -s);
        // glTexCoord2f(0, 1);
        // glVertex3f(-s, s, -s);
        // glEnd();


        glBegin(GL_TRIANGLES);
        glTexCoord2f(0, 0);
        glVertex2i(mPosition.x() * mScaleFactorX, mPosition.y() * mScaleFactorY);
        glTexCoord2f(1, 0);
        glVertex2i(mPosition.x() * mScaleFactorX + 100, mPosition.y() * mScaleFactorY + 100);
        glTexCoord2f(1, 1);
        glVertex2i(mPosition.x() * mScaleFactorX, mPosition.y() * mScaleFactorY + 100);
        glEnd();


        glDisable(GL_TEXTURE_2D);
    }

private:
    float mScaleFactorX;
    float mScaleFactorY;

    GLuint backgroundimage;

    QPoint mPosition;
    QTimer mpTimer;
};

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    glView win;
    win.show();
    // win.showFullScreen();

    return a.exec();
}

#include "main.moc"