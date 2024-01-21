# Network Monitoring

Network video monitoring based on imx6ull, programmed using the QT5 framework, consisting of both server and client components. The server's functionalities include UDP text communication, camera capture, local display, UDP broadcasting, and program termination. The client's functionalities encompass UDP text communication, UDP reception of video images, and video playback.

<img src=".\picture\image-20240120210505410.png" alt="image-20240120210505410" style="zoom: 67%;" />

## 1 Server (服务端)

<img src=".\picture\image-20240120184137153.png" alt="image-20240120184137153" style="zoom: 67%;" />

### 1.1 UDP Text Communication

- Instantiate a `QUdpSocket` object in the widget (window) class constructor without binding it to any port initially.
- When the "Bind Port" button is pressed, bind the previously instantiated `QUdpSocket` to port 7766.
- Similarly, when the "Unbind Port" button is pressed, close (unbind) the `QUdpSocket`.
- Implement mutual exclusion for the bind and unbind buttons in the widget constructor, bind button slot function, and unbind button slot function.
- Connect the `QUdpSocket`'s `readyRead` signal to the widget's `readPendingDatagrams` slot function in the widget constructor. In the `readPendingDatagrams` slot function, use the `readDatagram` function to read the datagrams received by the `QUdpSocket` and display the results in the `textBrowser`.
- Connect the `QUdpSocket`'s `stateChanged` signal to the widget's `mStateChanged` slot function in the widget constructor. In the `mStateChanged` slot function, display the current socket state in the `textBrowser` for debugging purposes.

1.2 Camera Capture

- Instantiate a `CaptureThread` object in the widget constructor, where capturing operations are performed.
- Connect the `clicked` signal of the "Start Capture" button to the `setThreadStart` slot function of the `CaptureThread` object. In the `setThreadStart` slot function, decide whether the thread should run based on whether the button is pressed.
- Connect the `clicked` signal of the "Local Display" checkbox to the `setLocalDisplay` slot function of the `CaptureThread` object. In the `setLocalDisplay` slot function, change the local display flag.
- Connect the `clicked` signal of the "Start Broadcasting" checkbox to the `setBroadcast` slot function of the `CaptureThread` object. In the `setBroadcast` slot function, change the start broadcasting flag.
- Connect the `imageReady` signal of the `CaptureThread` to the `showImage` slot function of the widget. In the `showImage` slot function, display the image prepared by the `CaptureThread` in the video playback box. The video playback box is essentially a label, and the `setPixmap` function under the label is used.
- In the `run` function of the `CaptureThread` thread, use the v4l2 device driver framework to operate the camera device. The flow is as follows:

<img src=".\picture\image-20240120155317652.png" alt="image-20240120155317652" style="zoom: 67%;" />

* In the `run` function of the `CaptureThread` thread, determine whether to enter the loop for yuyv_to_rgb format conversion, local display, and broadcasting based on the `startFlag`.

  - Loop through the buffer
  - Retrieve the buffer
  - Image processing
  - Local display (send the `imageReady` signal based on whether local display is enabled)
  - Broadcast image data
  - Requeue the buffer

* Image Processing

  Since it is a USB camera and not in RGB format, an additional yuyv_to_rgb format conversion is required. The CPU performs the conversion based on the formula.

### 1.3 Local Display

Construct a `QImage` object using the converted RGB format image. Pass this object as a parameter to the `showImage` slot function of the widget. In the `showImage` slot function, display it on the label.

### 1.4 Start Broadcasting

This part of the program is completed in the capture thread (if placed in the main thread, the interface will freeze).

- Create a new UDP socket.
- Create a byte array.
- Create an I/O read-write buffer.
- Convert the image to byte type and store it in `buff`.
- Convert it to base64Byte type.
- Use UDP to broadcast data.

### 1.5 Exit Program

On the development board, there is no "X" button to exit the program. Instead, you can use Ctrl+C to exit the program, but some resources may not be released. To address this, add an exit button to release resources.

- Add an exit button slot function.
  - Stop the capture thread.
    - Set the thread's running status to stop.
    - Add a destructor. In the destructor, set the running status to 0 and wait for the thread to finish.
    - Delete the thread and set the pointer to `nullptr`.
- Close the UDP socket.
  - Unbind the port.
  - Delete the UDP socket.
  - Set the pointer to `nullptr`.
- Close the main window.
  - Delete the UI interface.

## 2 client（客户端）

<img src=".\picture\image-20240120184414516.png" alt="image-20240120184414516" style="zoom: 67%;" />

### 2.1 UDP Text Communication

- Instantiate a `udpsocket` object in the widget (window) class constructor without port binding for now.
- When the bind port button is pressed, bind the recently instantiated `udpsocket` to port 7755.
- Similarly, when the unbind button is pressed, close (unbind) the `udpsocket`.
- In the widget constructor, bind port button slot, and unbind button slot, handle the mutual exclusion of the bind and unbind buttons.
- In the widget constructor, connect the `udpsocket`'s `readyRead` signal to the widget's `readPendingDatagrams` slot. In the `readPendingDatagrams` slot function, use the `readDatagram` function to read the datagrams received by `udpsocket` and display the result in `textBrowser`.
- In the widget constructor, connect the `udpsocket`'s `stateChanged` signal to the widget's `mStateChaged` slot. In the `mStateChaged` slot function, display the current socket state in `textBrowser` for debugging.

### 2.2 UDP Video Reception

- Instantiate a `udpsocket_video` object in the widget (window) class constructor without port binding.
- Connect the `clicked` signal of the start receiving checkbox to the `play_video` slot function of the widget. In the `play_video` slot function, perform the binding and unbinding operations of `udpsocket_video`, binding it to port 7756.

### 2.3 Play Received Video

Connect the `readyRead` signal of `udpsocket_video` to the `videoUpdate` slot function of the widget. In the `videoUpdate` slot function, handle and play the video data.

- Reset the data size.
- Store the data in `datagram`.
- Convert the data from base64 format to byte array format.
- Store it in `QImage` format.
- Display it in the label.

## 3 Other Software and Hardware Introduction

- Kernel: 4.1.15
- Root File System: Debian GNU/Linux 8 (jessie)
- Kernel enables UVC driver-free support.

<img src=".\picture\image-20231228211649871.png" alt="image-20231228211649871.png" style="zoom: 67%;" />

* Development Board: ALPHA I.MX6ULL Development Board
* Qt Version: 5.12.9
* Cross-Compiler Version: 4.9.4

