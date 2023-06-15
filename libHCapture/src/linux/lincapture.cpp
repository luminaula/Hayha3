#include <stdint.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>

#include <X11/extensions/XShm.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#include <string.h>
#include <stdlib.h>

#include "hcapture.hpp"

#include <string>


XImage *m_ximg;
XShmSegmentInfo m_shminfo;
Display *m_display;
Window m_root;
Window m_window;
XWindowAttributes m_window_attributes;
Screen *m_screen;

int m_height, m_width;


bool m_windowFound = false;
bool m_shmInited = false;
int err;

std::string m_captureTarget;

int getprop (Display *disp, char *name, Window win) {
    Atom prop = XInternAtom(disp,name,False), type;
    int form, r = 0;
    unsigned long len, remain;
    unsigned char *list;

    if (XGetWindowProperty(disp,win,prop,0,1024,False,AnyPropertyType,
            &type,&form,&len,&remain,&list) != Success) {
            return 0;
    }
    if (type == None){
        //mainCore->log(LOG_DEBUG,"%s is not available.\n",name);
    }
    else {
            r = *list;
    }
    XFree(list);
    return r;
}




char *winame (Display *display, Window win) {
    Atom prop = XInternAtom(display,"WM_NAME",False), type;
    int form;
    unsigned long remain, len;
    unsigned char *list;

    if (XGetWindowProperty(display,win,prop,0,1024,False,XA_STRING,
                &type,&form,&len,&remain,&list) != Success) {
        return NULL;
    }

    return (char*)list;
}


Window *winlist (Display *display, unsigned long *len) {
    Atom prop = XInternAtom(display,"_NET_CLIENT_LIST",False), type;
    int form;
    unsigned long remain;
    unsigned char *list;

    if (XGetWindowProperty(display,XDefaultRootWindow(display),prop,0,1024,False,XA_WINDOW,
                &type,&form,len,&remain,&list) != Success) {
        return 0;
    }
    
    return (Window*)list;
}

Window XfindWindow(const char *windowName){
    unsigned long lenn = 0;
    char *name;

    Window *win = (Window*)winlist(m_display,&lenn);
    if(!win)
        return 0;
    for(uint32_t i=0;i<lenn;i++){
        name = winame(m_display,win[i]);
        if(!name)
            return 0;
        if(strcmp(name,windowName) == 0){
            free(name);
            return win[i];
        }
        free(name);
    }
    return 0;

}

void initShm(int width,int height){
    
    XGetWindowAttributes(m_display, m_window, &m_window_attributes);
    m_screen = m_window_attributes.screen;
    m_ximg = XShmCreateImage(m_display, DefaultVisualOfScreen(m_screen),DefaultDepthOfScreen(m_screen),ZPixmap, NULL, &m_shminfo, width, height);
    m_shminfo.shmid = shmget(IPC_PRIVATE, m_ximg->bytes_per_line * m_ximg->height, IPC_CREAT|0777);
    m_shminfo.shmaddr = m_ximg->data = (char*)shmat(m_shminfo.shmid, 0, 0);
    m_shminfo.readOnly = False;
    XShmAttach(m_display, &m_shminfo);
    m_width = width;
    m_height = height;
    m_shmInited = true;
}

void deinitShm(){
    if(!m_shmInited)
        return;
    XDestroyImage(m_ximg);
    XShmDetach(m_display, &m_shminfo);
    shmdt(m_shminfo.shmaddr);
    m_shmInited = false;
}


void init(int width, int height){
    m_display = XOpenDisplay(nullptr);
    m_root = DefaultRootWindow(m_display);
    Window win = XfindWindow(m_captureTarget.c_str());
    if(win){
        m_windowFound = true;
        m_window = win;
    }
    else{
        m_window = m_root;
        m_windowFound = false;
    }
    initShm(width,height);
}

void deinit(){
    XCloseDisplay(m_display);
}


void* captureFrame(unsigned char *buffer,int x, int y, int width, int height){
    static int counter = 0;
    if(counter++ % 25 == 0 && !m_windowFound){
        Window win = XfindWindow(m_captureTarget.c_str());
        if(win){
            deinitShm();
            deinit();
            init(width,height);
            initShm(width,height);
        }

    }
    
    if(width != m_width || height != m_height){
        deinitShm();
        initShm(width,height);
    }
    
    
    XShmGetImage(m_display, m_window, m_ximg, x, y, 0x00ffffff);

    void *src = (void*)m_ximg->data;

    unsigned int size = m_width*m_height*4;

    //memcpy(buffer,src,size);

    return src;

}


void setMode(bool mode){
}

void setTarget(char *target){
    m_captureTarget = target;
}

void Error(void *e){
    XErrorEvent *error = (XErrorEvent *)e;
    switch(error->error_code){
        case BadWindow:
            m_windowFound = false;
            //LOG(//LOG_ERROR,"XError: BadWindow");
            //m_window = XfindWindow("Counter-Strike: Global Offensive - OpenGL");
            break;
        case BadDrawable:
            m_windowFound = false;
            //LOG(//LOG_ERROR,"XError: BadDrawable");
            //m_window = XfindWindow("Counter-Strike: Global Offensive - OpenGL");
            break;
        case BadMatch:
            //LOG(//LOG_ERROR,"XError: BadMatch");
            break;
        case BadAccess:
            //LOG(//LOG_ERROR,"XError: BadAccess");
            break;
        default:
            //LOG(//LOG_ERROR,"XError: Unhandled code");
            break;
    }
}

bool isAttached(){
    return m_windowFound;
}