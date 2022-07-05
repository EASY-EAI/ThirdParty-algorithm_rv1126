
#pragma once
#ifndef CAMERA_V4l2_OPERA_
#define CAMERA_V4l2_OPERA_

#include <asm/types.h>
#include <errno.h>
#include <fcntl.h>
#include <linux/videodev2.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include <chrono>
#include <thread>

#include "scan_pen.h"

 
namespace spen {

// #ifndef SPEN_CAMERA_FRAME
// #define SPEN_CAMERA_FRAME
// enum FrameFormat{
//   NV12,
//   NV21,
//   GRAY,
//   BGR,
//   RGB,
// };
// struct Frame{
//     uint8_t* buf;
//     int width;
//     int height;
//     int32_t sequence;
//     FrameFormat format;
//     // format?
//     void release(){
//       if(buf!=nullptr){
//         delete [] buf;
//         width = 0;
//         height = 0;
//       }
//     }
//     void create(int w, int h, FrameFormat f, uint8_t* ptr){
//       width = w;
//       height =h;
//       format = f;
//       buf = new uint8_t[w*h];
//       if(ptr){
//         memcpy(buf, ptr, w*h);
//       }
//     }
// };
// #endif


#define CLEAR(x) memset(&(x), 0, sizeof(x))
static int xioctl(int fh, int request, void* arg) {
    int r;
    do {
        r = ioctl(fh, request, arg);
    } while (-1 == r && EINTR == errno);
    return r;
}

struct FrameBuffer {
    void* start;
    size_t length;
};

#define DEBUG(...) printf(__VA_ARGS__)
#define ERROR(...) printf(__VA_ARGS__)

#define MAX_BUF_COUNT 4
static FrameBuffer buffers[MAX_BUF_COUNT];
static enum v4l2_buf_type buf_type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
static int fd_;
static int n_buffers;
static int nplanes;
static int frame_width;
static int frame_height;

static int camera_init(const char* dev, int width, int height) {
    int fd = open(dev, O_RDWR, 0);
    if (fd == -1) {
        printf("camera device %s open failed\n", dev);
        return -1;
    }
    fd_ = fd;

    // struct v4l2_input inp;
    // CLEAR(inp);
    // inp.index = 1;
    // if (-1 == xioctl(fd, VIDIOC_S_INPUT, &inp)) {
    //     printf("VIDIOC_S_INPUT %d error!\n", inp.index );
    //     return -1;
    // }

    // check capability
     struct v4l2_capability cap;
    CLEAR(cap);
    if (xioctl(fd, VIDIOC_QUERYCAP, &cap) == -1) {
        ERROR("device not suport operating\n");
        return -1;
    };
    DEBUG("camera capability 0x%x\n", cap.capabilities);
    if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE) && !(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE_MPLANE)) {
        ERROR("device not suport capture\n");
        return -1;
    }
    if (!(cap.capabilities & V4L2_CAP_STREAMING)) {
        ERROR("device not suport streaming\n");
        return -1;
    }
    if (!(cap.capabilities & V4L2_CAP_TIMEPERFRAME)) {
        ERROR("device not suport frame rate\n");
    }

    if (cap.capabilities & V4L2_CAP_VIDEO_CAPTURE) {
        buf_type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    } else if (cap.capabilities & V4L2_CAP_VIDEO_CAPTURE_MPLANE) {
        buf_type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
    }else{
      printf("not find correct  BUF TYPE\n");
      return -1;
    } 
    printf("buf type %d\n",buf_type);
    
    int aviable_format ;
#if 1
    struct v4l2_fmtdesc fmtdesc;
    CLEAR(fmtdesc);
    fmtdesc.index=0;
    fmtdesc.type=buf_type;
    DEBUG("supported frame format:\n");
    while(xioctl(fd,VIDIOC_ENUM_FMT,&fmtdesc)!=-1){
        DEBUG("\t%d:%s(%d)\n",fmtdesc.index,fmtdesc.description,fmtdesc.pixelformat);
        if(fmtdesc.pixelformat == V4L2_PIX_FMT_NV21 || fmtdesc.pixelformat ==V4L2_PIX_FMT_NV12){
            aviable_format = fmtdesc.pixelformat;
        }
        // check frame size of this format
        struct v4l2_frmsizeenum frmsize;
        CLEAR(frmsize);
        frmsize.index = 0;
        frmsize.pixel_format = fmtdesc.pixelformat; 
        while(xioctl(fd,VIDIOC_ENUM_FRAMESIZES,&frmsize)!=-1){
          if(frmsize.type == V4L2_FRMSIZE_TYPE_DISCRETE){
            DEBUG("\t:    DISCRETE size:%d,%d\n",frmsize.discrete.width, frmsize.discrete.height);	
          }else if(frmsize.type == V4L2_FRMSIZE_TYPE_STEPWISE){
            DEBUG("\t:    STEPWISE size:%d,%d,%d,  %d,%d,%d\n",frmsize.stepwise.min_width, frmsize.stepwise.max_width, frmsize.stepwise.step_width, frmsize.stepwise.min_height, frmsize.stepwise.max_height, frmsize.stepwise.step_height);	
          }else {
            DEBUG("\t:    CONTINUOUS size\n");
          }
            // check frame speed of this format.
            struct v4l2_frmivalenum  framival;
            CLEAR(framival);
            framival.index = 0;
            framival.pixel_format = fmtdesc.pixelformat; 
            framival.width = frmsize.discrete.width;
            framival.height = frmsize.discrete.height;
            while(xioctl(fd,VIDIOC_ENUM_FRAMEINTERVALS,&framival)!=-1){
                 DEBUG("\t:        FPS:%d,%d\n",framival.discrete.numerator, framival.discrete.denominator);	
                 framival.index++;
            }
            frmsize.index++;
        }

        fmtdesc.index++;
    }
#endif
    // set frame rate:
    if (0) {
        struct v4l2_streamparm parms;
        CLEAR(parms);
        parms.type = buf_type;
        parms.parm.capture.timeperframe.numerator = 1;
        parms.parm.capture.timeperframe.denominator = 30;
        // parms.parm.capture.capturemode = V4L2_MODE_VIDEO;
        // when different video have the same sensor source, 1:use sensor current win, 0:find the nearest win
        // parms.parm.capture.reserved[0] = 0;
        // parms.parm.capture.reserved[1] = 0; // 2:command, 1: wdr, 0: normal

        if (-1 == xioctl(fd, VIDIOC_S_PARM, &parms)) {
            printf("VIDIOC_S_PARM error\n");
            return -1;
        }
        if (-1 == xioctl(fd, VIDIOC_G_PARM, &parms)) {
            printf("VIDIOC_G_PARM error\n");
            return -1;
        }
        printf("v4l2_streamparm fps: %u, %u\n", parms.parm.capture.timeperframe.numerator,
               parms.parm.capture.timeperframe.denominator);
    }
    // set frame size
    printf("format V4L2_PIX_FMT_YUYV:%d , V4L2_PIX_FMT_NV21:%d, V4L2_PIX_FMT_NV12:%d\n",V4L2_PIX_FMT_YUYV, V4L2_PIX_FMT_NV21, V4L2_PIX_FMT_NV12);
    printf("aviable_format %d\n",aviable_format);
    struct v4l2_format format;
    CLEAR(format);
    format.type = buf_type;
    if(buf_type==V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE){
      printf("buf_type V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE V4L2_PIX_FMT_YUYV %d,%d\n",width,height);
      // TODO  set aviable size and  FORMAT
      format.fmt.pix_mp.width = width;
      format.fmt.pix_mp.height = height;
      format.fmt.pix_mp.pixelformat = aviable_format; // V4L2_PIX_FMT_NV21;  //V4L2_PIX_FMT_NV21M
      format.fmt.pix_mp.field = V4L2_FIELD_NONE;  //V4L2_FIELD_INTERLACED;
    }else {//V4L2_BUF_TYPE_VIDEO_CAPTURE
      format.fmt.pix.width = width;
      format.fmt.pix.height = height;
      format.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;// V4L2_PIX_FMT_NV21;
      format.fmt.pix.field = V4L2_FIELD_NONE;//V4L2_FIELD_INTERLACED;  // V4L2_FIELD_NONE
    }
    
    if (xioctl(fd, VIDIOC_S_FMT, &format) == -1) {
        ERROR("not support required prop: buf_type %d format %d, size %d,%d\n", buf_type, format.fmt.pix_mp.pixelformat, width,
              height);
        return -1;
    }
    if (-1 == xioctl(fd, VIDIOC_G_FMT, &format)) {
        printf("VIDIOC_G_FMT error!\n");
        return -1;
    }
    nplanes = format.fmt.pix_mp.num_planes;
    frame_width = format.fmt.pix_mp.width;
    frame_height = format.fmt.pix_mp.height;
    printf("resolution got from sensor = %d*%d num_planes = %d\n", format.fmt.pix_mp.width, format.fmt.pix_mp.height,
           format.fmt.pix_mp.num_planes);

    // set frame buffer
    struct v4l2_requestbuffers req;
    CLEAR(req);
    req.count = MAX_BUF_COUNT;
    req.type = buf_type;
    req.memory = V4L2_MEMORY_MMAP;
    if (xioctl(fd, VIDIOC_REQBUFS, &req) == -1) {
        ERROR("V4L2_MEMORY_MMAP VIDIOC_REQBUFS\n");
        return -1;
    }
    DEBUG("buffer count %d\n", req.count);

#define FMT_NUM_PLANES 1
    for (n_buffers = 0; n_buffers < req.count; ++n_buffers) {
        struct v4l2_buffer buf;
        struct v4l2_plane planes[FMT_NUM_PLANES];
        CLEAR(buf);
        CLEAR(planes);
        buf.type = buf_type;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = n_buffers;
        if (V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE == buf_type) {
            buf.m.planes = planes;
            buf.length = FMT_NUM_PLANES;
        }
        if (xioctl(fd, VIDIOC_QUERYBUF, &buf) == -1) {
            ERROR("V4L2_MEMORY_MMAP VIDIOC_QUERYBUF\n");
            return -1;
        }
        if (V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE == buf_type) {
            buffers[n_buffers].length = buf.m.planes[0].length;
            buffers[n_buffers].start = mmap(NULL, buf.m.planes[0].length, PROT_READ | PROT_WRITE, MAP_SHARED, fd,
                                            buf.m.planes[0].m.mem_offset);
        } else {
            buffers[n_buffers].length = buf.length;
            buffers[n_buffers].start = mmap(NULL, buf.length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, buf.m.offset);
        }
    }

    long tms0 = now();
    for (int i = 0; i < n_buffers; ++i) {
        struct v4l2_buffer buf;
        CLEAR(buf);
        buf.type = buf_type;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = i;
        struct v4l2_plane planes[FMT_NUM_PLANES];
        CLEAR(planes);
        if (V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE == buf_type) {
            buf.m.planes = planes;
            buf.length = FMT_NUM_PLANES;
        }
        if (xioctl(fd_, VIDIOC_QBUF, &buf) == -1) {
            ERROR("Start() :VIDIOC_QBUF failed \n");
            return -1;
        }
    }
    long tms1 = now();
    DEBUG("camera prepare buffer VIDIOC_QBUF  %ldms \n", (tms1 - tms0));
    return 0;
}
static int camera_start() {
    long tms1 = now();
    enum v4l2_buf_type type = buf_type;
    if (xioctl(fd_, VIDIOC_STREAMON, &type) == -1) {
        ERROR("Start() VIDIOC_STREAMON failed\n");
        return -1;
    }
    long tms2 = now();
    DEBUG("camera start previewing   %ldms.\n", (tms2 - tms1));
    return 0;
}
static int camera_read_frame(Frame* frame) {
    struct v4l2_buffer buf;
    int i, bytesused;
    CLEAR(buf);
    buf.type = buf_type;
    buf.memory = V4L2_MEMORY_MMAP;
// {
//       fd_set fds;
//     struct timeval tv;
//     int r;

//     FD_ZERO(&fds);
//     FD_SET(fd_, &fds);

//     tv.tv_sec = 2; /* Timeout. */
//     tv.tv_usec = 0;
//     r = select(fd_ + 1, &fds, NULL, NULL, &tv);

//     if (-1 == r) {
//         if (errno == EINTR)
//             return -1;
//         printf("select err\n");
//     }
//     if (r == 0) {
//         fprintf(stderr, "select timeout\n");
//         return -1;
//     }
// }
    struct v4l2_plane planes[FMT_NUM_PLANES];
    CLEAR(planes[0]);
    if (V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE == buf_type) {
        buf.m.planes = planes;
        buf.length = FMT_NUM_PLANES;
    }

    if (xioctl(fd_, VIDIOC_DQBUF, &buf) == -1) {
        ERROR("GetFrame  VIDIOC_DQBUF failed \n");
        return -1;
    }
    i = buf.index;
    if (V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE == buf_type) {
        bytesused = buf.m.planes[0].bytesused;
    } else {
        bytesused = buf.bytesused;
    }
    memcpy(frame->buf, buffers[i].start, bytesused);
    frame->width = frame_width;
    frame->height = frame_height;
    frame->format = GRAY;
    frame->sequence = buf.sequence;

    if (xioctl(fd_, VIDIOC_QBUF, &buf) == -1) {
        ERROR("GetFrame get VIDIOC_DQBUF (read current frame) failed \n");
        return -1;
    }
    // DEBUG("got one frame %d", buf.sequence);
    return 0;
}
static int camera_stop() {
    enum v4l2_buf_type type = buf_type;
    if (xioctl(fd_, VIDIOC_STREAMOFF, &type) == -1) {
        ERROR("camera VIDIOC_STREAMOFF failed\n");
        return -1;
    }
    DEBUG("camera stop previewing\n");
    return 0;
}
static int camera_close() {
    for (int i = 0; i < n_buffers; ++i) {
        if (-1 == munmap(buffers[i].start, buffers[i].length)) {
            ERROR("munmap failed\n");
        }
    }

    close(fd_);
    return 0;
}


class Camera {
   public:
    Frame frame_;
    int width_;
    int height_;
    char dev_buf[128];
    Camera(const char* dev, int width, int height) {
        width_ = width;
        height_ = height;
        strcpy(dev_buf, dev);
    }
    int Open() {
        camera_init(dev_buf, width_, height_);
        frame_.buf = new uint8_t[width_ * height_ * 3 / 2];
        frame_.width = width_;
        frame_.height = height_;
        return 0;
    }
    int Init() { return 0; }
    int Start() {
        camera_start();
        return 0;
    }
    int ReadFrame(Frame& frame) {
        printf("camera_read_frame\n");
        int ret = camera_read_frame(&frame_);
        frame = frame_;
        return ret;
    }
    int Stop() {
        camera_stop();
        return 0;
    }
    void Close() {
        camera_close();
        delete[] frame_.buf;
    }
};

}  // namespace spen
#endif
