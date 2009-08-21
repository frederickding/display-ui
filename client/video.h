#ifndef video_header
#define video_header


IVMRWindowlessControl *video_getpointer_VMRwc();
IGraphBuilder *video_getpointer_GraphBldr();
IMediaControl *video_getpointer_MediaCtrl();

HRESULT InitWindowlessVMR(HWND hwndApp, IGraphBuilder* pGraph, IVMRWindowlessControl** ppWc);
HRESULT video_load(HWND hwnd, video_element_t *video);
void video_unload(video_element_t *video);
void video_render(HWND hwnd, HDC hdc, video_element_t *video);

HRESULT video_getduration(LONGLONG *duration);
HRESULT video_getposition(LONGLONG *position);

void video_init();
void video_shutdown();

#endif

