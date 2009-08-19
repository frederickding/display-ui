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

#include <windows.h>
#include <gdiplus.h>
#include <gdiplusgraphics.h>
#include <urlmon.h>
#include <wchar.h>
#include <stdio.h>
#include <time.h>

#include "display_ui.h"
#include "freeimage.h"
#include "debug.h"
#include "weather.h"
#include "playlist.h"
#include "sha1.h"


#pragma comment (lib, "gdiplus.lib")
#pragma comment(lib, "urlmon.lib") 
#pragma comment(lib, "freeimage.lib") 
#pragma comment(lib, "msimg32.lib") 

#define MakeFont(name, bold, italic, underline, size) CreateFont(-size, \
			0, 0, 0, (bold ? FW_BOLD : FALSE), (italic ? FW_BOLD : FALSE), (underline ? FW_BOLD : FALSE), FALSE, ANSI_CHARSET, \
			OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_ROMAN, name)


LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
int loadImages();
void destroyImages();
VOID OnPaint(HDC hdc);
VOID RepaintHeadlines(HDC hdc);
VOID RepaintHeader(HDC hdc);
VOID RepaintContent(HDC hdc);

ULONG_PTR gdiplusToken;
LPVOID pHeaderImage;

char *headline_txt = "The quick brown fox jumps over the lazy dog | Lorem ipsum dolor sit amet, consectetur adipiscing elit. Nulla lobortis hendrerit hendrerit.";

Gdiplus::Image *content_bg;
Gdiplus::Bitmap *bmp;

int ticks = 0;

char *sig; // request signature (api key + date)

extern playlist_t g_playlist;
playlist_element_t *g_current_elem;
void *bmp_bytes;

void update(void *p) {
	weather_update(p);
	/*if(download("http://du.geekie.org/server/api/headlines?sys_name=1&sig=%s", "data\\headlines.dat") == S_OK) {

		FILE *fp = fopen("data\\headlines.dat", "r");

		if(fp) {
			if(headline_txt) {
				free(headline_txt);
			}
			fseek(fp, 0, SEEK_END);
			int size = ftell(fp);
			fseek(fp, 0, SEEK_SET);


			headline_txt = (unsigned short*) malloc(size + 2);
			memset(headline_txt, 0, size + 2);
			fread(headline_txt, 1, size, fp);

			fclose(fp);
		}
	}*/
}

// the entry point for any Windows program
int WINAPI WinMain(HINSTANCE hInstance,
				   HINSTANCE hPrevInstance,
				   LPSTR lpCmdLine,
				   int nCmdShow)
{
	HWND hWnd;
	WNDCLASSEX wc;
	Gdiplus::GdiplusStartupInput gdiplusStartupInput;
	
	ZeroMemory(&wc, sizeof(WNDCLASSEX));
	
	Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = (WNDPROC)WndProc;
	wc.hInstance = hInstance;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)COLOR_GRAYTEXT;    // not needed any more
	wc.lpszClassName = "WindowClass1";
//	wc.hIcon  = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_ICON1));
	
	RegisterClassEx(&wc);
	
	hWnd = CreateWindowEx(WS_EX_CLIENTEDGE,
		"WindowClass1",
		"Display UI Client", // perhaps could be customizable through Registry
		WS_OVERLAPPEDWINDOW,    // fullscreen values
		200, 200,
		/* use this for full-screen:
		-GetSystemMetrics(SM_CXFRAME), -GetSystemMetrics(SM_CYCAPTION) - 
			GetSystemMetrics(SM_CYFRAME),    // the starting x and y positions should be 0 */
		1280 + 2*(GetSystemMetrics(SM_CXFRAME) + GetSystemMetrics(SM_CXEDGE)),
		720 + 2 * (GetSystemMetrics(SM_CYSIZEFRAME) + GetSystemMetrics(SM_CYEDGE)) + GetSystemMetrics(SM_CYCAPTION),    // 760 for now to account for title bar
		NULL,
		NULL,
		hInstance,
		NULL);

	ShowWindow(hWnd, nCmdShow);

	debug_init();
	sig = (char *) malloc(41);
	generate_sig();
	weather_init();
	playlist_load();
	g_current_elem = g_playlist.first;

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


	switch (message) {
	case WM_CREATE:{
		FreeImage_Initialise(true);
		SetTimer(hWnd, 1, 1000, NULL);
		SetTimer(hWnd, 2, 900000, NULL);
		SetTimer(hWnd, 3, 40, NULL);
			
		fbmp_header = FreeImage_Load(FIF_JPEG, "img\\header.jpg", JPEG_DEFAULT);
		fbmp_bg = FreeImage_Load(FIF_JPEG, "img\\content-bg.jpg", JPEG_DEFAULT);
		fbmp_weatherbg = FreeImage_Load(FIF_PNG, "img\\partlycloudy.png", PNG_DEFAULT);
		fbmp_weathergrad = FreeImage_Load(FIF_PNG, "img\\weathergrad.png", PNG_DEFAULT);
		fbmp_headlines1 = FreeImage_Load(FIF_JPEG, "img\\headlines-1.jpg", JPEG_DEFAULT);
		fbmp_headlines2 = FreeImage_Load(FIF_JPEG, "img\\headlines-2.jpg", JPEG_DEFAULT);




		//SetTimer(hWnd, 4, 50, NULL);
		break;
		}
	case WM_PAINT: {
		PAINTSTRUCT ps;
		
		HDC hdc = BeginPaint(hWnd, &ps);
		HFONT font;
		
		hdcMem = CreateCompatibleDC(hdc);
		hbmMem = CreateCompatibleBitmap(hdc, 1280, 720);
		hOld   = SelectObject(hdcMem, hbmMem);
		
		StretchDIBits(hdcMem, 0, 111, 980, 553, 0, 0, 980, 553, FreeImage_GetBits(fbmp_bg), FreeImage_GetInfo(fbmp_bg), DIB_RGB_COLORS, SRCCOPY);
		StretchDIBits(hdcMem, 0, 0, 1280, 112, 0, 0, 1280, 112, FreeImage_GetBits(fbmp_header), FreeImage_GetInfo(fbmp_header), DIB_RGB_COLORS, SRCCOPY);
		
		// Draw date & time
		
		char *days[7] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
		char *days_abbr[7] = {"Sun", "Mon", "Tue", "Wed", "Thur", "Fri", "Sat"};
		
		time_t rawtime;
		tm *ptm;
		char datetimestr[17];
		time (&rawtime);
		ptm = localtime(&rawtime);
		
		memset(datetimestr, 0, 17);
		strftime(datetimestr, 16, "%b %d, %Y", ptm);
		
		HFONT font_dejavusans_bold_20 = MakeFont("DejaVu Sans", true, false, false, 20);
		HFONT font_dejavusans_bold_24 = MakeFont("DejaVu Sans", true, false, false, 24);
		HFONT font_dejavusans_cond_12 = MakeFont("DejaVu Sans Condensed", false, false, false, 12);
		HFONT font_dejavusans_cond_18 = MakeFont("DejaVu Sans Condensed", false, false, false, 18);
		HFONT font_dejavusans_cond_bold_24 = MakeFont("DejaVu Sans Condensed", true, false, false, 24);
		HFONT font_dejavusans_cond_bold_32 = MakeFont("DejaVu Sans Condensed", true, false, false, 32);
		HFONT font_dejavusans_cond_bold_42 = MakeFont("DejaVu Sans Condensed", true, false, false, 42);
		HFONT font_dejavusans_cond_bold_58 = MakeFont("DejaVu Sans Condensed", true, false, false, 58);

		SetBkMode(hdcMem, TRANSPARENT); 
		SetTextColor(hdcMem, color_white);
        SelectObject(hdcMem, font_dejavusans_bold_20);
        TextOut(hdcMem, 720, 25, days[ptm->tm_wday], strlen(days[ptm->tm_wday]));
        SelectObject(hdcMem, font_dejavusans_cond_bold_32);
        TextOut(hdcMem, 717, 50, datetimestr, strlen(datetimestr));
		
		memset(datetimestr, 0, 17);
		strftime(datetimestr, 16, "%H:%M:%S", ptm);
        SelectObject(hdcMem, font_dejavusans_cond_bold_58);
        TextOut(hdcMem, 975, 27, datetimestr, strlen(datetimestr));
		
		weather_t *current = weather_getcurrent();
		if(current) {
			FIBITMAP *fbmp_weather_cur = weather_getimage_current();

			// Paint weather panel
			

			//fbmp_weatherbg = FreeImage_Composite(fbmp_weathergrad, TRUE, NULL, fbmp_weatherbg);
			StretchDIBits(hdcMem, 980, 110, 300, 225, 0, 0, 300, 225, FreeImage_GetBits(fbmp_weatherbg), FreeImage_GetInfo(fbmp_weatherbg), DIB_RGB_COLORS, SRCCOPY);

			FreeImage_SetTransparent(fbmp_weather_cur, true);

			HDC temp = CreateCompatibleDC(hdcMem);
			HBITMAP temp_bmp = CreateCompatibleBitmap(hdc, 250, 180);
			SelectObject(temp, temp_bmp);
			
			if(fbmp_weather_cur){
				BLENDFUNCTION bf;
				bf.BlendOp = AC_SRC_OVER;
				bf.BlendFlags = 0;
				bf.SourceConstantAlpha = 255;
				bf.AlphaFormat = AC_SRC_ALPHA;
				
				StretchDIBits(temp, 0, 0, 250, 180, 0, 0, 250, 180, FreeImage_GetBits(fbmp_weather_cur), 
					FreeImage_GetInfo(fbmp_weather_cur), DIB_RGB_COLORS, SRCCOPY);
				AlphaBlend(hdcMem, 990, 120, 250, 180, temp, 0, 0, 250, 180, bf);
				//BitBlt(hdcMem, 990, 180, 1280, 720, temp, 0, 0, SRCCOPY);
				DeleteObject(temp_bmp);
				DeleteDC(temp);
			}
			
			
			// Display temperatures
			RECT textBound;
			textBound.left = 1110; textBound.top = 120;
			textBound.right = 1260; textBound.bottom = 180;
			SelectObject(hdcMem, font_dejavusans_cond_bold_42);
			DrawTextA(hdcMem, current->temp, strlen(current->temp), &textBound, DT_RIGHT);
			
			
			textBound.left = 975; textBound.top = 260;
			textBound.right = 1275; textBound.bottom = 335;
			SelectObject(hdcMem, font_dejavusans_cond_bold_32);
			if(current->description){
				DrawTextA(hdcMem, current->description, strlen(current->description), &textBound, DT_RIGHT | DT_WORDBREAK);
			}else{
				DrawTextA(hdcMem, "loading weather data...", strlen("loading weather data..."), &textBound, DT_RIGHT | DT_WORDBREAK);
			}
		}

		
		weather_fc_t **forecast = weather_getforecast();
		
		if(forecast) {
			
			FIBITMAP *fbmp_weather_fc0 = weather_getimage_fc(0);
			FIBITMAP *fbmp_weather_fc1 = weather_getimage_fc(1);

			//FreeImage_Rescale(fbmp_weather_fc0, 188, 135, FILTER_BILINEAR);
			//FreeImage_Rescale(fbmp_weather_fc1, 188, 135, FILTER_BILINEAR);

			StretchDIBits(hdcMem, 980, 335, 300, 150, 0, 0, 300, 150, FreeImage_GetBits(fbmp_weathergrad), FreeImage_GetInfo(fbmp_weathergrad), DIB_RGB_COLORS, SRCCOPY);
			
			RECT textBound;
			textBound.left = 980; textBound.top = 340;
			textBound.right = 980 + 145; textBound.bottom = 340 + 30;
			DrawTextA(hdcMem, days_abbr[ptm->tm_wday == 6 ? 0 : ptm->tm_wday + 1], 
					strlen(days_abbr[ptm->tm_wday == 6 ? 0 : ptm->tm_wday + 1]), &textBound, DT_RIGHT);
			
			textBound.left = 1130; textBound.top = 340;
			textBound.right = 1130+ 145; textBound.bottom = 340 + 30;
			DrawTextA(hdcMem, days_abbr[ptm->tm_wday == 6 ? 1 : (ptm->tm_wday == 5 ? 0 : ptm->tm_wday + 2)], 
					strlen(days_abbr[ptm->tm_wday == 6 ? 1 : (ptm->tm_wday == 5 ? 0 : ptm->tm_wday + 2)]), &textBound, DT_RIGHT);

			
			if(fbmp_weather_fc0 && fbmp_weather_fc1){
				HDC temp = CreateCompatibleDC(hdcMem);
				HBITMAP temp_bmp = CreateCompatibleBitmap(hdc, 300, 150);
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
				AlphaBlend(hdcMem, 980, 340, 300, 150, temp, 0, 0, 300, 150, bf);
				//BitBlt(hdcMem, 990, 180, 1280, 720, temp, 0, 0, SRCCOPY);
				DeleteObject(temp_bmp);
				DeleteDC(temp);
			}
			
			char high[8];

			sprintf(high, "hi %s", forecast[0]->temp_hi);
			textBound.left = 980; textBound.top = 440;
			textBound.right = 980 + 122; textBound.bottom = 440 + 40;
			SelectObject(hdcMem, font_dejavusans_cond_bold_32);
			DrawTextA(hdcMem, high, strlen(high), &textBound, DT_RIGHT);

			sprintf(high, "hi %s", forecast[1]->temp_hi);
			textBound.left = 1130; textBound.top = 440;
			textBound.right = 1130 + 122; textBound.bottom = 440 + 40;
			DrawTextA(hdcMem, high, strlen(high), &textBound, DT_RIGHT);

			SelectObject(hdcMem, font_dejavusans_cond_12);
			TextOut(hdcMem, 1102, 443, "low", strlen("low"));
			TextOut(hdcMem, 1252, 443, "low", strlen("low"));

			textBound.left = 1075; textBound.top = 455;
			textBound.right = 1075 + 50; textBound.bottom = 455 + 20;
			SelectObject(hdcMem, font_dejavusans_cond_18);
			DrawTextA(hdcMem, forecast[0]->temp_lo, strlen(forecast[0]->temp_lo), &textBound, DT_RIGHT);

			textBound.left = 1225; textBound.top = 455;
			textBound.right = 1225 + 50; textBound.bottom = 455 + 20;
			DrawTextA(hdcMem, forecast[1]->temp_lo, strlen(forecast[1]->temp_lo), &textBound, DT_RIGHT);
		}

		// Headlines
		static int xpos = 1280;
		StretchDIBits(hdcMem, 160, 664, 1120, 56, 0, 0, 1120, 56, FreeImage_GetBits(fbmp_headlines2), FreeImage_GetInfo(fbmp_headlines2), DIB_RGB_COLORS, SRCCOPY);
		
		xpos -= 2;

		SelectObject(hdcMem, font_dejavusans_cond_bold_24);
		TextOut(hdcMem, xpos, 664 + 16, headline_txt, strlen(headline_txt));
		
		SIZE extents;
		GetTextExtentPoint(hdcMem, headline_txt, strlen(headline_txt), &extents);
		if(xpos + extents.cx < 160) xpos = 1280;

		StretchDIBits(hdcMem, 0, 664, 160, 56, 0, 0, 160, 56, FreeImage_GetBits(fbmp_headlines1), FreeImage_GetInfo(fbmp_headlines1), DIB_RGB_COLORS, SRCCOPY);


		//int a = SetDIBitsToDevice(hdcMem, 0, 111, 980, 553, 0, 0, 0, 553, content_bg_bytes, &bmi, DIB_RGB_COLORS);
		//debug_print("ret: %d, err: %d\n", a, GetLastError());
		BitBlt(hdc, 0, 0, 1280, 720, hdcMem, 0, 0, SRCCOPY);

		
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

		
		/***********
		if(ps.rcPaint.top == 664) {
			RepaintHeadlines(hdc);
		} else if(ps.rcPaint.bottom == 112) {
			RepaintHeader(hdc);
		} else if(ps.rcPaint.top == 111) {
			RepaintContent(hdc);
			
			RepaintHeadlines(hdc);
			//RepaintHeader(hdc)
		} else {
			OnPaint(hdc);
			RepaintHeader(hdc);
			RepaintHeadlines(hdc);
		}	
		*********************/
		
		EndPaint(hWnd, &ps);
		break;
				   }
	case WM_TIMER:
		if(wParam == 1) {
			RECT rt;
			rt.top = 0; rt.left = 0; rt.right = 1280; rt.bottom = 112;
			InvalidateRect(hWnd, &rt, false);
			//ticks++;
			return 0;
		} else if(wParam == 2) {
			CreateThread(NULL, 0x2000, (unsigned long (__stdcall *)(void *))update, (void *)hWnd, 0, NULL);
			//weather_update();
			return 0;
		} else if(wParam == 3) {
			RECT rt, rt2;
			rt.top = 111; rt.left = 0; rt.right = 980; rt.bottom = 664;
			InvalidateRect(hWnd, &rt, false);
			rt2.top = 664; rt2.left = 0; rt2.right = 1280; rt2.bottom = 720;
			InvalidateRect(hWnd, &rt2, false);

			ticks++;
			return 0;
		}
		/* else if(wParam == 4) {
		RECT rt;
		rt.top = 111; rt.left = 0; rt.right = 980; rt.bottom = 664;
		InvalidateRect(hWnd, &rt, false);
		ticks++;
		return 0;
		} */
		break;
	case WM_ERASEBKGND:
		break;
	case WM_DESTROY:
		Gdiplus::GdiplusShutdown(gdiplusToken);
		FreeImage_Unload(fbmp_bg);
		FreeImage_Unload(fbmp_header);
		FreeImage_Unload(fbmp_weatherbg);
		FreeImage_Unload(fbmp_weathergrad);
		FreeImage_Unload(fbmp_headlines1);
		FreeImage_Unload(fbmp_headlines2);
		FreeImage_DeInitialise();
		

		debug_close();
		free(sig);
		free(bmp_bytes);
		weather_exit();
		//bmp->~Bitmap();
		//content_bg->~Image();
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

Gdiplus::Image *image;
/*
VOID OnPaint(HDC hdc){
	Gdiplus::Bitmap bmp(1280, 720);
	Gdiplus::Graphics gfx(hdc);
	// Gdiplus::Graphics* graphics = &gfx;
	Gdiplus::Graphics* graphics = Gdiplus::Graphics::FromImage(&bmp);

	// date/time stuff
	time_t rawtime;
	tm *ptm;
	time (&rawtime);
	ptm = localtime(&rawtime);

	Gdiplus::SolidBrush  whitebrush(Gdiplus::Color(255, 255, 255, 255));
	Gdiplus::FontFamily  dejavu(L"DejaVu Sans");
	Gdiplus::FontFamily  dejavu_cond(L"DejaVu Sans Condensed");

	wchar_t *days_abbr[7] = {L"Sun", L"Mon", L"Tue", L"Wed", L"Thur", L"Fri", L"Sat"};


	// graphics->SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
	// graphics->SetTextRenderingHint(Gdiplus::TextRenderingHintAntiAlias);


	// Draw a rectangle for the weather stuff
	Gdiplus::Pen pen (Gdiplus::Color(255, 0, 0, 255), 1);
	Gdiplus::RectF destRect3(980, 110, 300, 554);
	graphics->DrawRectangle(&pen, destRect3);


	Gdiplus::StringFormat format(0, LANG_NEUTRAL);
	format.SetAlignment(Gdiplus::StringAlignmentFar);
	format.SetLineAlignment(Gdiplus::StringAlignmentCenter);

	// Display weather info
	weather_t *current = weather_getcurrent();
	if(current) {
		Gdiplus::Image weather_bg2(L"img\\partlycloudy.png");
		graphics->DrawImage(&weather_bg2, Gdiplus::Rect(980, 110, 300, 225));

		Gdiplus::LinearGradientBrush  gradient(Gdiplus::Point(980, 110), Gdiplus::Point(1280, 335),
			Gdiplus::Color(150, 27, 85, 122), Gdiplus::Color(200, 22, 68, 94));

		Gdiplus::RectF destRect4(980, 110, 300, 225);
		graphics->FillRectangle(&gradient, destRect4);

		Gdiplus::Image weather_bg(L"img\\current.png");
		graphics->DrawImage(&weather_bg, Gdiplus::Rect(990, 120, 250, 180)); // 250x180 icons


		// Current temperature
		graphics->DrawString((unsigned short *) current->temp, -1, &Gdiplus::Font(&dejavu_cond, 42, Gdiplus::FontStyleBold, Gdiplus::UnitPixel), 
			Gdiplus::RectF(1110, 120, 150, 60), &format, &whitebrush);

		/*
		// Drop-shadow for description
		Gdiplus::Bitmap dropshadow(300, 75, graphics);
		Gdiplus::Graphics* tempgfx = Gdiplus::Graphics::FromImage(&dropshadow);
		tempgfx->DrawString((unsigned short *) current->description, -1, &Gdiplus::Font(&dejavu_cond, 36,
		Gdiplus::FontStyleBold, Gdiplus::UnitPixel), Gdiplus::RectF(0, 0, 300, 75), &format, 
		&Gdiplus::SolidBrush(Gdiplus::Color(150, 0, 0, 0)));

		Gdiplus::Image *thumb = dropshadow.GetThumbnailImage(20, 5, NULL, NULL);

		graphics->DrawImage(thumb, Gdiplus::RectF(990, 270, 300, 75));
		thumb->~Image();
		dropshadow.~Bitmap();
		tempgfx->~Graphics();
		
		// Description text
		graphics->DrawString((unsigned short *) current->description, -1, &Gdiplus::Font(&dejavu_cond, 32, Gdiplus::FontStyleBold, Gdiplus::UnitPixel), 
			Gdiplus::RectF(975, 260, 300, 75), &format, &whitebrush);
	}

	// Weather forecast

	weather_fc_t **forecast = weather_getforecast();

	if(forecast) {
		Gdiplus::LinearGradientBrush  gradient2(Gdiplus::Point(980, 335), Gdiplus::Point(980, 485),
			Gdiplus::Color(255, 40, 40, 40), Gdiplus::Color(255, 25, 25, 25));
		graphics->FillRectangle(&gradient2, Gdiplus::RectF(980, 335, 300, 150));
		// graphics->FillRectangle(&Gdiplus::SolidBrush(Gdiplus::Color(100, 100, 100, 100)), Gdiplus::RectF(980, 335, 300, 30));


		graphics->DrawLine(&Gdiplus::Pen(Gdiplus::Color(255, 100, 100, 100), 1), Gdiplus::Point(980, 335), Gdiplus::Point(1280, 335));
		graphics->DrawLine(&Gdiplus::Pen(Gdiplus::Color(255, 100, 100, 100), 1), Gdiplus::Point(1130, 335), Gdiplus::Point(1130, 485));


		// format.SetAlignment(Gdiplus::StringAlignmentCenter);
		graphics->DrawString((unsigned short *) days_abbr[ptm->tm_wday == 6 ? 0 : ptm->tm_wday + 1], -1, 
			&Gdiplus::Font(&dejavu, 24, Gdiplus::FontStyleBold, Gdiplus::UnitPixel), Gdiplus::RectF(980, 340, 145, 30), &format, &whitebrush);

		graphics->DrawString((unsigned short *) days_abbr[ptm->tm_wday == 6 ? 1 : (ptm->tm_wday == 5 ? 0 : ptm->tm_wday + 2)], -1, 
			&Gdiplus::Font(&dejavu, 24, Gdiplus::FontStyleBold, Gdiplus::UnitPixel), Gdiplus::RectF(1130, 340, 145, 30), &format, &whitebrush);

		wchar_t high[8];

		swprintf(high, L"hi %s", forecast[0]->temp_hi);
		graphics->DrawImage(&Gdiplus::Image(L"img\\forecast_0.png"), Gdiplus::Rect(980, 340, 188, 135));
		graphics->DrawString(high, -1, &Gdiplus::Font(&dejavu_cond, 32, Gdiplus::FontStyleBold, Gdiplus::UnitPixel),
			Gdiplus::RectF(980, 440, 125, 40), &format, &whitebrush);
		graphics->DrawString(L"low", -1, &Gdiplus::Font(&dejavu_cond, 12, Gdiplus::FontStyleRegular, Gdiplus::UnitPixel), 
			Gdiplus::PointF(1102, 443), &whitebrush);
		graphics->DrawString(forecast[0]->temp_lo, -1, &Gdiplus::Font(&dejavu_cond, 18, Gdiplus::FontStyleRegular, Gdiplus::UnitPixel), 
			Gdiplus::RectF(1075, 455, 50, 20), &format, &whitebrush);


		swprintf(high, L"hi %s", forecast[1]->temp_hi);
		graphics->DrawImage(&Gdiplus::Image(L"img\\forecast_1.png"), Gdiplus::Rect(1130, 340, 188, 135));
		graphics->DrawString(high, -1, &Gdiplus::Font(&dejavu_cond, 32, Gdiplus::FontStyleBold, Gdiplus::UnitPixel),
			Gdiplus::RectF(1130, 440, 125, 40), &format, &whitebrush);
		graphics->DrawString(L"low", -1, &Gdiplus::Font(&dejavu_cond, 12, Gdiplus::FontStyleRegular, Gdiplus::UnitPixel), 
			Gdiplus::PointF(1252, 443), &whitebrush);
		graphics->DrawString(forecast[0]->temp_lo, -1, &Gdiplus::Font(&dejavu_cond, 18, Gdiplus::FontStyleRegular, Gdiplus::UnitPixel), 
			Gdiplus::RectF(1225, 455, 50, 20), &format, &whitebrush);
	}

	Gdiplus::LinearGradientBrush  gradient3(Gdiplus::Point(980, 485), Gdiplus::Point(1280, 664),
		Gdiplus::Color(255, 200, 200, 200), Gdiplus::Color(255, 100, 100, 110));
	graphics->FillRectangle(&gradient3, Gdiplus::RectF(980, 485, 300, 179));


	gfx.DrawImage(&bmp, 0, 0, 1280, 720);
	graphics->~Graphics();
	gfx.~Graphics();
	bmp.~Bitmap();
}

VOID RepaintHeadlines(HDC hdc) {
	static int xpos = 1280;
	Gdiplus::Bitmap bmp(1280, 56);
	Gdiplus::Graphics gfx(hdc);
	Gdiplus::Graphics* graphics = Gdiplus::Graphics::FromImage(&bmp);

	Gdiplus::SolidBrush  whitebrush(Gdiplus::Color(255, 255, 255, 255));
	Gdiplus::FontFamily  dejavu(L"DejaVu Sans");
	Gdiplus::FontFamily  dejavu_cond(L"DejaVu Sans Condensed");

	// Draw headlines bg image
	graphics->DrawImage(&Gdiplus::Image(L"img\\headlines-2.jpg"), Gdiplus::RectF(160, 0, 1120, 56));

	// wchar_t tmp[5];
	// swprintf(tmp, L"%d", ticks);

	xpos -= 2;

	debug_print("%s", headline_txt);
	graphics->DrawString(headline_txt, -1, &Gdiplus::Font(&dejavu_cond, 24, Gdiplus::FontStyleBold, Gdiplus::UnitPixel), 
		Gdiplus::PointF((float)xpos, 16), NULL, &Gdiplus::SolidBrush(Gdiplus::Color(255, 240, 240, 240)));

	Gdiplus::RectF boundrect;

	graphics->MeasureString(headline_txt, -1, &Gdiplus::Font(&dejavu_cond, 24, Gdiplus::FontStyleBold, Gdiplus::UnitPixel), 
		Gdiplus::PointF((float)xpos, 16), NULL, &boundrect);

	if(boundrect.GetRight() <= 160) xpos = 1280;

	// graphics->DrawRectangle(&Gdiplus::Pen(Gdiplus::Color(255, 255, 255, 255)), boundrect);
	// debug_print("%s\n", "asdf");

	graphics->DrawImage(&Gdiplus::Image(L"img\\headlines-1.jpg"), Gdiplus::RectF(0, 0, 160, 56));

	gfx.DrawImage(&bmp, 0, 664, 1280, 56);
	graphics->~Graphics();
	gfx.~Graphics();
	bmp.~Bitmap();
}


VOID RepaintHeader(HDC hdc) {
	Gdiplus::Bitmap bmp(1280, 112);
	Gdiplus::Graphics gfx(hdc);
	Gdiplus::Graphics* graphics = Gdiplus::Graphics::FromImage(&bmp);

	wchar_t *days[7] = {L"Sunday", L"Monday", L"Tuesday", L"Wednesday", L"Thursday", L"Friday", L"Saturday"};

	// date/time stuff
	time_t rawtime;
	tm *ptm;
	char datetimestr[17];
	char datetimewstr[34];
	time (&rawtime);
	ptm = localtime(&rawtime);

	Gdiplus::SolidBrush  whitebrush(Gdiplus::Color(255, 255, 255, 255));
	Gdiplus::FontFamily  dejavu(L"DejaVu Sans");
	Gdiplus::FontFamily  dejavu_cond(L"DejaVu Sans Condensed");

	Gdiplus::Image header(L"img\\header.png");
	Gdiplus::RectF destRect(0, 0, 1280, 111);
	graphics->DrawImage(&header, destRect);

	// Draw date

	memset(datetimestr, 0, 17);
	memset(datetimewstr, 0, 34);
	strftime(datetimestr, 16, "%b %d, %Y", ptm);
	mbstowcs((unsigned short *)datetimewstr, (const char *) datetimestr, strlen(datetimestr));

	graphics->DrawString((unsigned short *) days[ptm->tm_wday], -1, &Gdiplus::Font(&dejavu, 20, Gdiplus::FontStyleBold, Gdiplus::UnitPixel), Gdiplus::PointF(720, 25), &whitebrush);
	graphics->DrawString((unsigned short *) datetimewstr, -1, &Gdiplus::Font(&dejavu_cond, 32, Gdiplus::FontStyleBold, Gdiplus::UnitPixel), Gdiplus::PointF(717, 50), &whitebrush);

	memset(datetimestr, 0, 17);
	memset(datetimewstr, 0, 34);
	strftime(datetimestr, 16, "%H:%M:%S", ptm);
	mbstowcs((unsigned short *)datetimewstr, (const char *) datetimestr, strlen(datetimestr));
	graphics->DrawString((unsigned short *) datetimewstr, -1, &Gdiplus::Font(&dejavu_cond, 58, Gdiplus::FontStyleBold, Gdiplus::UnitPixel), Gdiplus::PointF(975, 27), &whitebrush);


	gfx.DrawImage(&bmp, 0, 0, 0, 0, 1280, 112, Gdiplus::UnitPixel);
	graphics->~Graphics();
	gfx.~Graphics();
	bmp.~Bitmap();
}


VOID RepaintContent(HDC hdc) {
	Gdiplus::Graphics gfx(hdc);
	Gdiplus::Graphics* graphics = Gdiplus::Graphics::FromImage(bmp);

	Gdiplus::FontFamily  dejavu(L"DejaVu Sans");
	Gdiplus::FontFamily  dejavu_cond(L"DejaVu Sans Condensed");

	static int alpha = 0;
	static int angle = 0;
	static int inc   = 5;
	static int ticks_x = 100;
	static int ticks_y = 100;
	static int ticks_xs = 2;
	static int ticks_ys = 3;
	
	static int timeleft = 100;
	
	static Gdiplus::ColorMatrix cm = {1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
								0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
								0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
								0.0f, 0.0f, 0.0f, 0.8f, 0.0f, // 3,3
								0.0f, 0.0f, 0.0f, 0.0f, 1.0f};

	//cm.m[3][3] = 0.5f;
	
	//Gdiplus::TextureBrush temp(content_bg, Gdiplus::WrapModeClamp);
	//graphics->FillRectangle(&temp, Gdiplus::RectF(0, 0, 980, 553));
	graphics->DrawImage(content_bg, Gdiplus::Rect(0, 0, 980, 553));

	graphics->FillRectangle(&Gdiplus::SolidBrush(Gdiplus::Color(150, 20, 20, 20)), Gdiplus::RectF(0, 0, 980, 553));

	if(g_current_elem){
		if(g_current_elem->type == 1){
			if(timeleft == 0){
				if(g_current_elem->next) g_current_elem = g_current_elem->next;
				timeleft = g_current_elem->secs * 20;
				cm.m[3][3] = 0.2f;
			}else{ 
				timeleft--;
				if(timeleft < 10){
					cm.m[3][3] -= 0.1f;
				}else if(cm.m[3][3] < 1.0f) cm.m[3][3] += 0.1f;
			}
			
			

			image_element_t *image = (image_element_t *) g_current_elem->data;

			
			Gdiplus::ImageAttributes imageAtt;
			imageAtt.SetColorMatrix(&cm, Gdiplus::ColorMatrixFlagsDefault, Gdiplus::ColorAdjustTypeBitmap);
			Gdiplus::Image *img = Gdiplus::Image::FromFile(image->filename);

			graphics->DrawImage(img, 
				Gdiplus::Rect(980/2 - image->width/2, 553/2 - image->height/2, image->width, image->height),
				0, 0, image->width, image->height, Gdiplus::UnitPixel, &imageAtt);
			img->~Image();
		}
	}


/*	 wchar_t tmp[5];
	swprintf(tmp, L"%d", ticks);

	graphics->DrawString(tmp, -1, &Gdiplus::Font(&dejavu_cond, 46, Gdiplus::FontStyleBold, Gdiplus::UnitPixel), 
	Gdiplus::PointF(ticks_x, ticks_y), &Gdiplus::SolidBrush(Gdiplus::Color(255, 24, 48, 72)));
	ticks_x += ticks_xs;
	ticks_y += ticks_ys;
	if(ticks_x < 0 || ticks_x > 850) ticks_xs = -ticks_xs;
	if(ticks_y < 0 || ticks_y > 500) ticks_ys = -ticks_ys;

	Gdiplus::StringFormat format(0, LANG_NEUTRAL);
	format.SetAlignment(Gdiplus::StringAlignmentCenter);
	format.SetLineAlignment(Gdiplus::StringAlignmentCenter);

	graphics->TranslateTransform(500.0f, 300.0f);
	graphics->RotateTransform((float)angle, Gdiplus::MatrixOrderPrepend);

	graphics->DrawString(L"Content goes here", -1, &Gdiplus::Font(&dejavu_cond, 58, Gdiplus::FontStyleBold, Gdiplus::UnitPixel), 
	Gdiplus::RectF(-300, -50, 600, 100), &format, &Gdiplus::SolidBrush(Gdiplus::Color(alpha, 20, 20, 20)));

	angle += 2;
	alpha += inc;
	if(alpha >= 255 || alpha <= 0) inc = -inc;
	

	gfx.DrawImage(bmp, 0, 111, 0, 0, 980, 553, Gdiplus::UnitPixel);
	graphics->~Graphics();
	gfx.~Graphics();
	//bmp.~Bitmap();
}
*/
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

int download(char *url, char *dest_file){
	char *full_url = (char *) malloc(strlen(url) + 39);
	make_url(full_url, url);

	debug_print("%s\n", full_url);
	int ret = URLDownloadToFile(NULL, full_url, dest_file, 0, NULL);
	free(full_url);

	return ret;
}
