
#include <limits>
#define W32_LEAN_AND_MEAN
#include "OScommon.hpp"
#include "capture.hpp"
#include "captureWin.hpp"
#include "hcore.hpp"
#include "mouse.hpp"
#include "settings.hpp"
#include <comdef.h>
#include <d3d11.h>
#include <dxgi1_2.h>
#include <mftransform.h>
#include <shlwapi.h>
#include <stdio.h>
#include <windows.h>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "DXGI.lib")

namespace OS {

_COM_SMARTPTR_TYPEDEF(IDXGIFactory1, __uuidof(IDXGIFactory1));
_COM_SMARTPTR_TYPEDEF(ID3D11Device, __uuidof(ID3D11Device));
_COM_SMARTPTR_TYPEDEF(ID3D11DeviceContext, __uuidof(ID3D11DeviceContext));
_COM_SMARTPTR_TYPEDEF(IDXGIDevice, __uuidof(IDXGIDevice));
_COM_SMARTPTR_TYPEDEF(IDXGIOutput1, __uuidof(IDXGIOutput1));
_COM_SMARTPTR_TYPEDEF(IDXGIOutput, __uuidof(IDXGIOutput));
_COM_SMARTPTR_TYPEDEF(IDXGIAdapter1, __uuidof(IDXGIAdapter1));
_COM_SMARTPTR_TYPEDEF(IDXGIOutputDuplication, __uuidof(IDXGIOutputDuplication));
_COM_SMARTPTR_TYPEDEF(ID3D11Texture2D, __uuidof(ID3D11Texture2D));
_COM_SMARTPTR_TYPEDEF(IDXGIResource, __uuidof(IDXGIResource));

_COM_SMARTPTR_TYPEDEF(ID3D10Multithread, __uuidof(ID3D10Multithread));
_COM_SMARTPTR_TYPEDEF(IMFActivate, __uuidof(IMFActivate));
_COM_SMARTPTR_TYPEDEF(IMFAttributes, __uuidof(IMFAttributes));
_COM_SMARTPTR_TYPEDEF(IMFDXGIDeviceManager, __uuidof(IMFDXGIDeviceManager));
_COM_SMARTPTR_TYPEDEF(IMFTransform, __uuidof(IMFTransform));
_COM_SMARTPTR_TYPEDEF(IMFMediaEventGenerator, __uuidof(IMFMediaEventGenerator));
_COM_SMARTPTR_TYPEDEF(IMFMediaType, __uuidof(IMFMediaType));
_COM_SMARTPTR_TYPEDEF(IMFSample, __uuidof(IMFSample));
_COM_SMARTPTR_TYPEDEF(IMFMediaBuffer, __uuidof(IMFMediaBuffer));
_COM_SMARTPTR_TYPEDEF(IMF2DBuffer, __uuidof(IMF2DBuffer));

unsigned char **tmp;

bool m_initializedDXGI;
unsigned int m_width;
unsigned int m_height;

// DXGI

IDXGIFactory1Ptr m_dxgiFactory;
ID3D11DevicePtr m_device;
D3D_FEATURE_LEVEL m_featureLevel;
ID3D11DeviceContextPtr m_deviceContext;
IDXGIOutput1Ptr m_output;
IDXGIOutputDuplicationPtr m_dup;
bool m_releaseFrame;
ID3D11Texture2DPtr m_texture;
D3D11_MAPPED_SUBRESOURCE m_mapping;
bool m_surfaceMapped;

BYTE *m_pointer;
UINT m_pointerBufSize;
UINT m_pointerSize;
BOOL m_lastMouseVis;
POINT m_lastMousePos;

// GDI

HDC hScreen;
HDC hdcMem;
HBITMAP hBitmap;
uint32_t bitmapW, bitmapH;
HGDIOBJ hOld;
BITMAPINFOHEADER bmi = {0};

void initBitmap() {
    hBitmap = CreateCompatibleBitmap(hScreen, Settings::capture.w, Settings::capture.h);
    bitmapW = Settings::capture.w;
    bitmapH = Settings::capture.h;
}

void destroyBitmap() { DeleteObject(hBitmap); }

void initTexture(int width, int height) {
    HRESULT status;
    D3D11_TEXTURE2D_DESC texDesc;
    ZeroMemory(&texDesc, sizeof(texDesc));
    texDesc.Width = width;
    texDesc.Height = height;
    texDesc.MipLevels = 1;
    texDesc.ArraySize = 1;
    texDesc.SampleDesc.Count = 1;
    texDesc.SampleDesc.Quality = 0;
    texDesc.Usage = D3D11_USAGE_STAGING;
    texDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    texDesc.BindFlags = 0;
    texDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    texDesc.MiscFlags = 0;

    status = m_device->CreateTexture2D(&texDesc, NULL, &m_texture);
    if (FAILED(status)) {
        m_core->log(LOG_ERROR, "Failed to create texture");
        return;
    }
    m_width = width;
    m_height = height;
}

void deinitTexture() { m_texture = NULL; }

void deinitCapture() {
    deinitTexture();
    m_dup = NULL;
    m_output = NULL;
    m_deviceContext = NULL;
    m_device = NULL;
    m_dxgiFactory = NULL;
    m_initializedDXGI = false;
}

void initDXGI() {
    HRESULT status;

    status = CreateDXGIFactory1(__uuidof(IDXGIFactory1), (void **)(&m_dxgiFactory));
    if (FAILED(status)) {
        m_core->log(LOG_ERROR, "DXGI ERROR %08x", status);
        return;
    }

    bool done = false;
    IDXGIAdapter1Ptr adapter;
    for (int i = 0; m_dxgiFactory->EnumAdapters1(i, &adapter) != DXGI_ERROR_NOT_FOUND; i++) {
        IDXGIOutputPtr output;
        for (int i = 0; adapter->EnumOutputs(i, &output) != DXGI_ERROR_NOT_FOUND; i++) {
            DXGI_OUTPUT_DESC outputDesc;
            output->GetDesc(&outputDesc);
            if (!outputDesc.AttachedToDesktop) {
                output = NULL;
                continue;
            }

            m_output = output;
            if (!m_output) {
                deinitCapture();
                m_core->log(LOG_ERROR, "Failed to get IDXGIOutput1");
                return;
            }

            m_width = outputDesc.DesktopCoordinates.right - outputDesc.DesktopCoordinates.left;
            m_height = outputDesc.DesktopCoordinates.bottom - outputDesc.DesktopCoordinates.top;

            done = true;

            break;
        }

        if (done)
            break;

        adapter = NULL;
    }

    if (!done) {
        m_core->log(LOG_ERROR, "Failed to locate a valid output device");
        deinitCapture();
        return;
    }

    static const D3D_FEATURE_LEVEL featureLevels[] = {D3D_FEATURE_LEVEL_11_1, D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_1,
                                                      D3D_FEATURE_LEVEL_10_0, D3D_FEATURE_LEVEL_9_3,  D3D_FEATURE_LEVEL_9_2,
                                                      D3D_FEATURE_LEVEL_9_1};

#if DEBUG
#define CREATE_FLAGS (D3D11_CREATE_DEVICE_DEBUG)
#else
#define CREATE_FLAGS (0)
#endif

    status = D3D11CreateDevice(adapter, D3D_DRIVER_TYPE_UNKNOWN, NULL, CREATE_FLAGS | D3D11_CREATE_DEVICE_VIDEO_SUPPORT, featureLevels,
                               ARRAYSIZE(featureLevels), D3D11_SDK_VERSION, &m_device, &m_featureLevel, &m_deviceContext);
#undef CREATE_FLAGS

    if (FAILED(status)) {
        m_core->log(LOG_ERROR, "Failed to create D3D11 device");
        deinitCapture();
        return;
    }

    initTexture(m_width, m_height);

    IDXGIDevicePtr dxgi;
    status = m_device->QueryInterface(__uuidof(IDXGIDevice), (void **)&dxgi);
    if (FAILED(status)) {
        m_core->log(LOG_ERROR, "Failed to obtain the IDXGIDevice interface from the D3D11 device");
        deinitCapture();
        return;
    }

    dxgi->SetGPUThreadPriority(7);

    // we try this twice just incase we still get an error
    // on re-initialization
    for (int i = 0; i < 5; ++i) {
        status = m_output->DuplicateOutput(m_device, &m_dup);
        if (SUCCEEDED(status))
            break;
        std::this_thread::sleep_for(milliseconds(5000));
    }

    if (FAILED(status)) {
        m_core->log(LOG_ERROR, "DuplicateOutput Failed %08x", status);

        return;
    }

    m_initializedDXGI = true;

    m_core->log(LOG_INFO, "Init dxgi success");
}

void initGDI() {
    hScreen = GetDC(GetDesktopWindow());
    hdcMem = CreateCompatibleDC(hScreen);
    initBitmap();
    hOld = SelectObject(hdcMem, hBitmap);

    tmp = new unsigned char *[Settings::screen.h];

    bmi.biSize = sizeof(BITMAPINFOHEADER);
    bmi.biPlanes = 1;
    bmi.biBitCount = 32;
    bmi.biWidth = Settings::capture.w;
    bmi.biHeight = -Settings::capture.h;
    bmi.biCompression = BI_RGB;
    bmi.biSizeImage = 0;
}

/*
void initCapture(){
    tmp = new unsigned char*[Settings::screen.h];

    if(Settings::capture.DXGI)
        initDXGI();
    if(!m_initializedDXGI){
        m_core->log(LOG_WARNING,"Using slower capturing mode");
        initGDI();
    }


}
*/

void reinitGDI(Framebuffer &fb) {
    DeleteObject(hBitmap);
    hBitmap = CreateCompatibleBitmap(hScreen, fb.capture.w, fb.capture.h);
    bitmapW = fb.capture.w;
    bitmapH = fb.capture.h;
    hOld = SelectObject(hdcMem, hBitmap);

    bmi.biSize = sizeof(BITMAPINFOHEADER);
    bmi.biPlanes = 1;
    bmi.biBitCount = 32;
    bmi.biWidth = fb.capture.w;
    bmi.biHeight = -fb.capture.h;
    bmi.biCompression = BI_RGB;
    bmi.biSizeImage = 0;
}

void captureGDI(Framebuffer &fb) {
    if (bitmapH != fb.capture.h || bitmapW != fb.capture.w) {
        reinitGDI(fb);
    }

    BitBlt(hdcMem, 0, 0, fb.capture.w, fb.capture.h, hScreen, fb.capture.x, fb.capture.y, SRCCOPY);

    GetDIBits(hScreen, hBitmap, 0, fb.height, fb.data, (BITMAPINFO *)&bmi, DIB_RGB_COLORS);

    /*
    unsigned char *buffer = (unsigned char*)fb.data;

    for(int i=0;i<fb.detection.h;i++){
        tmp[i] =  &buffer[(int)((i+fb.detection.y-fb.capture.y)*
    fb.width*fb.bytesPerPixel +
    ((fb.detection.x-fb.capture.x)*fb.bytesPerPixel))];
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
}

void captureDXGI(Framebuffer &fb) {

    if (m_width != fb.capture.w || m_height != fb.capture.h) {
        m_core->log(LOG_INFO, "Resize Texture w %d h %d", fb.capture.w, fb.capture.h);
        deinitTexture();
        initTexture(fb.capture.w, fb.capture.h);
    }
    HRESULT status;
    IDXGIResourcePtr res;
    ID3D11Texture2DPtr src;
    DXGI_OUTDUPL_FRAME_INFO frameInfo;
    while (true) {

        status = m_dup->AcquireNextFrame(1000, &frameInfo, &res);
        if (status == DXGI_ERROR_WAIT_TIMEOUT) {
            // m_core->log(LOG_WARNING,"DXGI_ERROR_WAIT_TIMEOUT");
            continue;
        }
        fb.m_beginCapture = getCurrentTimeMicro();
        break;
    }

    if (m_surfaceMapped) {
        m_deviceContext->Unmap(m_texture, 0);
        m_surfaceMapped = false;
    }

    res.QueryInterface(IID_PPV_ARGS(&src));
    res = NULL;

    D3D11_BOX sourceRegion;
    sourceRegion.left = fb.capture.x;
    sourceRegion.right = fb.capture.x + fb.capture.w;
    sourceRegion.top = fb.capture.y;
    sourceRegion.bottom = fb.capture.y + fb.capture.h;
    sourceRegion.front = 0;
    sourceRegion.back = 1;

    m_deviceContext->CopySubresourceRegion(m_texture, 0, 0, 0, 0, src, 0, &sourceRegion);
    // SafeRelease(&src);

    m_dup->ReleaseFrame();

    status = m_deviceContext->Map(m_texture, 0, D3D11_MAP_READ, 0, &m_mapping);
    if (FAILED(status)) {
        m_core->log(LOG_ERROR, "Failed to map the texture");
        return;
    }
    m_surfaceMapped = true;

    const unsigned int size = fb.height * fb.width * fb.bytesPerPixel;

    uint16_t threadKey = m_core->generateThreadKey();
    m_core->tPool->resetTaskWaiting(threadKey);

    int threads = (std::max)(m_core->tPool->idleCount(), 8);

    unsigned char *source = (unsigned char *)m_mapping.pData;
    unsigned char *dst = (unsigned char *)fb.data;

    for (int i = 0; i < threads; i++) {
        int start = i * (size / threads);
        int end = (i + 1) * (size / threads);
        m_core->tPool->enqueueBlocking(threadKey, memcpy, dst + start, source + start, end - start);
    }

    m_core->tPool->waitForTasks(threadKey, 0);

    // memcpy(fb.data,m_mapping.pData,size);
}
/*
void captureFrame(Framebuffer &fb){
    static bool warnedAreaTooBig = false;

    if(!warnedAreaTooBig && (fb.width != fb.detection.w  || fb.height !=
fb.detection.h) ){ m_core->log(LOG_WARNING,"Capturing unneeded areas");
        warnedAreaTooBig = true;
    }

    if(m_initializedDXGI)
        captureDXGI(fb);
    else
        captureGDI(fb);
}
*/

void processFrame(Framebuffer &fb) {}

} // namespace OS
