#include "capture_thread.h"


unsigned char src_buffer[640*480*2];        //用来装v4l2出队的yuyv数据
unsigned char rgb_buffer[640*480*3];        //用来装yuyv转换为rgb的数据

void yuyv_to_rgb(unsigned char* yuv,unsigned char* rgb)
{
    unsigned int i;
    unsigned char* y0 = yuv + 0;
    unsigned char* u0 = yuv + 1;
    unsigned char* y1 = yuv + 2;
    unsigned char* v0 = yuv + 3;

    unsigned  char* r0 = rgb + 0;
    unsigned  char* g0 = rgb + 1;
    unsigned  char* b0 = rgb + 2;
    unsigned  char* r1 = rgb + 3;
    unsigned  char* g1 = rgb + 4;
    unsigned  char* b1 = rgb + 5;

    float rt0 = 0, gt0 = 0, bt0 = 0, rt1 = 0, gt1 = 0, bt1 = 0;

    for(i = 0; i <= (640 * 480) / 2 ;i++)
    {
        bt0 = 1.164 * (*y0 - 16) + 2.018 * (*u0 - 128);
        gt0 = 1.164 * (*y0 - 16) - 0.813 * (*v0 - 128) - 0.394 * (*u0 - 128);
        rt0 = 1.164 * (*y0 - 16) + 1.596 * (*v0 - 128);

        bt1 = 1.164 * (*y1 - 16) + 2.018 * (*u0 - 128);
        gt1 = 1.164 * (*y1 - 16) - 0.813 * (*v0 - 128) - 0.394 * (*u0 - 128);
        rt1 = 1.164 * (*y1 - 16) + 1.596 * (*v0 - 128);

        if(rt0 > 250)      rt0 = 255;
        if(rt0< 0)        rt0 = 0;

        if(gt0 > 250)     gt0 = 255;
        if(gt0 < 0)    gt0 = 0;

        if(bt0 > 250)    bt0 = 255;
        if(bt0 < 0)    bt0 = 0;

        if(rt1 > 250)    rt1 = 255;
        if(rt1 < 0)    rt1 = 0;

        if(gt1 > 250)    gt1 = 255;
        if(gt1 < 0)    gt1 = 0;

        if(bt1 > 250)    bt1 = 255;
        if(bt1 < 0)    bt1 = 0;

        *r0 = (unsigned char)rt0;
        *g0 = (unsigned char)gt0;
        *b0 = (unsigned char)bt0;

        *r1 = (unsigned char)rt1;
        *g1 = (unsigned char)gt1;
        *b1 = (unsigned char)bt1;

        yuv = yuv + 4;
        rgb = rgb + 6;
        if(yuv == NULL)
          break;

        y0 = yuv;
        u0 = yuv + 1;
        y1 = yuv + 2;
        v0 = yuv + 3;

        r0 = rgb + 0;
        g0 = rgb + 1;
        b0 = rgb + 2;
        r1 = rgb + 3;
        g1 = rgb + 4;
        b1 = rgb + 5;
    }
}

CaptureThread::~CaptureThread() {
    startFlag = false;
    wait();  // 等待线程结束
}

void CaptureThread::run()
{
    /* 下面的代码请参考正点原子C应用编程V4L2章节，摄像头编程，这里不作解释 */
#ifdef linux
#ifndef __arm__
    return;
#endif
    int video_fd = -1;
    struct v4l2_format fmt;
    struct v4l2_requestbuffers req_bufs;
    static struct v4l2_buffer buf;
    int n_buf;
    struct buffer_info bufs_info[VIDEO_BUFFER_COUNT];
    enum v4l2_buf_type type;

    video_fd = open(VIDEO_DEV, O_RDWR);
    if (0 > video_fd) {
        printf("ERROR: failed to open video device %s\n", VIDEO_DEV);
        return ;
    }

    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmt.fmt.pix.width = 640;
    fmt.fmt.pix.height = 480;
    fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;  //V4L2_PIX_FMT_RGB565
    fmt.fmt.pix.field = V4L2_FIELD_INTERLACED;

    if (0 > ioctl(video_fd, VIDIOC_S_FMT, &fmt)) {
        printf("ERROR: failed to VIDIOC_S_FMT\n");
        close(video_fd);
        return ;
    }

    if (0 > ioctl(video_fd, VIDIOC_G_FMT, &fmt)) {  //
        printf("ERROR: failed to VIDIOC_G_FMT\n");
        close(video_fd);
        return ;
    }

    printf("fmt.type:\t\t%d\n",fmt.type);           //
    printf("pix.pixelformat:\t%c%c%c%c\n", \
            fmt.fmt.pix.pixelformat & 0xFF,\
            (fmt.fmt.pix.pixelformat >> 8) & 0xFF, \
            (fmt.fmt.pix.pixelformat >> 16) & 0xFF,\
            (fmt.fmt.pix.pixelformat >> 24) & 0xFF);
    printf("pix.width:\t\t%d\n",fmt.fmt.pix.width);
    printf("pix.height:\t\t%d\n",fmt.fmt.pix.height);
    printf("pix.field:\t\t%d\n",fmt.fmt.pix.field);

    req_bufs.count = VIDEO_BUFFER_COUNT;
    req_bufs.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req_bufs.memory = V4L2_MEMORY_MMAP;

    if (0 > ioctl(video_fd, VIDIOC_REQBUFS, &req_bufs)) {
        printf("ERROR: failed to VIDIOC_REQBUFS\n");
        return ;
    }

    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;
    for (n_buf = 0; n_buf < VIDEO_BUFFER_COUNT; n_buf++) {

        buf.index = n_buf;
        if (0 > ioctl(video_fd, VIDIOC_QUERYBUF, &buf)) {
            printf("ERROR: failed to VIDIOC_QUERYBUF\n");
            return ;
        }

        bufs_info[n_buf].length = buf.length;
        bufs_info[n_buf].start = mmap(NULL, buf.length,
                                      PROT_READ | PROT_WRITE, MAP_SHARED,
                                      video_fd, buf.m.offset);
        if (MAP_FAILED == bufs_info[n_buf].start) {
            printf("ERROR: failed to mmap video buffer, size 0x%x\n", buf.length);
            return ;
        }
    }

    for (n_buf = 0; n_buf < VIDEO_BUFFER_COUNT; n_buf++) {

        buf.index = n_buf;
        if (0 > ioctl(video_fd, VIDIOC_QBUF, &buf)) {
            printf("ERROR: failed to VIDIOC_QBUF\n");
            return ;
        }
    }

    type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (0 > ioctl(video_fd, VIDIOC_STREAMON, &type)) {
        printf("ERROR: failed to VIDIOC_STREAMON\n");
        return ;
    }

    while (startFlag) {

        for (n_buf = 0; n_buf < VIDEO_BUFFER_COUNT; n_buf++) {

            buf.index = n_buf;

            if (0 > ioctl(video_fd, VIDIOC_DQBUF, &buf)) {
                printf("ERROR: failed to VIDIOC_DQBUF\n");
                return;
            }

            memcpy(src_buffer, bufs_info[n_buf].start, bufs_info[n_buf].length);                        //将帧缓冲数据复制到到src_buffer
            yuyv_to_rgb(src_buffer, rgb_buffer);                                                        //yuyv转rgb
            QImage qImage(rgb_buffer, fmt.fmt.pix.width, fmt.fmt.pix.height, QImage::Format_RGB888 );     //构造一个QImage对象


            /* 是否开启本地显示，开启本地显示可能会导致开启广播卡顿，它们互相制约 */
            if (startLocalDisplay)
                emit imageReady(qImage);

            /* 是否开启广播，开启广播会导致本地显示卡顿，它们互相制约 */
            if (startBroadcast) {
                /* udp套接字 */
                QUdpSocket udpSocket;

                /* QByteArray类型 */
                QByteArray byte;

                /* 建立一个用于IO读写的缓冲区 */
                QBuffer buff(&byte);

                /* image转为byte的类型，再存入buff */
                qImage.save(&buff, "JPEG", -1);

                /* 转换为base64Byte类型 */
                QByteArray base64Byte = byte.toBase64();

                /* 由udpSocket以广播的形式传输数据，端口号为8888 */
                udpSocket.writeDatagram(base64Byte.data(), base64Byte.size(), QHostAddress::Broadcast, 7756);
            }

            if (0 > ioctl(video_fd, VIDIOC_QBUF, &buf)) {
                printf("ERROR: failed to VIDIOC_QBUF\n");
                return;
            }
        }
    }
    msleep(800);//at lease 650

    for (int i = 0; i < VIDEO_BUFFER_COUNT; i++) {
        munmap(bufs_info[i].start, buf.length);
    }

    close(video_fd);
#endif
}

