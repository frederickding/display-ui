// rhhstv.cpp : Defines the entry point for the application.
//

#include <windows.h>
#include <gdiplus.h>
#include <gdiplusgraphics.h>
#include <urlmon.h>
#include <wchar.h>

#include <stdio.h>
#include <time.h>

#include "display_ui.h"
#include "debug.h"
#include "weather.h"
#include "sha1.h"

#pragma comment (lib, "gdiplus.lib")
#pragma comment(lib, "urlmon.lib") 

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
int loadImages();
void destroyImages();
VOID OnPaint(HDC hdc);

ULONG_PTR gdiplusToken;
LPVOID pHeaderImage;

int ticks = 0;

char *sig; // request signature (api key + date)

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
	
    RegisterClassEx(&wc);
	
    hWnd = CreateWindowEx(WS_EX_CLIENTEDGE,
		"WindowClass1",
		"Display-UI",
		WS_OVERLAPPEDWINDOW,    // fullscreen values
		200, 200,
	// use this for full-screen:
	//	-GetSystemMetrics(SM_CXFRAME), -GetSystemMetrics(SM_CYCAPTION) - 
	//		GetSystemMetrics(SM_CYFRAME),    // the starting x and y positions should be 0
		1280 + 2*(GetSystemMetrics(SM_CXFRAME) + GetSystemMetrics(SM_CXEDGE)), 720 + 2 * (GetSystemMetrics(SM_CYSIZEFRAME) + GetSystemMetrics(SM_CYBORDER) + GetSystemMetrics(SM_CYEDGE)) + GetSystemMetrics(SM_CYCAPTION),    // 760 for now to account for title bar
		NULL,
		NULL,
		hInstance,
		NULL);
	
    ShowWindow(hWnd, nCmdShow);
	//URLDownloadToFile(NULL, "http://du.geekie.org/server/api/weather/current/?sys_name=asdf&sig=7181f0adafe3dd03cfc98249c17b5836639653b8&location=CAXX0401", "data\\weather\\weather_c.dat", 0, NULL);
	
	debug_init();
	sig = (char *) malloc(41);
	generate_sig();
	weather_init();

    // enter the main loop:
	
    MSG msg;
	
	while(GetMessage(&msg, NULL, 0, 0) > 0){
        TranslateMessage(&msg);
        DispatchMessage(&msg);
	}
	

	return msg.wParam;
}


LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{


	switch (message){
	case WM_CREATE:
		SetTimer(hWnd, 1, 1000, NULL);
		break;
	case WM_PAINT:{
			PAINTSTRUCT ps;
			
			HDC hdc = BeginPaint(hWnd, &ps);

			OnPaint(hdc);
			EndPaint(hWnd, &ps);
			break;
		}
	case WM_TIMER:
		
		RECT rt;
		rt.top = 0; rt.left = 0; rt.right = 1280; rt.bottom = 720;
		InvalidateRect(hWnd, &rt, false);
		ticks++;
		break;
	case WM_DESTROY:
			Gdiplus::GdiplusShutdown(gdiplusToken);
			debug_close();
			free(sig);
			weather_exit();
			PostQuitMessage(0);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}


VOID OnPaint(HDC hdc){
	Gdiplus::Bitmap bmp(1280, 720);
	Gdiplus::Graphics gfx(hdc);
	Gdiplus::Graphics* graphics = Gdiplus::Graphics::FromImage(&bmp);


	Gdiplus::SolidBrush  whitebrush(Gdiplus::Color(255, 255, 255, 255));
	Gdiplus::FontFamily  dejavu(L"DejaVu Sans");
	Gdiplus::FontFamily  dejavu_cond(L"DejaVu Sans Condensed");

	wchar_t *days[7] = {L"Sunday", L"Monday", L"Tuesday", L"Wednesday", L"Thursday", L"Friday", L"Saturday"};
	wchar_t *days_abbr[7] = {L"Sun", L"Mon", L"Tue", L"Wed", L"Thur", L"Fri", L"Sat"};
	
	// date/time stuff
	time_t rawtime;
	tm *ptm;
	char datetimestr[17];
	char datetimewstr[34];
	time (&rawtime);
	ptm = localtime(&rawtime);
	if(ticks % 1 == 0){
		
		graphics->SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
		
		Gdiplus::Image content_bg(L"img\\content-bg.jpg");
		
		graphics->DrawImage(&content_bg, Gdiplus::Rect(0, 111, 980, 553));

		
		// Draw a rectangle for the weather stuff
		Gdiplus::Pen pen (Gdiplus::Color(255, 0, 0, 255), 1);
		Gdiplus::RectF destRect3(980, 110, 300, 554);
		graphics->DrawRectangle(&pen, destRect3);

		
		Gdiplus::StringFormat format(0, LANG_NEUTRAL);
		format.SetAlignment(Gdiplus::StringAlignmentFar);
		format.SetLineAlignment(Gdiplus::StringAlignmentCenter);
	
		// Display weather info
		weather_t *current = weather_getcurrent();
		if(current){
			Gdiplus::Image weather_bg2(L"img\\partlycloudy.png");
			graphics->DrawImage(&weather_bg2, Gdiplus::Rect(980, 110, 300, 225));
			
			Gdiplus::LinearGradientBrush  gradient(Gdiplus::Point(980, 110), Gdiplus::Point(1280, 335),
				Gdiplus::Color(200, 27, 85, 122), Gdiplus::Color(200, 22, 68, 94));
			
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
			*/
			// Description text
			graphics->DrawString((unsigned short *) current->description, -1, &Gdiplus::Font(&dejavu_cond, 32, Gdiplus::FontStyleBold, Gdiplus::UnitPixel), 
				Gdiplus::RectF(975, 260, 300, 75), &format, &whitebrush);
			
		}
		
		// Weather forecast
		
		weather_fc_t **forecast = weather_getforecast();
		
		if(forecast){
			Gdiplus::LinearGradientBrush  gradient2(Gdiplus::Point(980, 335), Gdiplus::Point(980, 485),
				Gdiplus::Color(255, 40, 40, 40), Gdiplus::Color(255, 25, 25, 25));
			graphics->FillRectangle(&gradient2, Gdiplus::RectF(980, 335, 300, 150));
			//graphics->FillRectangle(&Gdiplus::SolidBrush(Gdiplus::Color(100, 100, 100, 100)), Gdiplus::RectF(980, 335, 300, 30));
			
			
			graphics->DrawLine(&Gdiplus::Pen(Gdiplus::Color(255, 100, 100, 100), 1), Gdiplus::Point(980, 335), Gdiplus::Point(1280, 335));
			graphics->DrawLine(&Gdiplus::Pen(Gdiplus::Color(255, 100, 100, 100), 1), Gdiplus::Point(1130, 335), Gdiplus::Point(1130, 485));
			
			
			//format.SetAlignment(Gdiplus::StringAlignmentCenter);
			graphics->DrawString((unsigned short *) days_abbr[ptm->tm_wday == 6 ? 0 : ptm->tm_wday + 1], -1, 
				&Gdiplus::Font(&dejavu, 24, Gdiplus::FontStyleBold, Gdiplus::UnitPixel), Gdiplus::RectF(980, 340, 145, 30), &format, &whitebrush);
			
			graphics->DrawString((unsigned short *) days_abbr[ptm->tm_wday == 6 ? 1 : (ptm->tm_wday == 5 ? 0 : ptm->tm_wday + 1)], -1, 
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
	
		wchar_t tmp[4];
		swprintf(tmp, L"%d", ticks);
		graphics->DrawString(tmp, -1, &Gdiplus::Font(&dejavu, 18, Gdiplus::FontStyleBold, Gdiplus::UnitPixel), Gdiplus::PointF(100, 200), NULL, &Gdiplus::SolidBrush(Gdiplus::Color(ticks, 0, 0, 0)));
		
		
	}
//	if(ticks % 25 == 0){
		// Draw header image
		Gdiplus::Image header(L"img\\header.png");
		Gdiplus::RectF destRect(0, 0, 1280, 111);
		graphics->DrawImage(&header, destRect);
		
		// Draw date
		
		memset(datetimestr, 0, 17);
		memset(datetimewstr, 0, 34);
		strftime(datetimestr, 16, "%B %d, %Y", ptm);
		mbstowcs((unsigned short *)datetimewstr, (const char *) datetimestr, strlen(datetimestr));
		
		graphics->DrawString((unsigned short *) days[ptm->tm_wday], -1, &Gdiplus::Font(&dejavu, 20, Gdiplus::FontStyleBold, Gdiplus::UnitPixel), Gdiplus::PointF(720, 25), &whitebrush);
		graphics->DrawString((unsigned short *) datetimewstr, -1, &Gdiplus::Font(&dejavu_cond, 32, Gdiplus::FontStyleBold, Gdiplus::UnitPixel), Gdiplus::PointF(717, 50), &whitebrush);
		
		memset(datetimestr, 0, 17);
		memset(datetimewstr, 0, 34);
		strftime(datetimestr, 16, "%H:%M:%S", ptm);
		mbstowcs((unsigned short *)datetimewstr, (const char *) datetimestr, strlen(datetimestr));
		graphics->DrawString((unsigned short *) datetimewstr, -1, &Gdiplus::Font(&dejavu_cond, 58, Gdiplus::FontStyleBold, Gdiplus::UnitPixel), Gdiplus::PointF(975, 27), &whitebrush);
	
//	}
		

	// Draw headlines bg image
	Gdiplus::Image headlines(L"img\\headlines.jpg");
	Gdiplus::RectF destRect2(0, 664, 1280, 56);
	graphics->DrawImage(&headlines, destRect2);
	
	gfx.DrawImage(&bmp, 0, 0, 1280, 720);
	graphics->~Graphics();
	gfx.~Graphics();
	bmp.~Bitmap();
}


void generate_sig(){
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
	if(!SHA1Result(&sha, Message_Digest)){
		for(int i = 0; i < 20; i++){
			char temp[2];
			sprintf(temp, "%02x", Message_Digest[i]);
			strcat(sig, temp);
		}
	}else{
		// TODO: handle errors
	}
	//debug_print("%s\n", sig);
}

// format needs to contain a %s where the sig goes
void make_url(char *dest, char *format){
	sprintf(dest, format, sig);
}

int download(char *url, char *dest_file){
	char *full_url = (char *) malloc(strlen(url) + 39);
	make_url(full_url, url);
	int ret = URLDownloadToFile(NULL, full_url, dest_file, 0, NULL);
	free(full_url);

	return ret;
}
