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
#include <QSlider>
#include <QDoubleSpinBox>
#include <QFormLayout>

#include <QMouseEvent>
#include <QTimer>
#include <iostream>


class glView : public QOpenGLWidget
{
    Q_OBJECT

public:
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

        QPointF point = (base_point_);
        float a = point.x();
        float b = point.y();
        float w = image_size.width() * image_scale_;
        float h = image_size.height() * image_scale_;

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
        if (apEvent->button() == Qt::RightButton)
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
        else if (apEvent->button() == Qt::MiddleButton)
        {
            last_middle_mouse_pos_ = apEvent->pos();
        }
    }

    void mouseMoveEvent(QMouseEvent *event) override
    {
        std::cout << " buttons = " << event->buttons() << std::endl;
        std::cout << " fff = " << (event->buttons() & Qt::MiddleButton) << std::endl;
        if ((event->buttons() & Qt::MiddleButton) == 0)
        {
            return;
        }
        const QPointF delta = event->pos() - last_middle_mouse_pos_;
        last_middle_mouse_pos_ = event->pos();
        base_point_ = base_point_ + delta;
    }

    void wheelEvent(QWheelEvent *event) override
    {
        constexpr float MULTIPLIER = 1.1f;
        constexpr float DEMULTIPLIER = 1.0f / MULTIPLIER;

        const float multiplier = event->angleDelta().y() > 0
            ? MULTIPLIER
            : DEMULTIPLIER;

        image_scale_ *= multiplier;

        const QPointF mpos = event->position();
        const QPointF a = mpos - base_point_;
        const QPointF b = a * multiplier;
        const QPointF delta = a - b;
        base_point_ = base_point_ + delta;

        emit scaleChanged(image_scale_);
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

    void setScale(float scale)
    {
        image_scale_ = scale;
    }

signals:
    void scaleChanged(float scale);

private:
    QSize size_{};
    QSize image_size{};

    QPointF last_middle_mouse_pos_;

    float image_scale_{1};

    GLuint backgroundimage{};

    QPointF base_point_{};
    QTimer mpTimer{};
    const char *image_path = "C:\\Users\\nekita\\CLionProjects\\paint\\data\\spam.png";
};

class MainWindow : public QMainWindow
{
public:
    MainWindow()
    {
        QWidget *central_widget = new QWidget(this);
        auto layout = new QVBoxLayout(central_widget);
        layout->setContentsMargins(0, 0, 0, 0); {
            auto form_widget = new QWidget(this);
            auto form_layout = new QFormLayout(form_widget);
            layout->addWidget(form_widget); {
                auto scale_widgets = new QWidget(this);
                auto scale_layout = new QHBoxLayout(scale_widgets);

                scale_spinbox_ = new QDoubleSpinBox(this);
                scale_layout->addWidget(scale_spinbox_);
                scale_spinbox_->setMinimum(0.1);
                scale_spinbox_->setMaximum(10.0);
                scale_spinbox_->setValue(1.0);

                scale_slider_ = new QSlider(Qt::Horizontal, this);
                scale_layout->addWidget(scale_slider_);
                scale_slider_->setMinimum(1);
                scale_slider_->setMaximum(100);
                scale_slider_->setValue(10);

                form_layout->addRow(new QLabel("Scale:"), scale_widgets);
            }
        }

        view_ = new glView(this);
        layout->addWidget(view_);
        layout->setStretch(0, 0);
        layout->setStretch(1, 1);


        setCentralWidget(central_widget);

        ////////////
        connect(scale_spinbox_, &QDoubleSpinBox::valueChanged,
            [this](double value) {
                if (block_scale_change_)
                {
                    return;
                }
                block_scale_change_ = true;
                scale_slider_->setValue(value * 10);
                view_->setScale(value);
                block_scale_change_ = false;
            });

        connect(scale_slider_, &QSlider::valueChanged, [this](int value) {
            if (block_scale_change_)
            {
                return;
            }
            block_scale_change_ = true;
            scale_spinbox_->setValue(value / 10.0);
            view_->setScale(value / 10.0);
            block_scale_change_ = false;
        });

        connect(view_, &glView::scaleChanged, [this](float scale) {
            if (block_scale_change_)
            {
                return;
            }
            block_scale_change_ = true;
            scale_spinbox_->setValue(scale);
            scale_slider_->setValue(scale * 10);
            block_scale_change_ = false;
        });
    }

private:
    bool block_scale_change_{false};
    QDoubleSpinBox *scale_spinbox_;
    QSlider *scale_slider_;
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