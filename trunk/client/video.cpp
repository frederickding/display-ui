#include <windows.h>
#include <stdio.h>

#include <Qedit.h>
#include <dshow.h>

#include "debug.h"
#include "playlist.h"
#include "video.h"


IVMRWindowlessControl *g_iVMRwc = NULL;
IGraphBuilder *g_iGraphBldr		= NULL;
IMediaControl *g_iMedCtrl		= NULL;
IMediaEvent *g_iMedEvt			= NULL;
IMediaSeeking *g_iMedSeek		= NULL;

bool g_video_loaded = false;
volatile bool g_video_initialized = false;

IVMRWindowlessControl *video_getpointer_VMRwc(){
	return g_iVMRwc;
}

IGraphBuilder *video_getpointer_GraphBldr(){
	return g_iGraphBldr;	
}

IMediaControl *video_getpointer_MediaCtrl(){
	return g_iMedCtrl;
}

// src: http://msdn.microsoft.com/en-us/library/ms787876%28VS.85,printer%29.aspx
HRESULT InitWindowlessVMR( 
						  HWND hwndApp,                  // Window to hold the video. 
						  IGraphBuilder* pGraph,         // Pointer to the Filter Graph Manager. 
						  IVMRWindowlessControl** ppWc   // Receives a pointer to the VMR.
						  )
{ 
    if (!pGraph || !ppWc) 
        return E_POINTER;

    IBaseFilter* pVmr = NULL; 
    IVMRWindowlessControl* pWc = NULL; 

    // Create the VMR. 
    HRESULT hr = CoCreateInstance(CLSID_VideoMixingRenderer, NULL, CLSCTX_INPROC, IID_IBaseFilter, (void**)&pVmr); 

    if (FAILED(hr))
        return hr;

    // Add the VMR to the filter graph.
    hr = pGraph->AddFilter(pVmr, L"Video Mixing Renderer"); 
    if (FAILED(hr)){
        pVmr->Release();
        return hr;
    }

    // Set the rendering mode.  
    IVMRFilterConfig* pConfig; 
    hr = pVmr->QueryInterface(IID_IVMRFilterConfig, (void**)&pConfig); 
    if (SUCCEEDED(hr)){ 
        hr = pConfig->SetRenderingMode(VMRMode_Windowless); 
        pConfig->Release(); 
    }
	
    if (SUCCEEDED(hr)){
        // Set the window. 
        hr = pVmr->QueryInterface(IID_IVMRWindowlessControl, (void**)&pWc);
        if( SUCCEEDED(hr)) { 
            hr = pWc->SetVideoClippingWindow(hwndApp); 
            if (SUCCEEDED(hr)){
                *ppWc = pWc; // Return this as an AddRef'd pointer. 
            }else{
                // An error occurred, so release the interface.
                pWc->Release();
            }
        } 
    } 

    pVmr->Release(); 
    return hr; 
}

HRESULT video_load(HWND hwnd, video_element_t *video){
	HRESULT hr;
	
	debug_print("loading video...\n");
	
	if(!g_video_initialized){
		video_init();
	}

	hr = CoCreateInstance( CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER, IID_IGraphBuilder, (void **)&g_iGraphBldr);
	if(FAILED(hr)){
		debug_print("CoCreateInstance failed: hr = %08lX\n", hr);
		
	}
	

	hr = g_iGraphBldr->QueryInterface(IID_IMediaControl, (void **)&g_iMedCtrl);
	hr = g_iGraphBldr->QueryInterface(IID_IMediaEvent, (void **)&g_iMedEvt);
	hr = g_iGraphBldr->QueryInterface(IID_IMediaSeeking, (void **)&g_iMedSeek);

	hr = InitWindowlessVMR(hwnd, g_iGraphBldr, &g_iVMRwc);

	if (SUCCEEDED(hr)){
		debug_print("InitWindowslessVMR succeeded.\n");
		// Build the graph. For example:
		//g_iGraphBldr->RenderFile(L"C:\\Program Files\\Microsoft Platform SDK\\Samples\\Multimedia\\DirectShow\\Media\\Video\\skiing.avi", 0);

		g_iGraphBldr->RenderFile(video->filename_w, 0);
		//g_iGraphBldr->RenderFile(L"C:\\downloaded\\[Mazui]_Suzumiya_Haruhi_no_Yuuutsu_(2009)_-_21_[6CF2660F].mkv", 0);
		
		// Find the native video size.
		long width, height; 
		HRESULT hr = g_iVMRwc->GetNativeVideoSize(&width, &height, NULL, NULL); 
		
		debug_print("video dimensions: %dx%d\n", width, height);
		
		if (SUCCEEDED(hr)){
			RECT rc_src, rc_dst; 

			// Set the source rectangle.
			SetRect(&rc_src, 0, 0, width, height); 
			
			// Set the destination rectangle.
			if(width < 640){
				SetRect(&rc_dst, 980/2 - width/2, 553/2 - height/2 + 112, 
				980/2 + width/2, 553/2 + height/2 + 112); 
			}else{
				SetRect(&rc_dst, 0, 112, 979, 665);	
			}

			// Set the video position.
			hr = g_iVMRwc->SetVideoPosition(&rc_src, &rc_dst); 

			LONGLONG position = 0;
			LONGLONG duration;
			video_getduration(&duration);
			
			g_iMedSeek->SetPositions(&position, AM_SEEKING_AbsolutePositioning, &duration, AM_SEEKING_AbsolutePositioning);

			debug_print("starting video...");
			g_iMedCtrl->Run();

			g_video_loaded = true;
		}
	}else{
		printf("InitWindowslessVMR failed: %08lX\n", hr);
	}
	return hr;
}

void video_unload(video_element_t *video){
	if(g_iVMRwc) g_iVMRwc->Release();
	if(g_iGraphBldr) g_iGraphBldr->Release();
	if(g_iMedCtrl) g_iMedCtrl->Release();
	if(g_iMedEvt) g_iMedEvt->Release();
	if(g_iMedSeek) g_iMedSeek->Release();

	g_iVMRwc = NULL;
	g_iGraphBldr = NULL;
	g_iMedCtrl = NULL;
	g_iMedEvt = NULL;
	g_iMedSeek = NULL;

	g_video_loaded = false;
}

void video_render(HWND hwnd, HDC hdc, video_element_t *video){
	if(g_iVMRwc) g_iVMRwc->RepaintVideo(hwnd, hdc);
}

HRESULT video_getduration(LONGLONG *duration){
	return g_iMedSeek->GetDuration(duration);
}

HRESULT video_getposition(LONGLONG *current){
	return g_iMedSeek->GetCurrentPosition(current);
}

bool video_isloaded(){
	return g_video_loaded;
}

bool video_isinit(){
	return g_video_initialized;
}

void video_init(){
	//HRESULT hr;
	CoInitialize(NULL);
	
	g_video_initialized = true;

	debug_print("video initialized.\n");
}

void video_shutdown(){
	if(g_iVMRwc) g_iVMRwc->Release();
	if(g_iGraphBldr) g_iGraphBldr->Release();
	if(g_iMedCtrl) g_iMedCtrl->Release();
	if(g_iMedEvt) g_iMedEvt->Release();
	if(g_iMedSeek) g_iMedSeek->Release();

	CoUninitialize();
}
