#include "widget.h"
#include "ui_widget.h"

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
    udpSocket = new QUdpSocket(this);
    udpSocket_video = new QUdpSocket(this);

    ui->bind->setEnabled(true);
    ui->unbind->setEnabled(false);

    //接受消息
    connect(udpSocket, SIGNAL(readyRead()), this, SLOT(readPendingDatagrams()));
    connect(udpSocket, SIGNAL(stateChanged(QAbstractSocket::SocketState)), this, SLOT(mStateChaged(QAbstractSocket::SocketState)));

    connect(ui->checkBox, SIGNAL(clicked(bool)), this, SLOT(play_video(bool)));

    connect(udpSocket_video, SIGNAL(readyRead()), this,SLOT(videoUpdate()));
}

Widget::~Widget()
{
    delete ui;
}

void Widget::videoUpdate()
{
    QByteArray datagram;

    /* 数据大小重置 */
    datagram.resize(udpSocket_video->pendingDatagramSize());

    /* 数据存放到datagram中 */
    udpSocket_video->readDatagram(datagram.data(), datagram.size());

    QByteArray decryptedByte;
    decryptedByte = QByteArray::fromBase64(datagram.data());

    QImage image;
    image.loadFromData(decryptedByte);

    /* 显示图像 */
    ui->label->setPixmap(QPixmap::fromImage(image));
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
    udpSocket->bind(7755);
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
    //开发板地址
    udpSocket->writeDatagram(ui->lineEdit->text().toUtf8(), QHostAddress("192.168.1.103"), 7766);

}

void Widget::on_broadcast_clicked()
{
    udpSocket->writeDatagram(ui->lineEdit->text().toUtf8(), QHostAddress::Broadcast, 7766);
}

void Widget::mStateChaged(QAbstractSocket::SocketState socketState)
{
    switch (socketState) {
    case QAbstractSocket::BoundState:
        ui->textBrowser->append("socket状态BoundState");
        break;
    default:
        break;
    }
}

void Widget::play_video(bool state)
{
    if (state) {
        udpSocket_video->bind(7756);
    } else {
        udpSocket_video->close();
    }
}
