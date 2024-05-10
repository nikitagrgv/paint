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
    glView();

    void initializeGL() override;

    void resizeGL(int w, int h) override;

    void paintGL() override;

    void mousePressEvent(QMouseEvent *) override;

private:
    float mScaleFactorX;
    float mScaleFactorY;

    QPoint mPosition;
    QTimer mpTimer;
};

//------------------------------------------------------------------------------
glView::glView()
{
    connect(&mpTimer, SIGNAL(timeout()), this, SLOT(repaint()));
    mpTimer.start(33);
}

//------------------------------------------------------------------------------
void glView::initializeGL()
{
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, 800, 600, 0, 0, 1);
}

//------------------------------------------------------------------------------
void glView::resizeGL(int w, int h)
{
    glViewport(0, 0, w, h);

    mScaleFactorX = 800 / (float)w;
    mScaleFactorY = 600 / (float)h;
}

//------------------------------------------------------------------------------
void glView::paintGL()
{
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glColor4f(1.0f, 0.2f, 0.2f, 1.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2i(mPosition.x() * mScaleFactorX, mPosition.y() * mScaleFactorY);
    glVertex2i(mPosition.x() * mScaleFactorX + 100, mPosition.y() * mScaleFactorY + 100);
    glVertex2i(mPosition.x() * mScaleFactorX, mPosition.y() * mScaleFactorY + 100);
    glEnd();
}

//------------------------------------------------------------------------------
void glView::mousePressEvent(QMouseEvent *apEvent)
{
    mPosition = apEvent->pos();
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);

    ~MainWindow();

private:
    glView mView;
};

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    mView.show();
}

MainWindow::~MainWindow()
{
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    glView win;
    win.show();
    //win.showFullScreen();

    //MainWindow w;
    //w.show();

    return a.exec();
}

#include "main.moc"