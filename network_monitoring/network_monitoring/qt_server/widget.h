#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QUdpSocket>

#include "capture_thread.h"

QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

private slots:
    void on_bind_clicked();
    void on_unbind_clicked();
    void on_send_clicked();
    void on_broadcast_clicked();

private:
    Ui::Widget *ui;
    QUdpSocket *udpSocket;

    /* 摄像头线程 */
    CaptureThread *captureThread;

private slots:
    void readPendingDatagrams();
    void mStateChaged(QAbstractSocket::SocketState socketState);

    /* 显示图像 */
    void showImage(QImage);

    void on_act_clicked(bool checked);
    void on_quit_clicked();
};
#endif // WIDGET_H
