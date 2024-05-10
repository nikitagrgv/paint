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
        glBegin(GL_TRIANGLES);
        glVertex2i(mPosition.x() * mScaleFactorX, mPosition.y() * mScaleFactorY);
        glVertex2i(mPosition.x() * mScaleFactorX + 100, mPosition.y() * mScaleFactorY + 100);
        glVertex2i(mPosition.x() * mScaleFactorX, mPosition.y() * mScaleFactorY + 100);
        glEnd();
    }

    void mousePressEvent(QMouseEvent *apEvent) override
    {
        mPosition = apEvent->pos();
    }

private:
    float mScaleFactorX;
    float mScaleFactorY;

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