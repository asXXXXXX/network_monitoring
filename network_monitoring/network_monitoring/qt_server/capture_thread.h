#ifndef CAPTURE_TREAD_H
#define CAPTURE_TREAD_H

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#ifdef linux
#include <linux/fb.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/videodev2.h>
#include <linux/input.h>
#endif

#include <QThread>
#include <QDebug>
#include <QPushButton>
#include <QImage>
#include <QByteArray>
#include <QBuffer>
#include <QTime>
#include <QUdpSocket>

#define VIDEO_DEV			"/dev/video1"
#define VIDEO_BUFFER_COUNT	4

struct buffer_info {
    void *start;
    unsigned int length;
};

class CaptureThread : public QThread
{
    Q_OBJECT

signals:
    /* 图片准备完成 */
    void imageReady(QImage);

private:
    /* udp套接字 */
    QUdpSocket *udpSocket;

    /* 线程开启flag */
    bool startFlag = false;

    /* 开启广播flag */
    bool startBroadcast = false;

    /* 本地显示flag  */
    bool startLocalDisplay = false;
    void run() override;

public:
    CaptureThread(QObject *parent = nullptr) {
        Q_UNUSED(parent);
    }

    /* 添加析构函数声明 */
    ~CaptureThread();

public slots:
    /* 设置线程 */
    void setThreadStart(bool start) {           //决定线程是否运行
        startFlag = start;
        if (start) {
            if (!this->isRunning())             //判断线程是否正在运行
                this->start();
        } else {
            this->quit();
        }
    }

    /* 设置广播 */
    void setBroadcast(bool start) {
        startBroadcast = start;
    }

    /* 设置本地显示 */
    void setLocalDisplay(bool start) {
        startLocalDisplay = start;
    }
};

#endif // CAPTURE_TREAD_H
