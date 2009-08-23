/**
 * rhhstv.cpp, the entry point for the application
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
//#define CURL_STATICLIB

#include <windows.h>
#include <urlmon.h>
#include <wchar.h>
#include <stdio.h>
#include <time.h>

#include <Qedit.h>
#include <dshow.h>

#include <curl\curl.h>


// because dshow.h does REALLY stupid stuff
// (see line 7673 of strsafe.h. WTF, microsoft)
#undef sprintf
#undef strcat
#define sprintf sprintf
#define strcat strcat

#include "resource.h"
#include "display_ui.h"
#include "freeimage.h"
#include "debug.h"
#include "weather.h"
#include "playlist.h"
#include "video.h"
#include "sha1.h"


//#pragma comment(lib, "urlmon.lib") 
#pragma comment(lib, "freeimage.lib") 
#pragma comment(lib, "msimg32.lib")
#pragma comment(lib, "strmiids.lib")
#pragma comment(lib, "libcurl.lib")
#pragma comment(lib, "ws2_32")
#pragma comment(lib, "winmm")

#define FRAMES_PER_SEC 25 // should be a factor of 1000!
#define FRAME_INTERVAL 1000 / FRAMES_PER_SEC

#define MakeFont(name, bold, italic, underline, size) CreateFont(-size, \
			0, 0, 0, (bold ? FW_BOLD : FALSE), (italic ? FW_BOLD : FALSE), (underline ? FW_BOLD : FALSE), FALSE, ANSI_CHARSET, \
			OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_ROMAN, name)


LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
void load_headlines();

LPVOID pHeaderImage;

char *headline_txt = "The quick brown fox jumps over the lazy dog | Lorem ipsum dolor sit amet, consectetur adipiscing elit. Nulla lobortis hendrerit hendrerit.";

int g_ticks = 0;
int g_headline_x = 1280;
bool g_video_painting = false;

char *sig; // request signature (api key + date)

extern playlist_t g_playlist;
playlist_element_t *g_current_elem;
void *bmp_bytes;

void update(void *p) {
	weather_update(p, true);

	if(dui_download("http://du.geekie.org/server/api/headlines/fetch/?sys_name=1&sig=%s", "data\\headlines.dat") == CURLE_OK) {
		load_headlines();
	}
}

void load_headlines(){
	FILE *fp = fopen("data\\headlines.dat", "r");
	
	if(fp) {
		if(headline_txt) {
			free(headline_txt);
		}
		fseek(fp, 0, SEEK_END);
		int size = ftell(fp);
		fseek(fp, 0, SEEK_SET);
		
		
		headline_txt = (char *) malloc(size + 2);
		memset(headline_txt, 0, size + 2);
		fread(headline_txt, 1, size, fp);
		
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
	wc.lpszClassName = "WindowClass1";
	wc.hIcon  = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_ICON1));
	
	RegisterClassEx(&wc);
	
	debug_init();
	weather_init();
	
	sig = (char *) malloc(41);
	generate_sig();


	hWnd = CreateWindowEx(WS_EX_CLIENTEDGE,
		"WindowClass1",
		"Display UI Client", // perhaps could be customizable through Registry
		WS_OVERLAPPEDWINDOW,    // fullscreen values
		200, 200,
		/* use this for full-screen:
		-GetSystemMetrics(SM_CXFRAME), -GetSystemMetrics(SM_CYCAPTION) - 
			GetSystemMetrics(SM_CYFRAME),    // the starting x and y positions should be 0 */
		SCREEN_WIDTH + 2*(GetSystemMetrics(SM_CXFRAME) + GetSystemMetrics(SM_CXEDGE)),
		SCREEN_HEIGHT + 2 * (GetSystemMetrics(SM_CYSIZEFRAME) + GetSystemMetrics(SM_CYEDGE)) + GetSystemMetrics(SM_CYCAPTION),    // 760 for now to account for title bar
		NULL,
		NULL,
		hInstance,
		NULL);
	
	weather_update(hWnd, false);
	load_headlines();

	ShowWindow(hWnd, nCmdShow);

	//playlist_load(hWnd);
	
	CreateThread(NULL, 0x2000, (unsigned long (__stdcall *)(void *))playlist_load, (void *)hWnd, 0, NULL);
	CreateThread(NULL, 0x2000, (unsigned long (__stdcall *)(void *))update, (void *)hWnd, 0, NULL);

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
	
	COLORREF color_white = RGB(255, 255, 255);

	static FIBITMAP *fbmp_bg			= NULL;
	static FIBITMAP *fbmp_header		= NULL;
	static FIBITMAP *fbmp_weatherbg		= NULL;
	static FIBITMAP *fbmp_weathergrad	= NULL;
	static FIBITMAP *fbmp_headlines1	= NULL;
	static FIBITMAP *fbmp_headlines2	= NULL;
	static FIBITMAP *fbmp_loading		= NULL;


	switch (message) {
	case WM_CREATE:{
		FreeImage_Initialise(true);
		SetTimer(hWnd, 1, 1000, NULL);
		SetTimer(hWnd, 2, 900000, NULL);
		SetTimer(hWnd, 3, FRAME_INTERVAL, NULL);
		
#if SCREEN_WIDTH == 1366
		fbmp_header			= FreeImage_Load(FIF_JPEG, "img\\header-1366.jpg", JPEG_DEFAULT);
		fbmp_bg				= FreeImage_Load(FIF_JPEG, "img\\content-bg-1366.jpg", JPEG_DEFAULT);
		fbmp_headlines2		= FreeImage_Load(FIF_JPEG, "img\\headlines-2-1366.jpg", JPEG_DEFAULT);
#else
		fbmp_header			= FreeImage_Load(FIF_JPEG, "img\\header.jpg", JPEG_DEFAULT);
		fbmp_bg				= FreeImage_Load(FIF_JPEG, "img\\content-bg.jpg", JPEG_DEFAULT);
		fbmp_headlines2		= FreeImage_Load(FIF_JPEG, "img\\headlines-2.jpg", JPEG_DEFAULT);
#endif		
		fbmp_weatherbg		= FreeImage_Load(FIF_PNG, "img\\partlycloudy.png", PNG_DEFAULT);
		fbmp_weathergrad	= FreeImage_Load(FIF_PNG, "img\\weathergrad.png", PNG_DEFAULT);
		fbmp_headlines1		= FreeImage_Load(FIF_JPEG, "img\\headlines-1.jpg", JPEG_DEFAULT);
		fbmp_loading		= FreeImage_Load(FIF_PNG, "img\\loading.png", PNG_DEFAULT);
		
		FreeImage_PreMultiplyWithAlpha(fbmp_loading);
//	playlist_load(hWnd);

		//SetTimer(hWnd, 4, 50, NULL);
		break;
		}
	case WM_PAINT: {
		PAINTSTRUCT ps;

		
		BeginPaint(hWnd, &ps);

		static int timeleft = 0xffff;
		static int imgalpha = 205;
		

		hdcMem = CreateCompatibleDC(ps.hdc);
		hbmMem = CreateCompatibleBitmap(ps.hdc, SCREEN_WIDTH, SCREEN_HEIGHT);
		hOld   = SelectObject(hdcMem, hbmMem);
		
		StretchDIBits(hdcMem, 0, CONTENT_TOP, CONTENT_WIDTH, CONTENT_HEIGHT, 
			0, 0, CONTENT_WIDTH, CONTENT_HEIGHT, FreeImage_GetBits(fbmp_bg), FreeImage_GetInfo(fbmp_bg), DIB_RGB_COLORS, SRCCOPY);
		
		
		char *days[7] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
		char *days_abbr[7] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
		
		time_t rawtime;
		tm *ptm;
		char datetimestr[17];

		memset(datetimestr, 0, 17);
		time (&rawtime);
		ptm = localtime(&rawtime);

		
		SetBkMode(hdcMem, TRANSPARENT); 
		SetTextColor(hdcMem, color_white);
		
		HFONT font_dejavusans_bold_20 = MakeFont("DejaVu Sans", true, false, false, 20);
		HFONT font_dejavusans_bold_24 = MakeFont("DejaVu Sans", true, false, false, 24);
		HFONT font_dejavusans_cond_12 = MakeFont("DejaVu Sans Condensed", false, false, false, 12);
		HFONT font_dejavusans_cond_18 = MakeFont("DejaVu Sans Condensed", false, false, false, 18);
		HFONT font_dejavusans_cond_bold_24 = MakeFont("DejaVu Sans Condensed", true, false, false, 24);
		HFONT font_dejavusans_cond_bold_32 = MakeFont("DejaVu Sans Condensed", true, false, false, 32);
		HFONT font_dejavusans_cond_bold_42 = MakeFont("DejaVu Sans Condensed", true, false, false, 42);
		HFONT font_dejavusans_cond_bold_58 = MakeFont("DejaVu Sans Condensed", true, false, false, 58);
		
		
		//debug_print("%08lX\n", g_current_elem);
		if(!g_current_elem){
			g_current_elem = g_playlist.first;
		}

		if(g_current_elem){
			if(g_current_elem->ready){
				if(g_current_elem->type == 1){
					
					image_element_t *image = (image_element_t *) g_current_elem->data;
					//debug_print("%d", image->loaded);
					//debug_print("asdf\n");
					if(timeleft == 0xffff) timeleft = g_current_elem->secs * FRAMES_PER_SEC / 8;
					image->bf.SourceConstantAlpha = imgalpha;
					
					AlphaBlend(hdcMem, CONTENT_WIDTH/2 - image->width/2, CONTENT_TOP + CONTENT_HEIGHT/2 - image->height/2, 
						image->width, image->height, image->hdc, 0, 0, image->width, image->height, image->bf);
					
					if(timeleft == 0){
						if(g_current_elem->next) g_current_elem = g_current_elem->next;
						debug_print("%08lX\n", g_current_elem);
						StretchDIBits(hdcMem, 0, CONTENT_TOP, CONTENT_WIDTH, CONTENT_HEIGHT, 0, 0, CONTENT_WIDTH, CONTENT_HEIGHT, 
							FreeImage_GetBits(fbmp_bg), FreeImage_GetInfo(fbmp_bg), DIB_RGB_COLORS, SRCCOPY);
						timeleft = 0xffff;
						imgalpha = 55;
					}else{
						timeleft--;
						if(timeleft < 15){
							imgalpha -= 15;
						}else if(imgalpha < 255) imgalpha += 10;
					}

			/*		HDC temp = CreateCompatibleDC(hdcMem);
					HBITMAP temp_bmp = CreateCompatibleBitmap(hdc, FreeImage_GetWidth(image->fbmp_image), FreeImage_GetHeight(image->fbmp_image));
					SelectObject(temp, temp_bmp);
	*/
				//	image->hdc = CreateCompatibleDC(hdcMem);
				//	image->hbm = CreateCompatibleBitmap(image->hdc, FreeImage_GetWidth(image->fbmp_image), FreeImage_GetHeight(image->fbmp_image));
				//	SelectObject(image->hdc, image->hbm);
					
					

					//SetTextColor(image->hdc, RGB(0, 0, 0));
					//TextOut(image->hdc, 100, 100, "asdf", 4);
					//BitBlt(hdcMem, 0, 112, 	FreeImage_GetWidth(image->fbmp_image), FreeImage_GetHeight(image->fbmp_image), image->hdc, 0, 0, SRCCOPY);
				//	DeleteObject(image->hbm);
				//	DeleteDC(image->hdc);
					
				}else if(g_current_elem->type == 3){
					static int iter = 0;
					LONGLONG position;
					LONGLONG duration;
					//if(g_current_elem->next) g_current_elem = g_current_elem->next;

					//debug_print("video\n");

					video_element_t *video = (video_element_t *) g_current_elem->data;
					
					if(iter == 0) {
						debug_print("loading video.........\n");
						video_load(hWnd, video);
						g_video_painting = true;
					}
					if(video_isloaded()){
						video_getduration(&duration);
						video_getposition(&position);
						
						
						video_render(hWnd, hdcMem, video);
						
						
						video_getduration(&duration);
						video_getposition(&position);
						//debug_print("%lld / %lld\n", position, duration);
						
						iter++;
						
						if(position >= duration){
							debug_print("video end reached.\n");
							video_unload(video);
							g_video_painting = false;
							iter = 0;
							if(g_current_elem->next) g_current_elem = g_current_elem->next;
							debug_print("%08lX\n", g_current_elem);
							//playlist_dumpitems();
						}
					}
					

					//video->iVMRwc->RepaintVideo(hWnd, hdcMem);
				}
			}else{
				char msg[64];
				static int t = 0;
				
				HDC hdcTemp = CreateCompatibleDC(hdcMem);
				HBITMAP hbmTemp = CreateCompatibleBitmap(ps.hdc, 400, 225);
				SelectObject(hdcTemp, hbmTemp);
				
				BLENDFUNCTION bf;
				bf.BlendOp = AC_SRC_OVER;
				bf.BlendFlags = 0;
				bf.SourceConstantAlpha = 255;
				bf.AlphaFormat = AC_SRC_ALPHA;

				StretchDIBits(hdcTemp, 0, 0, 400, 225, 0, 0, 400, 225, FreeImage_GetBits(fbmp_loading), FreeImage_GetInfo(fbmp_loading), DIB_RGB_COLORS, SRCCOPY);
				AlphaBlend(hdcMem, CONTENT_WIDTH/2 - 200, CONTENT_TOP + CONTENT_HEIGHT/2 - 225/2, 400, 225, hdcTemp, 0, 0, 400, 225, bf);
				
				if(g_current_elem->type == 1){
					image_element_t *img = (image_element_t *)g_current_elem->data;
					sprintf(msg, "File: %s\n%d bytes downloaded.", img->filename, (int)g_current_elem->progress_now);
				}else if(g_current_elem->type == 3){
					video_element_t *vid = (video_element_t *)g_current_elem->data;
					sprintf(msg, "File: %s\n%d bytes downloaded.", vid->filename, (int)g_current_elem->progress_now);
				}else{
					sprintf(msg, "%d bytes downloaded.", (int)g_current_elem->progress_now);
				}
				SelectObject(hdcMem, font_dejavusans_cond_bold_24);
				
				debug_print("%s\n", msg);

				RECT textBound;
				textBound.left = CONTENT_WIDTH/2 - 200 + 10; textBound.top = CONTENT_TOP + CONTENT_HEIGHT/2 - 225/2 + 95;
				textBound.right = CONTENT_WIDTH/2 + 190; textBound.bottom = CONTENT_TOP + CONTENT_HEIGHT/2 + 395/2;
				DrawTextA(hdcMem, msg, strlen(msg), &textBound, DT_LEFT);

				DeleteObject(hbmTemp);
				DeleteDC(hdcTemp);

				if(t++ / FRAMES_PER_SEC >= 5){
					// 5 seconds and the playlist item wasn't loaded; move on to next one
					if(g_current_elem->next) g_current_elem = g_current_elem->next;
					t = 0;
				}
			}
		}
		

		weather_t *current = weather_getcurrent();
		if(current) {
			FIBITMAP *fbmp_weather_cur = weather_getimage_current();

			// Paint weather panel
			

			//fbmp_weatherbg = FreeImage_Composite(fbmp_weatergrad, TRUE, NULL, fbmp_weatherbg);
			StretchDIBits(hdcMem, CONTENT_WIDTH, CONTENT_TOP - 1, 300, 225, 0, 0, 300, 225, 
				FreeImage_GetBits(fbmp_weatherbg), FreeImage_GetInfo(fbmp_weatherbg), DIB_RGB_COLORS, SRCCOPY);

			FreeImage_SetTransparent(fbmp_weather_cur, true);

			
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
				//BitBlt(hdcMem, 990, 180, 1280, 720, temp, 0, 0, SRCCOPY);
				DeleteObject(temp_bmp);
				DeleteDC(temp);
			}
			
			
			// Display temperatures
			RECT textBound;
			textBound.left = CONTENT_WIDTH + 30; textBound.top = CONTENT_TOP + 8;
			textBound.right = SCREEN_WIDTH - 20; textBound.bottom = CONTENT_TOP + 68;
			SelectObject(hdcMem, font_dejavusans_cond_bold_42);
			DrawTextA(hdcMem, current->temp, strlen(current->temp), &textBound, DT_RIGHT);
			
			
			textBound.left = CONTENT_WIDTH - 5; textBound.top = CONTENT_TOP + 148;
			textBound.right = CONTENT_WIDTH - 5 + 295; textBound.bottom = CONTENT_TOP + 148 + 75;
			SelectObject(hdcMem, font_dejavusans_cond_bold_32);
			//SetTextAlign(hdcMem,  TA_RIGHT);
			if(current->description){
				//ExtTextOut(hdcMem, 1280, 280, NULL, &textBound, current->description, strlen(current->description), NULL);
				DrawTextA(hdcMem, current->description, strlen(current->description), &textBound, DT_RIGHT | DT_WORDBREAK);
			}else{
				//ExtTextOut(hdcMem, 1280, 280, NULL, &textBound, "loading weather data...", strlen("loading weather data..."), NULL);
				DrawTextA(hdcMem, "loading weather data...", strlen("loading weather data..."), &textBound, DT_RIGHT | DT_WORDBREAK);
			}
			
			//SetTextAlign(hdcMem, TA_LEFT | TA_TOP);
		}

		
		weather_fc_t **forecast = weather_getforecast();
		
		if(forecast) {
			
			FIBITMAP *fbmp_weather_fc0 = weather_getimage_fc(0);
			FIBITMAP *fbmp_weather_fc1 = weather_getimage_fc(1);

			//FreeImage_Rescale(fbmp_weather_fc0, 188, 135, FILTER_BILINEAR);
			//FreeImage_Rescale(fbmp_weather_fc1, 188, 135, FILTER_BILINEAR);

			StretchDIBits(hdcMem, CONTENT_WIDTH, CONTENT_TOP + 223, 300, 150, 0, 0, 300, 150, FreeImage_GetBits(fbmp_weathergrad), FreeImage_GetInfo(fbmp_weathergrad), DIB_RGB_COLORS, SRCCOPY);
			
			SelectObject(hdcMem, font_dejavusans_bold_24);
			RECT textBound;
			textBound.left = CONTENT_WIDTH; textBound.top = CONTENT_TOP + 228;
			textBound.right = CONTENT_WIDTH + 145; textBound.bottom = CONTENT_TOP + 228 + 30;

			DrawTextA(hdcMem, days_abbr[ptm->tm_wday == 6 ? 0 : ptm->tm_wday + 1], 
					strlen(days_abbr[ptm->tm_wday == 6 ? 0 : ptm->tm_wday + 1]), &textBound, DT_RIGHT);
			
			textBound.left = CONTENT_WIDTH + 150; textBound.top = CONTENT_TOP + 228;
			textBound.right = CONTENT_WIDTH + 150 + 145; textBound.bottom = CONTENT_TOP + 228 + 30;

			DrawTextA(hdcMem, days_abbr[ptm->tm_wday == 6 ? 1 : (ptm->tm_wday == 5 ? 0 : ptm->tm_wday + 2)], 
					strlen(days_abbr[ptm->tm_wday == 6 ? 1 : (ptm->tm_wday == 5 ? 0 : ptm->tm_wday + 2)]), &textBound, DT_RIGHT);

			
			if(fbmp_weather_fc0 && fbmp_weather_fc1){
				HDC temp = CreateCompatibleDC(hdcMem);
				HBITMAP temp_bmp = CreateCompatibleBitmap(ps.hdc, 300, 150);
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
	
				AlphaBlend(hdcMem, CONTENT_WIDTH, CONTENT_TOP + 228, 300, 150, temp, 0, 0, 300, 150, bf);
				//BitBlt(hdcMem, 990, 180, 1280, 720, temp, 0, 0, SRCCOPY);
				DeleteObject(temp_bmp);
				DeleteDC(temp);
			}
			
			char high[8];

			sprintf(high, "hi %s", forecast[0]->temp_hi);
			textBound.left = CONTENT_WIDTH; textBound.top = CONTENT_TOP + 328;
			textBound.right = CONTENT_WIDTH + 120; textBound.bottom = CONTENT_TOP + 328 + 40;
			SelectObject(hdcMem, font_dejavusans_cond_bold_32);
			DrawTextA(hdcMem, high, strlen(high), &textBound, DT_RIGHT);

			sprintf(high, "hi %s", forecast[1]->temp_hi);
			textBound.left = CONTENT_WIDTH + 150; textBound.top = CONTENT_TOP + 328;
			textBound.right = CONTENT_WIDTH + 150 + 120; textBound.bottom = CONTENT_TOP + 328 + 40;
			DrawTextA(hdcMem, high, strlen(high), &textBound, DT_RIGHT);

			SelectObject(hdcMem, font_dejavusans_cond_12);
			TextOut(hdcMem, CONTENT_WIDTH + 124, CONTENT_TOP + 331, "low", strlen("low"));
			TextOut(hdcMem, CONTENT_WIDTH + 124 + 150, CONTENT_TOP + 331, "low", strlen("low"));

			textBound.left = CONTENT_WIDTH + 95; textBound.top = CONTENT_TOP + 343;
			textBound.right = CONTENT_WIDTH + 95 + 48; textBound.bottom = CONTENT_TOP + 343 + 20;
			SelectObject(hdcMem, font_dejavusans_cond_18);
			DrawTextA(hdcMem, forecast[0]->temp_lo, strlen(forecast[0]->temp_lo), &textBound, DT_RIGHT);

			textBound.left = CONTENT_WIDTH + 95 + 150; textBound.top = CONTENT_TOP + 343;
			textBound.right = CONTENT_WIDTH + 95 + 150 + 48; textBound.bottom = CONTENT_TOP + 343 + 20;
			DrawTextA(hdcMem, forecast[1]->temp_lo, strlen(forecast[1]->temp_lo), &textBound, DT_RIGHT);
		}
		
		
		StretchDIBits(hdcMem, 0, 0, SCREEN_WIDTH, CONTENT_TOP, 0, 0, SCREEN_WIDTH, CONTENT_TOP, 
			FreeImage_GetBits(fbmp_header), FreeImage_GetInfo(fbmp_header), DIB_RGB_COLORS, SRCCOPY);

		memset(datetimestr, 0, 17);
		strftime(datetimestr, 16, "%b %d, %Y", ptm);
		SetBkMode(hdcMem, TRANSPARENT); 
		SetTextColor(hdcMem, color_white);
        SelectObject(hdcMem, font_dejavusans_bold_20);
        TextOut(hdcMem, CONTENT_WIDTH - 260, 25, days[ptm->tm_wday], strlen(days[ptm->tm_wday]));
        SelectObject(hdcMem, font_dejavusans_cond_bold_32);
        TextOut(hdcMem, CONTENT_WIDTH - 263, 50, datetimestr, strlen(datetimestr));
		
		memset(datetimestr, 0, 17);
		strftime(datetimestr, 16, "%H:%M:%S", ptm);
        SelectObject(hdcMem, font_dejavusans_cond_bold_58);
        TextOut(hdcMem, CONTENT_WIDTH + 20, 27, datetimestr, strlen(datetimestr));


		HPEN pen = CreatePen(PS_DASH, 2, RGB(100, 100, 100));
		SelectObject(hdcMem, pen);
		MoveToEx(hdcMem, CONTENT_WIDTH, CONTENT_TOP, NULL);
		LineTo(hdcMem, CONTENT_WIDTH, CONTENT_BOTTOM);
		DeleteObject(pen);

		StretchDIBits(hdcMem, 160, CONTENT_BOTTOM, SCREEN_WIDTH - 160, 56, 0, 0, SCREEN_WIDTH - 160, 56, 
			FreeImage_GetBits(fbmp_headlines2), FreeImage_GetInfo(fbmp_headlines2), DIB_RGB_COLORS, SRCCOPY);
		

		SelectObject(hdcMem, font_dejavusans_cond_bold_24);
		TextOut(hdcMem, g_headline_x, CONTENT_BOTTOM + 16, headline_txt, strlen(headline_txt));
		
		SIZE extents;
		GetTextExtentPoint32(hdcMem, headline_txt, strlen(headline_txt), &extents);
		if(g_headline_x + extents.cx < 160) g_headline_x = SCREEN_WIDTH;

		StretchDIBits(hdcMem, 0, CONTENT_BOTTOM, 160, 56, 0, 0, 160, 56, FreeImage_GetBits(fbmp_headlines1), FreeImage_GetInfo(fbmp_headlines1), DIB_RGB_COLORS, SRCCOPY);


		//int a = StretchDIBits(ps.hdc, 0, 0, 1280, 720, 0, 0, 1280, 720, content_bg_bytes, &bmi, DIB_RGB_COLORS);
		//debug_print("ret: %d, err: %d\n", a, GetLastError());

		BitBlt(ps.hdc, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, hdcMem, 0, 0, SRCCOPY);
		
	//	video_element_t *video = (video_element_t *) g_current_elem->data;
	//			
		
		DeleteObject(font_dejavusans_bold_20);
		DeleteObject(font_dejavusans_bold_24);
		DeleteObject(font_dejavusans_cond_12);
		DeleteObject(font_dejavusans_cond_18);
		DeleteObject(font_dejavusans_cond_bold_24);
		DeleteObject(font_dejavusans_cond_bold_32);
		DeleteObject(font_dejavusans_cond_bold_42);
		DeleteObject(font_dejavusans_cond_bold_58);

		SelectObject(hdcMem, hOld);
		DeleteObject(hbmMem);
		DeleteDC    (hdcMem);
		
		EndPaint(hWnd, &ps);
		break;
				   }
	case WM_TIMER:
		if(wParam == 1) {
			RECT rt;
			rt.top = 0; rt.left = 0; rt.right = SCREEN_WIDTH; rt.bottom = CONTENT_TOP;
			InvalidateRect(hWnd, &rt, false);
			//g_ticks++;
			return 0;
		} else if(wParam == 2) {
			CreateThread(NULL, 0x2000, (unsigned long (__stdcall *)(void *))update, (void *)hWnd, 0, NULL);
			//weather_update();
			return 0;
		} else if(wParam == 3) {
			RECT rt, rt2;
			rt.top = CONTENT_TOP; rt.left = 0; rt.right = CONTENT_WIDTH; rt.bottom = CONTENT_BOTTOM;
			if(!g_video_painting) InvalidateRect(hWnd, &rt, false);
			rt2.top = CONTENT_BOTTOM; rt2.left = 0; rt2.right = SCREEN_WIDTH; rt2.bottom = SCREEN_HEIGHT;
			InvalidateRect(hWnd, &rt2, false);
			g_headline_x -= 2;
			g_ticks++;
			return 0;
		}
		/* else if(wParam == 4) {
		RECT rt;
		rt.top = 111; rt.left = 0; rt.right = 980; rt.bottom = 664;
		InvalidateRect(hWnd, &rt, false);
		g_ticks++;
		return 0;
		} */
		break;
	case WM_ERASEBKGND:
		break;
	case WM_DESTROY:
		
		playlist_unload();

		FreeImage_Unload(fbmp_bg);
		FreeImage_Unload(fbmp_header);
		FreeImage_Unload(fbmp_weatherbg);
		FreeImage_Unload(fbmp_weathergrad);
		FreeImage_Unload(fbmp_headlines1);
		FreeImage_Unload(fbmp_headlines2);
		FreeImage_Unload(fbmp_loading);
		FreeImage_DeInitialise();
		

		debug_close();
		free(sig);
		free(bmp_bytes);
		weather_exit();
		video_shutdown();
		//bmp->~Bitmap();
		//content_bg->~Image();
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

void generate_sig() {
	char *api_key = "41e0a9448f91edba4b05c6c2fc0edb1d6418aa292b5b2942637bec43a29b9523";
	char date[11];
	time_t rawtime;
	tm * ptm;
	time (&rawtime);
	ptm = gmtime (&rawtime);

	strftime(date, 11, "%Y-%m-%d", ptm);

	memset(sig, 0, 41);

	// SHA1 hash the produced string
	SHA1Context sha;
	uint8_t Message_Digest[20];
	SHA1Reset(&sha);
	SHA1Input(&sha, (const unsigned char *) api_key, strlen(api_key));
	SHA1Input(&sha, (const unsigned char *) date, strlen(date));
	if(!SHA1Result(&sha, Message_Digest)) {
		for(int i = 0; i < 20; i++) {
			char temp[2];
			sprintf(temp, "%02x", Message_Digest[i]);
			strcat(sig, temp);
		}
	} else {
		// TODO: handle errors
	}
	debug_print("%s\n", sig);
}

// format needs to contain a %s where the sig goes
void make_url(char *dest, char *format){
	sprintf(dest, format, sig);
}



size_t writefunc(void *ptr, size_t size, size_t nmemb, void *stream){
	/*int ret = 0;
	int tmp = nmemb * size;
	while(nmemb > 0){
		if(nmemb > 4096){
			nmemb -= 4096;
			ret += fwrite(ptr, size, 4096, (FILE *)stream);
		}else{
			ret += fwrite(ptr, size, nmemb, (FILE *)stream);
			nmemb = 0;
		}
	}
	//debug_print("[curl/writefunc] ret: %d; size: %d\n", ret, tmp);
	fflush((FILE *) stream);*/
	unsigned long ret = 0;
	WriteFile((HANDLE)stream, ptr, size * nmemb, &ret, NULL);
	return ret;
}

int download_curl(char *url, char *dest_file, bool show_progress, void *callback, void *arg){
	CURL *curl;
	CURLcode res = (CURLcode)-1;

	curl = curl_easy_init();

	if(curl){
		//FILE *fp = fopen(dest_file, "wb");
		HANDLE file = CreateFile(dest_file, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if(file){
			
			debug_print("[download_curl] URL: %s\n", url);
			debug_print("[download_curl] dest file: %s\n", dest_file);

			curl_easy_setopt(curl, CURLOPT_URL, url);
			curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/5.0 (compatible; Display UI Client)");
			curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 3);
			curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);

			if(show_progress){
				curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0);
				curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, callback);
				curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, arg);
			}	
			
			curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writefunc);
			
			// write to the destination file
			curl_easy_setopt(curl, CURLOPT_WRITEDATA, file);
			res = curl_easy_perform(curl);
			//fclose(fp);
			if(res != CURLE_OK) debug_print("[download_curl] error %d\n", res);
			//debug_print("[download_curl] finished dl! %s\n", dest_file);
			//fwrite(full_url, 1, strlen(full_url), fp);
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

int download_curl(char *url, char *dest_file){
	return download_curl(url, dest_file, false, NULL, NULL);
}

int dui_download(char *url, char *dest_file, bool show_progress, void *callback, void *arg){

	char *full_url = (char *) malloc(strlen(url) + 39);
	char *temp_dest = (char *) malloc(strlen(dest_file) + 5);
	make_url(full_url, url);
	sprintf(temp_dest, "%s.tmp", dest_file);
	
	debug_print("%s .. %s\n", temp_dest, dest_file);

	int ret = download_curl(full_url, temp_dest, show_progress, callback, arg);
		//URLDownloadToFile(NULL, full_url, temp_dest, 0, NULL);//download_curl(full_url, dest_file);


	if(ret == CURLE_OK){
		MoveFileEx(temp_dest, dest_file, MOVEFILE_REPLACE_EXISTING);
	}

	free(full_url);
	free(temp_dest);

	return ret;
}

int dui_download(char *url, char *dest_file){
	return dui_download(url, dest_file, false, NULL, NULL);
}
