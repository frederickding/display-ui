/**
 * display_ui.cpp, the entry point for the application
 *
 * Copyright 2009 Kirill Peretoltchine, Frederick Ding
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 *
 * You may obtain a copy of the License at
 *      http://www.apache.org/licenses/LICENSE-2.0
 * or the full licensing terms for this project at
 *      http://code.google.com/p/display-ui/wiki/License
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * $Id$
 */

#define _WIN32_IE 0x0501

//#define FULLSCREEN
//#define SKIPVIDEO

#include <windows.h>
#include <urlmon.h>
#include <wchar.h>
#include <stdio.h>
#include <time.h>

//#include <Qedit.h>
#include <dshow.h>

#include <curl\curl.h>


// because dshow.h does REALLY stupid stuff
// (see line 7673 of strsafe.h. WTF, microsoft)
#undef sprintf
#undef strcat

#include "resource.h"
#include "display_ui.h"
#include "freeimage.h"
#include "debug.h"
#include "weather.h"
#include "playlist.h"
#include "video.h"
#include "config.h"

#pragma comment(lib, "freeimage.lib")
#pragma comment(lib, "msimg32.lib")
#pragma comment(lib, "strmiids.lib")
#pragma comment(lib, "libcurl.lib")
#pragma comment(lib, "version.lib")
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "winmm.lib")

#define FRAMES_PER_SEC 25 // should be a factor of 1000!
#define FRAME_INTERVAL 1000 / FRAMES_PER_SEC

#define MakeFont(name, bold, italic, underline, size) CreateFont(-size, \
			0, 0, 0, (bold ? FW_BOLD : FALSE), (italic ? FW_BOLD : FALSE), (underline ? FW_BOLD : FALSE), FALSE, ANSI_CHARSET, \
			OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, PROOF_QUALITY, DEFAULT_PITCH | FF_ROMAN, name)

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
void load_headlines();
void repaint_content(HWND hWnd, HDC hdcMem, PAINTSTRUCT ps);
void repaint_weather(HDC hdcMem, PAINTSTRUCT ps);
void repaint_headlines(HDC hdcMem, PAINTSTRUCT ps);
void repaint_header(HDC hdcMem, PAINTSTRUCT ps);

char *g_headline_txt = NULL; //"The quick brown fox jumps over the lazy dog | Lorem ipsum dolor sit amet, consectetur adipiscing elit. Nulla lobortis hendrerit hendrerit.";
int g_ticks = 0;
int g_headline_x = DISPLAY_WIDTH;
bool g_video_painting = false;
bool g_goto_next = false;

static FIBITMAP *g_fbmp_bg			= NULL;
static FIBITMAP *g_fbmp_header		= NULL;
static FIBITMAP *g_fbmp_weatherbg	= NULL;
static FIBITMAP *g_fbmp_weathergrad	= NULL;
static FIBITMAP *g_fbmp_headlines1	= NULL;
static FIBITMAP *g_fbmp_headlines2	= NULL;
static FIBITMAP *g_fbmp_loading		= NULL;
HFONT g_font_dejavusans_bold_20;
HFONT g_font_dejavusans_bold_24;
HFONT g_font_dejavusans_cond_12;
HFONT g_font_dejavusans_cond_18;
HFONT g_font_dejavusans_cond_bold_24;
HFONT g_font_dejavusans_cond_bold_32;
HFONT g_font_dejavusans_cond_bold_42;
HFONT g_font_dejavusans_cond_bold_58;

extern playlist_t g_playlist;
playlist_element_t *g_current_elem;

// Called from a thread to update weather & headlines
// params:
//		p:	 The HWND of the window in which to paint the info
void update(void *p) {
	weather_update(p, true);

	if(dui_download("/api/headlines/fetch/", "data\\headlines.dat") == CURLE_OK) {
		load_headlines();
	}
}

void update_playlist(void *p) {
	playlist_retry_load((HWND)p);
}

// Reads headline data from a file
void load_headlines(){
	FILE *fp = fopen("data\\headlines.dat", "r");
	
	if(fp) {
		if(g_headline_txt) {
			free(g_headline_txt);
		}
		fseek(fp, 0, SEEK_END);
		int size = ftell(fp);
		fseek(fp, 0, SEEK_SET);
		
		
		g_headline_txt = (char *) malloc(size + 2);
		memset(g_headline_txt, 0, size + 2);
		fread(g_headline_txt, 1, size, fp);

		g_headline_x = DISPLAY_WIDTH;
		
		fclose(fp);
	}
}

// the entry point for any Windows program
int WINAPI WinMain(HINSTANCE hInstance,
				   HINSTANCE hPrevInstance,
				   LPSTR lpCmdLine,
				   int nCmdShow)
{
	HWND hWnd;
	WNDCLASSEX wc;
	
	ZeroMemory(&wc, sizeof(WNDCLASSEX));
	

	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = (WNDPROC)WndProc;
	wc.hInstance = hInstance;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)COLOR_GRAYTEXT;    // not needed any more
	wc.lpszClassName = "display-ui-windowclass";
	wc.hIcon  = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_ICON1));
	
	RegisterClassEx(&wc);
	
	debug_init();
	config_load();

	config_generate_sig();

	weather_init();

#ifdef FULLSCREEN
	
	// Set screen to specified resolution if program is compiled to run in fullscreen mode
	DEVMODE dmScreenSettings;            
	memset(&dmScreenSettings, 0, sizeof(dmScreenSettings));
	dmScreenSettings.dmSize = sizeof(dmScreenSettings);   
	dmScreenSettings.dmPelsWidth = SCREEN_WIDTH;      
	dmScreenSettings.dmPelsHeight = SCREEN_HEIGHT;         
	dmScreenSettings.dmBitsPerPel = 32;      
	dmScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;
	
	if (ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL){
		MessageBox(NULL, "Could not change screen resolution!", "Error", MB_ICONERROR);
	}

	hWnd = CreateWindowEx(0,
		"display-ui-windowclass",
		"Display UI Client", // perhaps could be customizable through Registry
		WS_POPUP | WS_VISIBLE,    // fullscreen values
		0, 0,//-GetSystemMetrics(SM_CXFRAME), -GetSystemMetrics(SM_CYFRAME),
		//-GetSystemMetrics(SM_CXBORDER) -GetSystemMetrics(SM_CXEDGE), - GetSystemMetrics(SM_CYBORDER) - GetSystemMetrics(SM_CYEDGE),
		SCREEN_WIDTH,// + 2*GetSystemMetrics(SM_CXFRAME),
		SCREEN_HEIGHT,// + 2*GetSystemMetrics(SM_CYFRAME),
		NULL,
		NULL,
		hInstance,
		NULL);
#endif
#ifndef FULLSCREEN
	hWnd = CreateWindowEx(0,
		"display-ui-windowclass",
		"Display UI Client", // perhaps could be customizable through Registry
		WS_POPUPWINDOW | WS_VISIBLE,    // fullscreen values
		GetSystemMetrics(SM_CXSCREEN)/2 - SCREEN_WIDTH/2, GetSystemMetrics(SM_CYSCREEN)/2 - SCREEN_HEIGHT/2,
		SCREEN_WIDTH,// + GetSystemMetrics(SM_CXFRAME) + 2*(GetSystemMetrics(SM_CXEDGE)),
		SCREEN_HEIGHT,// + GetSystemMetrics(SM_CYSIZEFRAME) + 2*(GetSystemMetrics(SM_CYEDGE)) + GetSystemMetrics(SM_CYCAPTION),
		NULL,
		NULL,
		hInstance,
		NULL);
#endif
	
	// Load the old headline data to display in case the server is down
	load_headlines();
	
	ShowWindow(hWnd, nCmdShow);
	
	// Update playlist and weather/headlines separately to avoid lag
	CloseHandle(CreateThread(NULL, 0x2000, (unsigned long (__stdcall *)(void *))playlist_load, (void *)hWnd, 0, NULL));
	CloseHandle(CreateThread(NULL, 0x2000, (unsigned long (__stdcall *)(void *))update, (void *)hWnd, 0, NULL));

	// enter the main loop:

	MSG msg;

	while(GetMessage(&msg, NULL, 0, 0) > 0) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}


	return msg.wParam;
}


LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{

	HDC          hdcMem = NULL;
	HBITMAP      hbmMem = NULL;
	HANDLE       hOld   = NULL;
	


	switch (message) {
	case WM_CREATE:{
		FreeImage_Initialise(true);
		SetTimer(hWnd, 1, 1000, NULL); // header
		SetTimer(hWnd, 2, 900000, NULL); // weather
		SetTimer(hWnd, 3, FRAME_INTERVAL, NULL); // content
		
#if SCREEN_WIDTH == 1366
		g_fbmp_header			= FreeImage_Load(FIF_JPEG, "img\\header-1366.jpg", JPEG_DEFAULT);
		g_fbmp_bg				= FreeImage_Load(FIF_JPEG, "img\\content-bg-1366.jpg", JPEG_DEFAULT);
		g_fbmp_headlines2		= FreeImage_Load(FIF_JPEG, "img\\headlines-2-1366.jpg", JPEG_DEFAULT);
#else
		g_fbmp_header			= FreeImage_Load(FIF_JPEG, "img\\header.jpg", JPEG_DEFAULT);
		g_fbmp_bg				= FreeImage_Load(FIF_JPEG, "img\\content-bg.jpg", JPEG_DEFAULT);
		g_fbmp_headlines2		= FreeImage_Load(FIF_JPEG, "img\\headlines-2.jpg", JPEG_DEFAULT);
#endif		
		g_fbmp_weatherbg		= FreeImage_Load(FIF_PNG, "img\\partlycloudy.png", PNG_DEFAULT);
		g_fbmp_weathergrad		= FreeImage_Load(FIF_PNG, "img\\weathergrad.png", PNG_DEFAULT);
		g_fbmp_headlines1		= FreeImage_Load(FIF_JPEG, "img\\headlines-1.jpg", JPEG_DEFAULT);
		g_fbmp_loading			= FreeImage_Load(FIF_PNG, "img\\loading.png", PNG_DEFAULT);

		// Initialize global fonts to use throughout application
		g_font_dejavusans_bold_20 = MakeFont("DejaVu Sans", true, false, false, 20);
		g_font_dejavusans_bold_24 = MakeFont("DejaVu Sans", true, false, false, 24);
		g_font_dejavusans_cond_12 = MakeFont("DejaVu Sans Condensed", false, false, false, 12);
		g_font_dejavusans_cond_18 = MakeFont("DejaVu Sans Condensed", false, false, false, 18);
		g_font_dejavusans_cond_bold_24 = MakeFont("DejaVu Sans Condensed", true, false, false, 24);
		g_font_dejavusans_cond_bold_32 = MakeFont("DejaVu Sans Condensed", true, false, false, 32);
		g_font_dejavusans_cond_bold_42 = MakeFont("DejaVu Sans Condensed", true, false, false, 42);
		g_font_dejavusans_cond_bold_58 = MakeFont("DejaVu Sans Condensed", true, false, false, 58);
		
		// Loading image is a PNG with alpha-transparency
		FreeImage_PreMultiplyWithAlpha(g_fbmp_loading);

		break;
		}
	case WM_PAINT: {
		PAINTSTRUCT ps;
		BeginPaint(hWnd, &ps);
		
		// Create HDC and bitmap for double-buffering
		hdcMem = CreateCompatibleDC(ps.hdc);
		hbmMem = CreateCompatibleBitmap(ps.hdc, SCREEN_WIDTH, SCREEN_HEIGHT);
		hOld   = SelectObject(hdcMem, hbmMem);
		
		// White text with transparent background
		SetBkMode(hdcMem, TRANSPARENT); 
		SetTextColor(hdcMem, RGB(255, 255, 255));
		
		// Paint only the region that was invalidated (i.e. the part of the screen that
		// will be actually be updated). CPU usage drops by about 10% as a result.
		if(ps.rcPaint.top == CONTENT_BOTTOM){
			repaint_headlines(hdcMem, ps);
		}else if(ps.rcPaint.bottom == CONTENT_TOP){
			repaint_header(hdcMem, ps);
		}else if(ps.rcPaint.top == CONTENT_TOP && ps.rcPaint.left == 0){
			repaint_content(hWnd, hdcMem, ps);
			repaint_headlines(hdcMem, ps);
		}else{
			repaint_weather(hdcMem, ps);
			repaint_header(hdcMem, ps);
		}	
		
		// Need to call RepaintVideo regularly to make sure that DirectShow renders it
		// correctly. However, the window must NOT repaint that region (i.e. never invalidate
		// the containing rectangle). While the video is playing, the content are rect is
		// not invalidated, but repaint_content must be called because it tells DShow to
		// render the video frames.
		if(g_video_painting){
			repaint_content(hWnd, hdcMem, ps);
		}
		
		BitBlt(ps.hdc, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, hdcMem, 0, 0, SRCCOPY);
		SelectObject(hdcMem, hOld);
		DeleteObject(hbmMem);
		DeleteDC    (hdcMem);
		
		EndPaint(hWnd, &ps);
		break;
				   }
	case WM_TIMER:
		if(wParam == 1){
			// Redraw the header once per second
			RECT rt;
			static int secs = 0;
			rt.top = 0; rt.left = 0; rt.right = DISPLAY_WIDTH; rt.bottom = CONTENT_TOP;
			InvalidateRect(hWnd, &rt, false);
			
			if(secs++ % (FRAMES_PER_SEC * 5) == 0){
				rt.top = CONTENT_TOP; rt.left = CONTENT_WIDTH; rt.right = DISPLAY_WIDTH; rt.bottom = CONTENT_BOTTOM;
				InvalidateRect(hWnd, &rt, false);
			}

			return 0;
		}else if(wParam == 2){
			// Update weather & headlines
			CloseHandle(CreateThread(NULL, 0x2000, (unsigned long (__stdcall *)(void *))update, (void *)hWnd, 0, NULL));
			CloseHandle(CreateThread(NULL, 0x2000, (unsigned long (__stdcall *)(void *))update_playlist, (void *)hWnd, 0, NULL));
			//weather_update();
			return 0;
		}else if(wParam == 3){
			// Repaint the content area and headlines
			RECT rt, rt2;
			rt.top = CONTENT_TOP; rt.left = 0; rt.right = CONTENT_WIDTH; rt.bottom = CONTENT_BOTTOM;
			
			// Do NOT invalidate content area rect if video is playing to avoid flickering
			if(!g_video_painting) 
				InvalidateRect(hWnd, &rt, false);

			rt2.top = CONTENT_BOTTOM; rt2.left = 0; rt2.right = DISPLAY_WIDTH; rt2.bottom = SCREEN_HEIGHT;
			InvalidateRect(hWnd, &rt2, false);

			g_headline_x -= 2;
			g_ticks++;
			return 0;}
	case WM_KEYDOWN:
		// Quit on esc or Q; skip current playlist item on space
		if(wParam == VK_ESCAPE || wParam == 'q' || wParam == 'Q'){
			SendMessage(hWnd, WM_DESTROY, 0, 0);
		}else if(wParam == VK_SPACE){
			g_goto_next = true;
		}else if(wParam == VK_F5){
			CloseHandle(CreateThread(NULL, 0x2000, (unsigned long (__stdcall *)(void *))update, (void *)hWnd, 0, NULL));
			CloseHandle(CreateThread(NULL, 0x2000, (unsigned long (__stdcall *)(void *))update_playlist, (void *)hWnd, 0, NULL));
		}
		break;
	//case WM_ERASEBKGND:
		
	//	break;
	case WM_DESTROY:
		
		// Unload everything when program is shut down.
		playlist_unload();
		
		debug_print("[msg loop] playlist unloaded!\n");

		FreeImage_Unload(g_fbmp_bg);
		FreeImage_Unload(g_fbmp_header);
		FreeImage_Unload(g_fbmp_weatherbg);
		FreeImage_Unload(g_fbmp_weathergrad);
		FreeImage_Unload(g_fbmp_headlines1);
		FreeImage_Unload(g_fbmp_headlines2);
		FreeImage_Unload(g_fbmp_loading);
		FreeImage_DeInitialise();

		DeleteObject(g_font_dejavusans_bold_20);
		DeleteObject(g_font_dejavusans_bold_24);
		DeleteObject(g_font_dejavusans_cond_12);
		DeleteObject(g_font_dejavusans_cond_18);
		DeleteObject(g_font_dejavusans_cond_bold_24);
		DeleteObject(g_font_dejavusans_cond_bold_32);
		DeleteObject(g_font_dejavusans_cond_bold_42);
		DeleteObject(g_font_dejavusans_cond_bold_58);

		debug_print("[msg loop] fonts and graphics unloaded!\n");
		debug_close();
		debug_print("[msg loop] debug unloaded!\n");
		weather_exit();
		debug_print("[msg loop] weather unloaded!\n");
		video_shutdown();
		debug_print("[msg loop] video unloaded!\n");
		//bmp->~Bitmap();
		//content_bg->~Image();
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// Repaints the main content area in which media/slideshows are displayed.
//		hWnd:		A handle to the window in which the drawing is being done.
//		hdcMem:		The HDC to which to render. This should be the off-screen HDC created
//					in the WM_PAINT handler for double-buffering. 
//		ps:			The PAINTSTRUCT obtained from a call to BeginPaint in the WM_PAINT handler.
void repaint_content(HWND hWnd, HDC hdcMem, PAINTSTRUCT ps){
	
	static int timeleft = 0xffff;
	static int imgalpha = 205;
	
	// Draw background image
	StretchDIBits(hdcMem, 0, CONTENT_TOP, CONTENT_WIDTH, CONTENT_HEIGHT, 
			0, 0, CONTENT_WIDTH, CONTENT_HEIGHT, FreeImage_GetBits(g_fbmp_bg), 
			FreeImage_GetInfo(g_fbmp_bg), DIB_RGB_COLORS, SRCCOPY);
	
	// Because of how playlists are loaded in a thread, there is no way of knowing
	// exactly when the playlists will finish loading, so this must be done to ensure
	// that as soon as they finish, this global variable is set to the first item
	if(!g_current_elem){
		g_current_elem = g_playlist.first;
	}

	if(g_current_elem){
		// "ready" is set when the media item is completely finished loading. This is
		// done to avoid attempts to draw, for example, an image that has only been
		// downloaded halfway.
		if(g_current_elem->ready){
			if(g_current_elem->type == 1){
				image_element_t *image = (image_element_t *) g_current_elem->data;

				// timeleft is set to 0xffff when the previous item has finished its
				// display period, and upon initialization of the program. This is to 
				// make sure that the items are actually displayed for the correct
				// amount of time.
				if(timeleft == 0xffff) timeleft = g_current_elem->secs * FRAMES_PER_SEC;

				// The AlphaBlend function uses a BLENDFUNCTION struct to specify properties
				// of alpha blending (such as the alpha value for the entire image). This is
				// controlled here using the imgalpha value to allow for fade in/out effects
				// . And I refuse to put a period on the above line because this text aligns
				// . And that is so cool; that line aligned as well. But okay I'll stop now.
				image->bf.SourceConstantAlpha = imgalpha;

				// For efficiency reasons, images are drawn once to an HDC which is created 
				// during playlist initialization. All that needs to be done during redraws
				// is this call to AlphaBlend to paint the image onto the window's HDC.
				AlphaBlend(hdcMem, CONTENT_WIDTH/2 - image->width/2, CONTENT_TOP + CONTENT_HEIGHT/2 - image->height/2, 
					image->width, image->height, image->hdc, 0, 0, image->width, image->height, image->bf);
				
				// If the user pressed space to skip an item, do the fadeout during the next
				// 15 redraws, and reset the goto-next flag to false.
				if(g_goto_next){
					timeleft = 15;
					g_goto_next = false;
				}
				
				if(timeleft == 0){
					// Reached the end of the current item's timeframe. Go to the next one.
					if(g_current_elem->next){
						g_current_elem = g_current_elem->next;
						if(g_current_elem == g_playlist.first) playlist_update(hWnd);
					}
					debug_print("[repaint_content - image] switched to playlist item @ 0x%08lX\n", g_current_elem);

					// Draw the background image. Don't remember why this is necessary, but there's probably a reason.
					// Otherwise why would this be here? -_-
					StretchDIBits(hdcMem, 0, CONTENT_TOP, CONTENT_WIDTH, CONTENT_HEIGHT, 0, 0, CONTENT_WIDTH, CONTENT_HEIGHT, 
						FreeImage_GetBits(g_fbmp_bg), FreeImage_GetInfo(g_fbmp_bg), DIB_RGB_COLORS, SRCCOPY);

					// Reset to default values
					timeleft = 0xffff;
					imgalpha = 55;
				}else{
					// Didn't reach zero yet, so update time and alpha counters.
					// Time always decreases.
					timeleft--;
					// Decrease alpha during last 15 iterations. Otherwise increase if it isn't 255 (opaque)
					if(timeleft < 15){
						imgalpha -= 15;
					}else if(imgalpha < 255) imgalpha += 10;
				}
				
			}else if(g_current_elem->type == 3){
#ifdef SKIPVIDEO
				if(g_current_elem->next) g_current_elem = g_current_elem->next;
#endif
#ifndef SKIPVIDEO
				static int iter = 0;
				LONGLONG position;
				LONGLONG duration;

				video_element_t *video = (video_element_t *) g_current_elem->data;
				
				if(iter == 0){
					// Load the video if it is not yet loaded.
					debug_print("[repaint_content - video] Loading video...\n");
					video_load(hWnd, video);
					g_video_painting = true;
				}
				if(video_isloaded()){
					// Must tell DShow to render it every time.
					video_render(hWnd, hdcMem, video);
				
					video_getduration(&duration);
					video_getposition(&position);
					iter++;

					if(position >= duration || g_goto_next){
						// Reached video end, or user pressed space to skip the video.
						g_goto_next = false;
						debug_print("[repaint_content - video] Video end reached.\n");
						video_unload(video);
						g_video_painting = false;
						iter = 0;
						if(g_current_elem->next){
							g_current_elem = g_current_elem->next;
							if(g_current_elem == g_playlist.first) playlist_update(hWnd);
						}
						debug_print("[repaint_content - video] switched to playlist item @ 0x%08lX\n", g_current_elem);
					}
				}
#endif
			}
		}else{
			// If "ready" isn't set to true, media is still loading. Show the "loading" image.
			char msg[64];
			static int t = 0;
			
			// Need to draw it to another HDC to make it possible to do alpha blending.
			HDC hdcTemp = CreateCompatibleDC(hdcMem);
			HBITMAP hbmTemp = CreateCompatibleBitmap(ps.hdc, 400, 225);
			SelectObject(hdcTemp, hbmTemp);
			
			BLENDFUNCTION bf;
			bf.BlendOp = AC_SRC_OVER;
			bf.BlendFlags = 0;
			bf.SourceConstantAlpha = 255;
			bf.AlphaFormat = AC_SRC_ALPHA;

			StretchDIBits(hdcTemp, 0, 0, 400, 225, 0, 0, 400, 225, FreeImage_GetBits(g_fbmp_loading), FreeImage_GetInfo(g_fbmp_loading), DIB_RGB_COLORS, SRCCOPY);
			AlphaBlend(hdcMem, CONTENT_WIDTH/2 - 200, CONTENT_TOP + CONTENT_HEIGHT/2 - 225/2, 400, 225, hdcTemp, 0, 0, 400, 225, bf);
			
			if(g_current_elem->type == 1){
				// Print progress message for images.
				image_element_t *img = (image_element_t *)g_current_elem->data;
				if(g_current_elem->progress_total){
					sprintf(msg, "Progress: %d%%\nFile: %s\n%d/%d bytes.", (int)(g_current_elem->progress_now/g_current_elem->progress_total * 100), 
						img->filename, (int)g_current_elem->progress_now, (int)g_current_elem->progress_total);
				}else{
					// Avoid division by zero if total is unknown.
					sprintf(msg, "File: %s\n%d bytes downloaded.", img->filename, (int)g_current_elem->progress_now, (int)g_current_elem->progress_total);
				}
				
			}else if(g_current_elem->type == 3){
				// Print progress message for videos.
				video_element_t *vid = (video_element_t *)g_current_elem->data;
				if(g_current_elem->progress_total){
					sprintf(msg, "Progress: %d%%\nFile: %s\n%d/%d bytes.", (int)(g_current_elem->progress_now/g_current_elem->progress_total * 100), 
						vid->filename, (int)g_current_elem->progress_now, (int)g_current_elem->progress_total);
				}else{
					// Avoid division by zero if total is unknown.
					sprintf(msg, "File: %s\n%d bytes downloaded.", vid->filename, (int)g_current_elem->progress_now, (int)g_current_elem->progress_total);
				}
				
			}else{
				sprintf(msg, "%d bytes downloaded.", (int)g_current_elem->progress_now);
			}

			SelectObject(hdcMem, g_font_dejavusans_cond_bold_24);
			
			RECT textBound;
			textBound.left = CONTENT_WIDTH/2 - 200 + 20; textBound.top = CONTENT_TOP + CONTENT_HEIGHT/2 - 225/2 + 95;
			textBound.right = CONTENT_WIDTH/2 + 190; textBound.bottom = CONTENT_TOP + CONTENT_HEIGHT/2 + 395/2;
			DrawTextA(hdcMem, msg, strlen(msg), &textBound, DT_LEFT);

			DeleteObject(hbmTemp);
			DeleteDC(hdcTemp);

			if(t++ / FRAMES_PER_SEC >= 5){
				// 5 seconds and the playlist item wasn't loaded; move on to next one
				if(g_current_elem->next){
					g_current_elem = g_current_elem->next;
					if(g_current_elem == g_playlist.first) playlist_update(hWnd);
				}
				t = 0;
			}
		}
	}
}

// Paint the weather panel.
//		hdcMem:		The HDC to which to render. This should be the off-screen HDC created
//					in the WM_PAINT handler for double-buffering. 
//		ps:			The PAINTSTRUCT obtained from a call to BeginPaint in the WM_PAINT handler.
void repaint_weather(HDC hdcMem, PAINTSTRUCT ps){
	
	char *days_abbr[7] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
	tm *ptm;
	time_t rawtime;
	
	time (&rawtime);
	ptm = localtime(&rawtime);

	// Get current weather data.
	weather_t *current = weather_getcurrent();

	if(current){
		FIBITMAP *fbmp_weather_cur = weather_getimage_current();

		StretchDIBits(hdcMem, CONTENT_WIDTH, CONTENT_TOP - 1, 300, 225, 0, 0, 300, 225, 
			FreeImage_GetBits(g_fbmp_weatherbg), FreeImage_GetInfo(g_fbmp_weatherbg), DIB_RGB_COLORS, SRCCOPY);

		FreeImage_SetTransparent(fbmp_weather_cur, true);

		
		// Draw current weather icon if it is available
		if(fbmp_weather_cur){
			HDC temp = CreateCompatibleDC(hdcMem);
			HBITMAP temp_bmp = CreateCompatibleBitmap(ps.hdc, 250, 180);
			SelectObject(temp, temp_bmp);

			BLENDFUNCTION bf;
			bf.BlendOp = AC_SRC_OVER;
			bf.BlendFlags = 0;
			bf.SourceConstantAlpha = 255;
			bf.AlphaFormat = AC_SRC_ALPHA;
			
			StretchDIBits(temp, 0, 0, 250, 180, 0, 0, 250, 180, FreeImage_GetBits(fbmp_weather_cur), 
				FreeImage_GetInfo(fbmp_weather_cur), DIB_RGB_COLORS, SRCCOPY);

			AlphaBlend(hdcMem, CONTENT_WIDTH + 10, CONTENT_TOP + 8, 250, 180, temp, 0, 0, 250, 180, bf);
			DeleteObject(temp_bmp);
			DeleteDC(temp);
		}
		
		
		// Display current temperature
		RECT textBound;
		textBound.left = CONTENT_WIDTH + 30; textBound.top = CONTENT_TOP + 8;
		textBound.right = DISPLAY_WIDTH - 20; textBound.bottom = CONTENT_TOP + 68;
		SelectObject(hdcMem, g_font_dejavusans_cond_bold_42);
		DrawTextA(hdcMem, current->temp, strlen(current->temp), &textBound, DT_RIGHT);
		
		// Display current weather description or loading message if it isn't done loading
		textBound.left = CONTENT_WIDTH - 5; textBound.top = CONTENT_TOP + 148;
		textBound.right = CONTENT_WIDTH - 5 + 295; textBound.bottom = CONTENT_TOP + 148 + 75;
		SelectObject(hdcMem, g_font_dejavusans_cond_bold_32);

		if(current->description){
			DrawTextA(hdcMem, current->description, strlen(current->description), &textBound, DT_RIGHT | DT_WORDBREAK);
		}else{
			DrawTextA(hdcMem, "loading weather data...", strlen("loading weather data..."), &textBound, DT_RIGHT | DT_WORDBREAK);
		}
		
	}

	// Get weather forecast data.	
	weather_fc_t **forecast = weather_getforecast();
	
	if(forecast){
		FIBITMAP *fbmp_weather_fc0 = weather_getimage_fc(0);
		FIBITMAP *fbmp_weather_fc1 = weather_getimage_fc(1);
		
		// Draw gradient background for forecast area in weather panel (because GDI can't do gradients).
		StretchDIBits(hdcMem, CONTENT_WIDTH, CONTENT_TOP + 223, 300, 150, 0, 0, 300, 150, FreeImage_GetBits(g_fbmp_weathergrad), FreeImage_GetInfo(g_fbmp_weathergrad), DIB_RGB_COLORS, SRCCOPY);
		
		SelectObject(hdcMem, g_font_dejavusans_bold_24);
		RECT textBound;
		textBound.left = CONTENT_WIDTH; textBound.top = CONTENT_TOP + 228;
		textBound.right = CONTENT_WIDTH + 145; textBound.bottom = CONTENT_TOP + 228 + 30;
		
		// Draw day name for first day
		DrawTextA(hdcMem, days_abbr[ptm->tm_wday == 6 ? 0 : ptm->tm_wday + 1], 
				strlen(days_abbr[ptm->tm_wday == 6 ? 0 : ptm->tm_wday + 1]), &textBound, DT_RIGHT);
		
		textBound.left = CONTENT_WIDTH + 150; textBound.top = CONTENT_TOP + 228;
		textBound.right = CONTENT_WIDTH + 150 + 145; textBound.bottom = CONTENT_TOP + 228 + 30;
		
		// Draw day name for second day. Make sure that day loops back correctly for weekends.
		DrawTextA(hdcMem, days_abbr[ptm->tm_wday == 6 ? 1 : (ptm->tm_wday == 5 ? 0 : ptm->tm_wday + 2)], 
				strlen(days_abbr[ptm->tm_wday == 6 ? 1 : (ptm->tm_wday == 5 ? 0 : ptm->tm_wday + 2)]), &textBound, DT_RIGHT);

		
		// If both weather forecast icons are done loading, draw them
		if(fbmp_weather_fc0 && fbmp_weather_fc1){
			// Create one HDC for entire area onto which both icons will be drawn
			// and draw it onto the main window's HDC all at once.
			HDC temp = CreateCompatibleDC(hdcMem);
			HBITMAP temp_bmp = CreateCompatibleBitmap(ps.hdc, 300, 135);
			SelectObject(temp, temp_bmp);
			
			BLENDFUNCTION bf;
			bf.BlendOp = AC_SRC_OVER;
			bf.BlendFlags = 0;
			bf.SourceConstantAlpha = 255;
			bf.AlphaFormat = AC_SRC_ALPHA;
			
			SetStretchBltMode(temp, COLORONCOLOR);
			
			StretchDIBits(temp, 0, 0, 188, 135, 0, 0, FreeImage_GetWidth(fbmp_weather_fc0), FreeImage_GetHeight(fbmp_weather_fc0), 
				FreeImage_GetBits(fbmp_weather_fc0), FreeImage_GetInfo(fbmp_weather_fc0), DIB_RGB_COLORS, SRCCOPY);
			StretchDIBits(temp, 150, 0, 188, 135, 0, 0, FreeImage_GetWidth(fbmp_weather_fc1), FreeImage_GetHeight(fbmp_weather_fc1), 
				FreeImage_GetBits(fbmp_weather_fc1), FreeImage_GetInfo(fbmp_weather_fc1), DIB_RGB_COLORS, SRCCOPY);

			AlphaBlend(hdcMem, CONTENT_WIDTH, CONTENT_TOP + 228, 300, 135, temp, 0, 0, 300, 135, bf);
			//BitBlt(hdcMem, 990, 180, 1280, 720, temp, 0, 0, SRCCOPY);
			DeleteObject(temp_bmp);
			DeleteDC(temp);
		}
		
		// Draw high and low temps for both forecast days

		char high[8];		
		sprintf(high, "hi %s", forecast[0]->temp_hi);
		textBound.left = CONTENT_WIDTH; textBound.top = CONTENT_TOP + 328;
		textBound.right = CONTENT_WIDTH + 120; textBound.bottom = CONTENT_TOP + 328 + 40;
		SelectObject(hdcMem, g_font_dejavusans_cond_bold_32);
		DrawTextA(hdcMem, high, strlen(high), &textBound, DT_RIGHT);

		sprintf(high, "hi %s", forecast[1]->temp_hi);
		textBound.left = CONTENT_WIDTH + 150; textBound.top = CONTENT_TOP + 328;
		textBound.right = CONTENT_WIDTH + 150 + 120; textBound.bottom = CONTENT_TOP + 328 + 40;
		DrawTextA(hdcMem, high, strlen(high), &textBound, DT_RIGHT);

		SelectObject(hdcMem, g_font_dejavusans_cond_12);
		TextOut(hdcMem, CONTENT_WIDTH + 124, CONTENT_TOP + 331, "low", strlen("low"));
		TextOut(hdcMem, CONTENT_WIDTH + 124 + 150, CONTENT_TOP + 331, "low", strlen("low"));

		textBound.left = CONTENT_WIDTH + 95; textBound.top = CONTENT_TOP + 343;
		textBound.right = CONTENT_WIDTH + 95 + 48; textBound.bottom = CONTENT_TOP + 343 + 20;
		SelectObject(hdcMem, g_font_dejavusans_cond_18);
		DrawTextA(hdcMem, forecast[0]->temp_lo, strlen(forecast[0]->temp_lo), &textBound, DT_RIGHT);

		textBound.left = CONTENT_WIDTH + 95 + 150; textBound.top = CONTENT_TOP + 343;
		textBound.right = CONTENT_WIDTH + 95 + 150 + 48; textBound.bottom = CONTENT_TOP + 343 + 20;
		DrawTextA(hdcMem, forecast[1]->temp_lo, strlen(forecast[1]->temp_lo), &textBound, DT_RIGHT);
	}
}

// Redraw the header (incl. time & date)
// params:
//		hdcMem:		The HDC to which to render. This should be the off-screen HDC created
//					in the WM_PAINT handler for double-buffering. 
//		ps:			The PAINTSTRUCT obtained from a call to BeginPaint in the WM_PAINT handler.
void repaint_header(HDC hdcMem, PAINTSTRUCT ps){
	
	char *days[7] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
	char *days_abbr[7] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
	
	time_t rawtime;
	tm *ptm;
	char datetimestr[17];
	
	time (&rawtime);
	ptm = localtime(&rawtime);
	
	// Draw background image
	StretchDIBits(hdcMem, 0, TOP_SPACER, DISPLAY_WIDTH, CONTENT_TOP - TOP_SPACER, 0, 0, DISPLAY_WIDTH, CONTENT_TOP - TOP_SPACER, 
		FreeImage_GetBits(g_fbmp_header), FreeImage_GetInfo(g_fbmp_header), DIB_RGB_COLORS, SRCCOPY);
	
	// Draw date in format [month day, year]
	strftime(datetimestr, 16, "%b %d, %Y", ptm);
	SetBkMode(hdcMem, TRANSPARENT); 
	SetTextColor(hdcMem, RGB(255, 255, 255));
	SelectObject(hdcMem, g_font_dejavusans_bold_20);
	TextOut(hdcMem, CONTENT_WIDTH - 260, TOP_SPACER + 25, days[ptm->tm_wday], strlen(days[ptm->tm_wday]));
	SelectObject(hdcMem, g_font_dejavusans_cond_bold_32);
	TextOut(hdcMem, CONTENT_WIDTH - 263, TOP_SPACER + 50, datetimestr, strlen(datetimestr));
	
	// Draw time in 24-hr format
	memset(datetimestr, 0, 17);
	strftime(datetimestr, 16, "%H:%M:%S", ptm);
	SelectObject(hdcMem, g_font_dejavusans_cond_bold_58);
	TextOut(hdcMem, CONTENT_WIDTH + 20, TOP_SPACER + 27, datetimestr, strlen(datetimestr));
	
	// Draw line to separate weather panel from content panel
	HPEN pen = CreatePen(PS_DASH, 2, RGB(100, 100, 100));
	SelectObject(hdcMem, pen);
	MoveToEx(hdcMem, CONTENT_WIDTH, CONTENT_TOP, NULL);
	LineTo(hdcMem, CONTENT_WIDTH, CONTENT_BOTTOM);
	DeleteObject(pen);
}

// Repaint moving headlines at bottom of screen
void repaint_headlines(HDC hdcMem, PAINTSTRUCT ps){
	
	// Draw the right part of the background image
	StretchDIBits(hdcMem, 160, CONTENT_BOTTOM, DISPLAY_WIDTH - 160, 56, 0, 0, DISPLAY_WIDTH - 160, 56, 
		FreeImage_GetBits(g_fbmp_headlines2), FreeImage_GetInfo(g_fbmp_headlines2), DIB_RGB_COLORS, SRCCOPY);
	
	// Draw text if it exists
	if(g_headline_txt){
		SelectObject(hdcMem, g_font_dejavusans_cond_bold_24);
		TextOut(hdcMem, g_headline_x, CONTENT_BOTTOM + 16, g_headline_txt, strlen(g_headline_txt));
		
		// If the right edge of the text is past the left side of the screen, start again at the right side.
		SIZE extents;
		GetTextExtentPoint32(hdcMem, g_headline_txt, strlen(g_headline_txt), &extents);
		if(g_headline_x + extents.cx < 160) g_headline_x = DISPLAY_WIDTH;
	}
	
	// Draw the left part of the background image. This is done to create the effect of the headlines
	// scrolling *under* the "headlines" part of the background image.
	StretchDIBits(hdcMem, 0, CONTENT_BOTTOM, 160, 56, 0, 0, 160, 56, FreeImage_GetBits(g_fbmp_headlines1), FreeImage_GetInfo(g_fbmp_headlines1), DIB_RGB_COLORS, SRCCOPY);
}

// Create a URL to be used with the display-ui API. Allows for formatting with different servers;
// includes system name and signature into URL as required by the server. 
// params:
//		dest:	The destination buffer which will hold the full URL. This must be an allocated block 
//				of memory of sufficient size. The directory must exist.
//		url:	The incomplete URL to be formatted. If it has additional parameters, it should not 
//				end in &. If it does not have additional params, it should not end in ? as it will be appended. 
//				Examples:	/api/weather/current/?location=CAXX0401    -- with additional params
//							/api/playlists/fetch/					   -- without additional params
void make_url(char *dest, char *url){
	if(strchr(url, '?') == NULL){
		sprintf(dest, "%s%s?sys_name=%s&sig=%s", config_get_url(), url, config_get_sysname(), config_get_sig());
	}else{
		sprintf(dest, "%s%s&sys_name=%s&sig=%s", config_get_url(), url, config_get_sysname(), config_get_sig());
	}
}

// Write callback for cURL to save downloaded data to a file.
// stream must be a valid HANDLE set with CURLOPT_WRITEDATA
size_t writefunc(void *ptr, size_t size, size_t nmemb, void *stream){
	unsigned long ret = 0;
	WriteFile((HANDLE)stream, ptr, size * nmemb, &ret, NULL);
	return ret;
}

// Download a file to disk using cURL.
// params:
//		url:		A pointer to a null-terminated string containing the HTTP URL from which to download.
//		dest_file:	A pointer to a null-terminated string containing the destination filename. The 
//					file's directory must exist; however, the file itself need not exist and will be
//					overwritten if it does exist.
//		show_prgrs:	Specifies whether to call the downlaod-progress callback. If this value is TRUE,
//					callback *MUST* point to a valid progress callback function.
//		callback:	If show_progress is TRUE, must be a valid progreess callback function, which defined as:
//					int progress_cb(void *arg, double dltotal, double dlnow, double ultotal, double ulnow);
//					This parameter is ignored if show_progress is FALSE.
//		arg:		Parameter that is passed to the callback function. Can be NULL if the callback does not
//					use it. This parameter is ignored if show_progress is FALSE.
int download_curl(char *url, char *dest_file, bool show_progress, void *callback, void *arg){
	CURL *curl;
	CURLcode res = (CURLcode)-1;

	curl = curl_easy_init();

	if(curl){
		// Use Win32 APIs for writing because fprintf was doing weird stuff
		HANDLE file = CreateFile(dest_file, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if(file){
			
			debug_print("[download_curl] URL: %s\n", url);
			debug_print("[download_curl] dest file: %s\n", dest_file);
			
			// Configure cURL to download from the given URL
			curl_easy_setopt(curl, CURLOPT_URL, url);
			curl_easy_setopt(curl, CURLOPT_USERAGENT, config_get_useragent());
			curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 5);
			curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
			
			// Tell cURL to call the progress callback if it is set
			if(show_progress){
				curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0);
				curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, callback);
				curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, arg);
			}	
			
			curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writefunc);
			curl_easy_setopt(curl, CURLOPT_WRITEDATA, file);
			res = curl_easy_perform(curl);

			if(res != CURLE_OK) debug_print("[download_curl] error %d\n", res);

			CloseHandle(file);
			curl_easy_cleanup(curl);
			
		}else{
			curl_easy_cleanup(curl);
			debug_print("[download_curl] failed to open file for writing!\n");
			return -1;
		}
		
	}else{
		debug_print("[download_curl] failed to initialize cURL!\n");
		return -1;
	}
	
	return res;
}
// Download a file to disk using cURL. Saves to a .tmp file until download is complete
// to avoid data corruption from incomplete downloads.
// params:
//		url:		A pointer to a null-terminated string containing the HTTP URL from which to download.
//		dest_file:	A pointer to a null-terminated string containing the destination filename. The 
//					file's directory must exist; however, the file itself need not exist and will be
//					overwritten if it does exist.
int download_curl(char *url, char *dest_file){
	char *temp_dest = (char *) malloc(strlen(dest_file) + 5);
	sprintf(temp_dest, "%s.tmp", dest_file);
	
	int ret = download_curl(url, temp_dest, false, NULL, NULL);
	
	// Move from the .tmp to the actual destination file if the download was successful
	if(ret == CURLE_OK){
		MoveFileEx(temp_dest, dest_file, MOVEFILE_REPLACE_EXISTING);
	}
	
	free(temp_dest);

	return ret;
}

// Creates valid DUI-API-compatible URL from an incomplete URL and downloads to disk.
// Saves to temp file before overwriting destination file. Use this for downloading from the DUI server.
// params:
//		dest:	The destination buffer which will hold the full URL. This must be an allocated block 
//				of memory of sufficient size. The directory must exist.
//		url:	The incomplete URL to be formatted. If it has additional parameters, it should not 
//				end in &. If it does not have additional params, it should not end in ? as it will be appended. 
//				Examples:	/api/weather/current/?location=CAXX0401    -- with additional params
//							/api/playlists/fetch/					   -- without additional params
//		show_prgrs:	Specifies whether to call the downlaod-progress callback. If this value is TRUE,
//					callback *MUST* point to a valid progress callback function.
//		callback:	If show_progress is TRUE, must be a valid progreess callback function, which defined as:
//					int progress_cb(void *arg, double dltotal, double dlnow, double ultotal, double ulnow);
//					This parameter is ignored if show_progress is FALSE.
//		arg:		Parameter that is passed to the callback function. Can be NULL if the callback does not
//					use it. This parameter is ignored if show_progress is FALSE.
int dui_download(char *url, char *dest_file, bool show_progress, void *callback, void *arg){

	char *full_url = (char *) malloc(strlen(config_get_url()) + strlen(url) + strlen("&sys_name=&sig=")
									+ strlen(config_get_sysname()) + strlen(config_get_sig()) + 1);
	char *temp_dest = (char *) malloc(strlen(dest_file) + 5);
	make_url(full_url, url);
	sprintf(temp_dest, "%s.tmp", dest_file);

	int ret = download_curl(full_url, temp_dest, show_progress, callback, arg);

	if(ret == CURLE_OK){
		MoveFileEx(temp_dest, dest_file, MOVEFILE_REPLACE_EXISTING);
	}

	if(full_url) free(full_url);
	if(temp_dest) free(temp_dest);

	return ret;
}

// Creates valid DUI-API-compatible URL from an incomplete URL and downloads to disk.
// Use this for downloading from the DUI server. Assumes no progress callback.
// params:
//		dest:	The destination buffer which will hold the full URL. This must be an allocated block 
//				of memory of sufficient size. The directory must exist.
//		url:	The incomplete URL to be formatted. If it has additional parameters, it should not 
//				end in &. If it does not have additional params, it should not end in ? as it will be appended. 
//				Examples:	/api/weather/current/?location=CAXX0401    -- with additional params
//							/api/playlists/fetch/					   -- without additional params
int dui_download(char *url, char *dest_file){
	return dui_download(url, dest_file, false, NULL, NULL);
}

// Creates valid DUI-API-compatible URL from an incomplete URL and downloads to disk.
// Retries indefinitely if the download operation failed. 
// Use this for downloading from the DUI server. Assumes no progress callback.
// params:
//		dest:	The destination buffer which will hold the full URL. This must be an allocated block 
//				of memory of sufficient size. The directory must exist.
//		url:	The incomplete URL to be formatted. If it has additional parameters, it should not 
//				end in &. If it does not have additional params, it should not end in ? as it will be appended. 
//				Examples:	/api/weather/current/?location=CAXX0401    -- with additional params
//							/api/playlists/fetch/					   -- without additional params
int dui_download_retry(char *url, char *dest_file){
	int result = dui_download(url, dest_file, false, NULL, NULL);
	while(result != CURLE_OK){
		result = dui_download(url, dest_file, false, NULL, NULL);
	}
	return result;
}
