#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QUdpSocket>

QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();
    QUdpSocket *udpSocket;
    QUdpSocket *udpSocket_video;

private slots:
    void on_bind_clicked();

    void on_unbind_clicked();

    void on_send_clicked();

    void on_broadcast_clicked();

    void mStateChaged(QAbstractSocket::SocketState);

    void readPendingDatagrams();

    void videoUpdate();

    void play_video(bool);

private:
    Ui::Widget *ui;
};
#endif // WIDGET_H
