#include "widget.h"
#include "ui_widget.h"

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
    /* udp套接字 */
    udpSocket = new QUdpSocket(this);

    ui->bind->setEnabled(true);
    ui->unbind->setEnabled(false);

    captureThread = new CaptureThread(this);

    //接受消息
    connect(udpSocket, SIGNAL(readyRead()), this, SLOT(readPendingDatagrams()));
    connect(udpSocket, SIGNAL(stateChanged(QAbstractSocket::SocketState)), this, SLOT(mStateChaged(QAbstractSocket::SocketState)));

    connect(ui->act, SIGNAL(clicked(bool)), captureThread, SLOT(setThreadStart(bool)));
    connect(captureThread, SIGNAL(imageReady(QImage)), this, SLOT(showImage(QImage)));
    connect(ui->checkBox, SIGNAL(clicked(bool)), captureThread, SLOT(setLocalDisplay(bool)));
    connect(ui->checkBox_2, SIGNAL(clicked(bool)), captureThread, SLOT(setBroadcast(bool)));
}

Widget::~Widget()
{
    delete ui;
}

void Widget::readPendingDatagrams()
{
    QByteArray tmpByteArray;
    tmpByteArray.resize(udpSocket->pendingDatagramSize());
    QHostAddress ipaddr;
    quint16 port;
    while(udpSocket->hasPendingDatagrams()) {
        udpSocket->readDatagram(tmpByteArray.data(), tmpByteArray.size(), &ipaddr, &port);
        //打印消息
        ui->textBrowser->append("接收来自:" + ipaddr.toString()
                                + ":" + QString::number(port) + ":" + tmpByteArray);
    }
}

void Widget::on_bind_clicked()
{
    ui->bind->setEnabled(false);
    ui->unbind->setEnabled(true);
    udpSocket->bind(7766);
}

void Widget::on_unbind_clicked()
{
    udpSocket->close();
    ui->bind->setEnabled(true);
    ui->unbind->setEnabled(false);
}

void Widget::on_send_clicked()
{
    ui->textBrowser->append("发送："+ui->lineEdit->text());
    //自发自收
    udpSocket->writeDatagram(ui->lineEdit->text().toUtf8(), QHostAddress("192.168.1.102"), 7755);
}

void Widget::on_broadcast_clicked()
{
    udpSocket->writeDatagram(ui->lineEdit->text().toUtf8(), QHostAddress::Broadcast, 7755);
}

void Widget::mStateChaged(QAbstractSocket::SocketState socketState)
{
    switch (socketState) {
    case QAbstractSocket::BoundState:
        ui->textBrowser->append("socket状态BoundState");
        break;
    case QAbstractSocket::ClosingState:
        ui->textBrowser->append("socket状态ClosingState");
        break;
    default:
        break;
    }
}

void Widget::showImage(QImage image)
{
    ui->label->setPixmap(QPixmap::fromImage(image));
}

void Widget::on_act_clicked(bool state)
{
    if (state) {
        ui->act->setText("停止采集");
    } else {
        ui->act->setText(("开始采集"));
    }
}

void Widget::on_quit_clicked()
{
    // 停止捕获线程
    if (captureThread) {
        captureThread->setThreadStart(false);
        captureThread->wait();  // 等待线程退出
        delete captureThread;
        captureThread = nullptr;
    }

    // 关闭UDP套接字
    if (udpSocket) {
        udpSocket->close();
        delete udpSocket;
        udpSocket = nullptr;
    }

    // 关闭主窗口
    close();
}
