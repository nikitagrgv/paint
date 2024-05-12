#include <QApplication>
#include <QDoubleSpinBox>
#include <QEvent>
#include <QFormLayout>
#include <QGridLayout>
#include <QLabel>
#include <QMainWindow>
#include <QMenuBar>
#include <QMouseEvent>
#include <QOpenGLWindow>
#include <QPaintEvent>
#include <QPainter>
#include <QPen>
#include <QPointer>
#include <QPushButton>
#include <QSlider>
#include <QTimer>
#include <QUndoStack>

#include <QtOpenGLWidgets/QOpenGLWidget>
#include <iostream>

struct Globals
{
    QUndoStack *undo_stack = nullptr;
} globals;

class glView : public QOpenGLWidget
{
    Q_OBJECT

public:
    glView(QWidget *parent)
        : QOpenGLWidget(parent)
    {
        image_ = QImage(200, 200, QImage::Format_RGBA8888);
        image_.fill(Qt::white);

        connect(&mpTimer, &QTimer::timeout, this, QOverload<>::of(&glView::repaint));
        mpTimer.start(33);
    }

    void initializeGL() override
    {
        gl_intialized_ = true;
        glMatrixMode(GL_PROJECTION);
        create_gl_image();
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
        glBindTexture(GL_TEXTURE_2D, image_gl_);

        QPointF point = (base_point_);
        float a = point.x();
        float b = point.y();
        float w = image_size_.width() * image_scale_;
        float h = image_size_.height() * image_scale_;

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
        if (apEvent->button() == Qt::MiddleButton)
        {
            last_middle_mouse_pos_ = apEvent->pos();
        }
        else if (apEvent->button() == Qt::LeftButton)
        {
            prev_image_ = image_;
            last_line_pos_ = toImagePos(apEvent->pos());
            draw_point(last_line_pos_, QColor(255, 255, 255, 255));
        }
    }

    void mouseReleaseEvent(QMouseEvent *event) override
    {
        class PaintCommand : public QUndoCommand
        {
        public:
            PaintCommand(glView *view, QImage new_image, QImage prev_image)
                : view_(view)
                , new_image_(std::move(new_image))
                , prev_image_(std::move(prev_image))
            {}

            void undo() override { view_->setImage(prev_image_); }

            void redo() override { view_->setImage(new_image_); }

        private:
            QPointer<glView> view_;
            QImage new_image_;
            QImage prev_image_;
        };

        if (event->button() == Qt::LeftButton)
        {
            globals.undo_stack->push(new PaintCommand(this, image_, std::move(prev_image_)));
        }
    }

    void setImage(QImage image)
    {
        image_ = std::move(image);
        if (image_.format() != QImage::Format_RGBA8888)
        {
            image_ = image_.convertToFormat(QImage::Format_RGBA8888);
        }
        if (gl_intialized_)
        {
            load_image_to_gl();
            repaint();
        }
    }

    void mouseMoveEvent(QMouseEvent *event) override
    {
        if ((event->buttons() & Qt::MiddleButton))
        {
            const QPointF delta = event->pos() - last_middle_mouse_pos_;
            last_middle_mouse_pos_ = event->pos();
            base_point_ = base_point_ + delta;
        }
        else if (event->buttons() & Qt::LeftButton)
        {
            const QPoint image_pos = toImagePos(event->pos());
            draw_line(image_pos, QColor(255, 255, 255, 255));
        }
    }

    void load_image_to_gl()
    {
        image_size_.setWidth(image_.width());
        image_size_.setHeight(image_.height());
        glBindTexture(GL_TEXTURE_2D, image_gl_);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image_.width(), image_.height(), 0, GL_RGBA,
            GL_UNSIGNED_BYTE, image_.bits());
    }

    void draw_point(const QPoint &image_pos, const QColor &color)
    {
        image_.setPixel(image_pos, color.rgba());
        load_image_to_gl();
    }

    void draw_line(const QPoint &image_pos, const QColor &color)
    {
        int x0 = last_line_pos_.x();
        int x1 = image_pos.x();
        int y0 = last_line_pos_.y();
        int y1 = image_pos.y();
        const bool x_major = abs(x1 - x0) > abs(y1 - y0);
        if ((x_major && x0 > x1) || (!x_major && y0 > y1))
        {
            std::swap(x0, x1);
            std::swap(y0, y1);
        }

        const double x0f = x0;
        const double x1f = x1;
        const double y0f = y0;
        const double y1f = y1;

        const auto get_k = [](double x0, double x1, double y0, double y1) {
            return (y1 - y0) / (x1 - x0);
        };
        const auto get_b = [](double x0, double x1, double y0, double y1) {
            return (x0 * y1 - x1 * y0) / (x0 - x1);
        };

        if (x_major)
        {
            const double k = get_k(x0f, x1f, y0f, y1f);
            const double b = get_b(x0f, x1f, y0f, y1f);
            for (int x = x0; x <= x1; x++)
            {
                const double y = k * x + b;
                draw_point(QPoint(x, round(y)), color);
            }
        }
        else
        {
            const double k = get_k(y0f, y1f, x0f, x1f);
            const double b = get_b(y0f, y1f, x0f, x1f);
            for (int y = y0; y <= y1; y++)
            {
                const double x = k * y + b;
                draw_point(QPoint(round(x), y), color);
            }
        }

        last_line_pos_ = image_pos;
    }

    void zoom_to_position(int dir, const QPointF &position)
    {
        constexpr float MULTIPLIER = 1.1f;
        constexpr float DEMULTIPLIER = 1.0f / MULTIPLIER;

        const float multiplier = dir > 0 ? MULTIPLIER : DEMULTIPLIER;

        image_scale_ *= multiplier;

        const QPointF a = position - base_point_;
        const QPointF b = a * multiplier;
        const QPointF delta = a - b;
        base_point_ = base_point_ + delta;

        emit scaleChanged(image_scale_);
    }

    void wheelEvent(QWheelEvent *event) override
    {
        if (event->modifiers() == Qt::ControlModifier)
        {
            zoom_to_position(event->angleDelta().y(), event->position());
        }


        const bool scroll_y = event->modifiers() == Qt::NoModifier;
        const bool scroll_x = event->modifiers() == Qt::ShiftModifier;
        if (scroll_x || scroll_y)
        {
            const double delta = event->angleDelta().y() * 0.3;
            if (scroll_x)
            {
                base_point_.setX(base_point_.x() - delta);
            }
            else
            {
                base_point_.setY(base_point_.y() + delta);
            }
        }
    }

    QImage create_gl_image()
    {
        glEnable(GL_TEXTURE_2D); // Enable texturing

        glGenTextures(1, &image_gl_);            // Obtain an id for the texture
        glBindTexture(GL_TEXTURE_2D, image_gl_); // Set as the current texture

        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);

        image_size_.setWidth(image_.width());
        image_size_.setHeight(image_.height());

        load_image_to_gl();

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

        glDisable(GL_TEXTURE_2D);

        return image_;
    }

    void setScale(float scale) { image_scale_ = scale; }

    QPoint toImagePos(const QPointF screen_pos) const
    {
        QPointF point_f = (screen_pos - base_point_) / image_scale_;
        return QPoint((point_f.x()), (point_f.y()));
    }

signals:
    void scaleChanged(float scale);

private:
    QSize size_{};
    QSize image_size_{};

    QPointF last_middle_mouse_pos_;

    float image_scale_{1};

    GLuint image_gl_{};

    QPoint last_line_pos_{};

    QPointF base_point_{};
    QTimer mpTimer{};
    QImage image_;

    QImage prev_image_;

    bool gl_intialized_{false};
};

class MainWindow : public QMainWindow
{
public:
    MainWindow()
    {
        globals.undo_stack = new QUndoStack();
        qApp->installEventFilter(this);

        QWidget *central_widget = new QWidget(this);
        auto layout = new QVBoxLayout(central_widget);
        layout->setContentsMargins(0, 0, 0, 0);
        {
            auto form_widget = new QWidget(this);
            auto form_layout = new QFormLayout(form_widget);
            layout->addWidget(form_widget);
            {
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

        init_menu();

        ////////////
        connect(scale_spinbox_, &QDoubleSpinBox::valueChanged, [this](double value) {
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

    void init()
    {
        const char *image_path = "C:\\Users\\nekita\\CLionProjects\\paint\\data\\spam.png";
        setImage(QImage(image_path));
    }

    void setImage(QImage image) { view_->setImage(std::move(image)); }

    ~MainWindow() override { delete globals.undo_stack; }

private:
    bool eventFilter(QObject *watched, QEvent *event) override
    {
        if (event->type() == QEvent::KeyRelease)
        {
            QKeyEvent *key_event = static_cast<QKeyEvent *>(event);
            if (key_event->modifiers() == Qt::ControlModifier)
            {
                if (key_event->key() == Qt::Key_Z)
                {
                    globals.undo_stack->undo();
                    return true;
                }
                if (key_event->key() == Qt::Key_Y)
                {
                    globals.undo_stack->redo();
                    return true;
                }
            }
        }
        return QMainWindow::eventFilter(watched, event);
    }

    void init_menu()
    {
        // file
        {
            QMenu *file_menu = menuBar()->addMenu(tr("&File"));

            auto *new_action = new QAction("New", nullptr);
            file_menu->addAction(new_action);

            auto *open_action = new QAction("Open", nullptr);
            file_menu->addAction(open_action);

            auto save_action = new QAction("Save", nullptr);
            file_menu->addAction(save_action);
        }
        // edit
        {
            QMenu *edit_menu = menuBar()->addMenu(tr("&Edit"));

            auto undo_action = globals.undo_stack->createUndoAction(this, "Undo");
            edit_menu->addAction(undo_action);

            auto redo_action = globals.undo_stack->createRedoAction(this, "Redo");
            edit_menu->addAction(redo_action);
        }
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
    win.resize(1024, 628);
    win.show();

    win.init();
    // win.sho\wFullScreen();

    return a.exec();
}

#include "main.moc"