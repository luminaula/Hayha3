#include <windows.h>
#include "hcapture.hpp"
#include <limits>
#include <comdef.h>
#include <dxgi1_5.h>
#include <d3d11.h>
#include <d3d12.h>
#include <mftransform.h>
#include <shlwapi.h>
#include <stdio.h>
#include <stdint.h>
#include <thread>
#include <chrono>

#include <stdio.h>

#include <iostream>

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"d3d11.lib")
#pragma comment(lib, "DXGI.lib")

_COM_SMARTPTR_TYPEDEF(IDXGIFactory5         , __uuidof(IDXGIFactory5         ));
_COM_SMARTPTR_TYPEDEF(ID3D11Device          , __uuidof(ID3D11Device          ));
_COM_SMARTPTR_TYPEDEF(ID3D11DeviceContext   , __uuidof(ID3D11DeviceContext   ));
_COM_SMARTPTR_TYPEDEF(IDXGIDevice           , __uuidof(IDXGIDevice           ));
_COM_SMARTPTR_TYPEDEF(IDXGIOutput5          , __uuidof(IDXGIOutput5          ));
_COM_SMARTPTR_TYPEDEF(IDXGIOutput           , __uuidof(IDXGIOutput           ));
_COM_SMARTPTR_TYPEDEF(IDXGIAdapter1         , __uuidof(IDXGIAdapter1         ));
_COM_SMARTPTR_TYPEDEF(IDXGIOutputDuplication, __uuidof(IDXGIOutputDuplication));
_COM_SMARTPTR_TYPEDEF(ID3D11Texture2D       , __uuidof(ID3D11Texture2D       ));
_COM_SMARTPTR_TYPEDEF(IDXGIResource         , __uuidof(IDXGIResource         ));




bool           m_initializedDXGI;
unsigned int   m_width;
unsigned int   m_height;

//DXGI

IDXGIFactory5Ptr                m_dxgiFactory;
ID3D11DevicePtr                 m_device;
D3D_FEATURE_LEVEL               m_featureLevel;
ID3D11DeviceContextPtr          m_deviceContext;
IDXGIOutput5Ptr                 m_output;
IDXGIOutputDuplicationPtr       m_dup;
bool                            m_releaseFrame;
ID3D11Texture2DPtr              m_texture;
D3D11_MAPPED_SUBRESOURCE        m_mapping;
bool                            m_surfaceMapped;

BYTE *                          m_pointer;
UINT                            m_pointerBufSize;
UINT                            m_pointerSize;
BOOL                            m_lastMouseVis;
POINT                           m_lastMousePos;

//GDI

HDC hScreen;
HDC hdcMem;
HWND hProcess;
HBITMAP hBitmap;
uint32_t bitmapW,bitmapH;
HGDIOBJ hOld;
BITMAPINFOHEADER bmi = {0};

bool useDXGI = false;
bool initedGDI = false;

std::string captureTarget;

void initBitmap(int width,int height){
    hBitmap = CreateCompatibleBitmap(hScreen, width,height);
    bitmapW = width;
    bitmapH = height;
}

void destroyBitmap(){
    DeleteObject(hBitmap);
}

void initTexture(int width, int height){
    HRESULT status;
    D3D11_TEXTURE2D_DESC texDesc;
    ZeroMemory(&texDesc, sizeof(texDesc));
    texDesc.Width              = width;
    texDesc.Height             = height;
    texDesc.MipLevels          = 1;
    texDesc.ArraySize          = 1;
    texDesc.SampleDesc.Count   = 1;
    texDesc.SampleDesc.Quality = 0;
    texDesc.Usage              = D3D11_USAGE_STAGING;
    texDesc.Format             = DXGI_FORMAT_B8G8R8A8_UNORM;
    texDesc.BindFlags          = 0;
    texDesc.CPUAccessFlags     = D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;
    texDesc.MiscFlags          = 0;

    status = m_device->CreateTexture2D(&texDesc, NULL, &m_texture);
    if (FAILED(status))
    {
        //m_core->log(LOG_ERROR,"Failed to create texture");
        return;
    }
    m_width = width;
    m_height = height;
}

void deinitTexture(){
    m_texture = NULL;
}

void deinitCapture(){
    deinitTexture();
    m_dup = NULL;
    m_output = NULL;
    m_deviceContext = NULL;
    m_device = NULL;
    m_dxgiFactory = NULL;
    m_initializedDXGI = false;
}

void initDXGI(){
    HRESULT status;

    status = CreateDXGIFactory1(__uuidof(IDXGIFactory1), (void **)(&m_dxgiFactory));
    if (FAILED(status))
    {
        //m_core->log(LOG_ERROR,"DXGI ERROR %08x",status);
        return;
    }

    bool done = false;
    IDXGIAdapter1Ptr adapter;
    for (int i = 0; m_dxgiFactory->EnumAdapters1(i, &adapter) != DXGI_ERROR_NOT_FOUND; i++)
    {
        IDXGIOutputPtr output;
        for (int i = 0; adapter->EnumOutputs(i, &output) != DXGI_ERROR_NOT_FOUND; i++)
        {
            DXGI_OUTPUT_DESC outputDesc;
            output->GetDesc(&outputDesc);

            if (!outputDesc.AttachedToDesktop)
            {
                output = NULL;
                continue;
            }

            m_output = output;
            if (!m_output)
            {
                deinitCapture();
                //m_core->log(LOG_ERROR,"Failed to get IDXGIOutput1");
                return;
            }

            m_width  = outputDesc.DesktopCoordinates.right  - outputDesc.DesktopCoordinates.left;
            m_height = outputDesc.DesktopCoordinates.bottom - outputDesc.DesktopCoordinates.top;

            done = true;

            break;
        }

        if (done)
            break;

        adapter = NULL;
    }

    if (!done)
    {
        //m_core->log(LOG_ERROR,"Failed to locate a valid output device");
        deinitCapture();
        return;
    }

    static const D3D_FEATURE_LEVEL featureLevels[] = {
        D3D_FEATURE_LEVEL_12_1,
        D3D_FEATURE_LEVEL_12_0,
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
        D3D_FEATURE_LEVEL_9_3,
        D3D_FEATURE_LEVEL_9_2,
        D3D_FEATURE_LEVEL_9_1
    };

    #if DEBUG
        #define CREATE_FLAGS (D3D11_CREATE_DEVICE_DEBUG)
    #else
        #define CREATE_FLAGS (0)
    #endif

    status = D3D11CreateDevice(
        adapter,
        D3D_DRIVER_TYPE_UNKNOWN,
        NULL,
        CREATE_FLAGS | D3D11_CREATE_DEVICE_VIDEO_SUPPORT | DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING,
        featureLevels, ARRAYSIZE(featureLevels),
        D3D11_SDK_VERSION,
        &m_device,
        &m_featureLevel,
        &m_deviceContext
    );
    #undef CREATE_FLAGS  

    if (FAILED(status))
    {
        //m_core->log(LOG_ERROR,"Failed to create D3D11 device");
        deinitCapture();
        return;
    }


    initTexture(m_width,m_height);



    IDXGIDevicePtr dxgi;
    status = m_device->QueryInterface(__uuidof(IDXGIDevice), (void **)&dxgi);
    if (FAILED(status))
    {
        //m_core->log(LOG_ERROR,"Failed to obtain the IDXGIDevice interface from the D3D11 device");
        deinitCapture();
        return;
    }

    dxgi->SetGPUThreadPriority(7);

    // we try this twice just incase we still get an error
    // on re-initialization

    const DXGI_FORMAT supportedFormats[] = {
        DXGI_FORMAT_R8G8B8A8_UNORM
    };

    for(int i = 0; i < 5; ++i)
    {
        status = m_output->DuplicateOutput(m_device, &m_dup);
        if (SUCCEEDED(status))
            break;
        std::this_thread::sleep_for(std::chrono::milliseconds(5000));
    }

    if (FAILED(status))
    {
        //m_core->log(LOG_ERROR,"DuplicateOutput Failed %08x",status);

        return;
    }

    m_initializedDXGI = true;


    //m_core->log(LOG_INFO,"Init dxgi success");
}


void initGDI(int width, int height){
    hProcess = FindWindow(NULL,captureTarget.c_str());
    if(hProcess == NULL)
        return;
    hScreen = GetWindowDC(hProcess);
    //hScreen = GetDC(GetDesktopWindow());
    hdcMem = CreateCompatibleDC (hScreen);
    initBitmap(width,height);
    hOld = SelectObject(hdcMem, hBitmap);

    
    
    //tmp = new unsigned char*[Settings::screen.h];

    bmi.biSize = sizeof(BITMAPINFOHEADER);
    bmi.biPlanes = 1;
    bmi.biBitCount = 32;
    bmi.biWidth = width;
    bmi.biHeight = -height;
    bmi.biCompression = BI_RGB;
    bmi.biSizeImage = 0;

    initedGDI = true;

}

void deInitGDI(){
    initedGDI = false;
    DeleteObject(hBitmap);
    DeleteDC(hdcMem);
}

void reinitGDI(int width, int height){
    DeleteObject(hBitmap);
    hBitmap = CreateCompatibleBitmap(hScreen,width,height);
    bitmapW = width;
    bitmapH = height;
    

    bmi.biSize = sizeof(BITMAPINFOHEADER);
    bmi.biPlanes = 1;
    bmi.biBitCount = 32;
    bmi.biWidth = width;
    bmi.biHeight = -height;
    bmi.biCompression = BI_RGB;
    bmi.biSizeImage = 0;

}

void *captureGDI(unsigned char *buffer,int x,int y, int width,int height){
    static uint32_t counter = 0;

    if(counter++ % 25 == 0){
        HWND win = FindWindow(NULL,captureTarget.c_str());
        if(win == NULL){
            if(initedGDI){
                deInitGDI();
            }
            return buffer;
        }
    }
    

    if(!initedGDI){
        initGDI(416,416);
        return buffer;
    }

    

    if(bitmapH != height || bitmapW != width){
        reinitGDI(width,height);
    }

    hOld = SelectObject(hdcMem, hBitmap);

    BitBlt(hdcMem, 0, 0, width, height, hScreen, x, y, SRCCOPY);


    GetDIBits(hScreen, hBitmap, 0, height, buffer, (BITMAPINFO*)&bmi, DIB_RGB_COLORS);
    

    /*
    unsigned char *buffer = (unsigned char*)fb.data;

    for(int i=0;i<fb.detection.h;i++){
        tmp[i] =  &buffer[(int)((i+fb.detection.y-fb.capture.y)* fb.width*fb.bytesPerPixel + ((fb.detection.x-fb.capture.x)*fb.bytesPerPixel))];
    }
    

    uint16_t threadKey = m_core->generateThreadKey();
    m_core->tPool->resetTaskWaiting(threadKey);
    int threads = 4;
    for(int i=0;i<threads;i++){
        int start = (int)(i * ((float)fb.fHeight/threads));
        int end = (int)((float)fb.fHeight/threads*(i+1));                
        m_core->tPool->enqueueBlocking(threadKey,&Framebuffer::scaleRows,fb,start,end,fb.detection.w,fb.detection.h,tmp);
        
    }

    m_core->tPool->waitForTasks(threadKey, 5000);

    */


   return buffer;

}

void *captureDXGI(unsigned char *buffer,int x,int y, int width,int height){
    if(m_width != width || m_height != height){
        //m_core->log(LOG_INFO,"Resize Texture w %d h %d",fb.capture.w,fb.capture.h);
        deinitTexture();
        initTexture(width,height);
    }



    HRESULT status;
    IDXGIResourcePtr res;
    ID3D11Texture2DPtr src;
    DXGI_OUTDUPL_FRAME_INFO frameInfo;
    while(true){
        
        status = m_dup->AcquireNextFrame(1000, &frameInfo, &res);
        if(status == DXGI_ERROR_WAIT_TIMEOUT){
            //m_core->log(LOG_WARNING,"DXGI_ERROR_WAIT_TIMEOUT");
            continue;
        }
        if(status !=0){
            //printf("Error\n");
            std::this_thread::sleep_for(std::chrono::seconds(10));
            initDXGI();
        }
        //fb.m_beginCapture = getCurrentTimeMicro();
        break;
    }
        

    if (m_surfaceMapped){
        m_deviceContext->Unmap(m_texture, 0);
        m_surfaceMapped = false;
    }

    res.QueryInterface(IID_PPV_ARGS(&src));
    res = NULL;

    D3D11_BOX sourceRegion;
    sourceRegion.left = x;
    sourceRegion.right = x + width;
    sourceRegion.top = y;
    sourceRegion.bottom = y + height;
    sourceRegion.front = 0;
    sourceRegion.back = 1;

    m_deviceContext->CopySubresourceRegion(m_texture,0,0,0,0,src,0,&sourceRegion);
    //SafeRelease(&src);
    
    m_dup->ReleaseFrame();

    status = m_deviceContext->Map(m_texture, 0, D3D11_MAP_READ, 0, &m_mapping);
    if (FAILED(status))
    {
        //m_core->log(LOG_ERROR,"Failed to map the texture");
        return buffer;
    }
    m_surfaceMapped = true;
    
    const unsigned int size = height * width * 4;
    
    /*

    uint16_t threadKey = m_core->generateThreadKey();
    m_core->tPool->resetTaskWaiting(threadKey);
    
    int threads = (std::max)(m_core->tPool->idleCount(),8);

    unsigned char *source = (unsigned char*)m_mapping.pData;
    unsigned char *dst = (unsigned char*)fb.data;

    for(int i=0;i<threads;i++){
        int start = i * (size/threads);
        int end = (i+1) * (size/threads);
        m_core->tPool->enqueueBlocking(threadKey,memcpy,dst+start,source+start,end-start);
    }

    m_core->tPool->waitForTasks(threadKey, 0);
    
    */
    return m_mapping.pData;
    //memcpy(buffer,m_mapping.pData,size);
}

void init(int width, int height){
    if(useDXGI){
        initDXGI();
    }
    else{
        //m_core->log(LOG_WARNING,"Using slower capturing mode");
        initGDI(width,height);
    }
    

    
}

void deinit(){

}


void *captureFrame(unsigned char *buffer,int x, int y, int width, int height){
    if(m_initializedDXGI && useDXGI){
        return captureDXGI(buffer,x,y,width,height);
    }
    else{
        return captureGDI(buffer,x,y,width,height);
    }
}

void setMode(bool mode){
    useDXGI = mode;
}

void setTarget(char *target){
    captureTarget = target;
}

void Error(void *e){
    
}

bool isAttached(){
    bool ret = IsWindowVisible(hProcess);
    return ret;
}