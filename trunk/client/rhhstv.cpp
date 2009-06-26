// rhhstv.cpp : Defines the entry point for the application.
//

#include <windows.h>
#include <gdiplus.h>
#include <gdiplusgraphics.h>
#include <urlmon.h>
#include <wchar.h>

#include <stdio.h>
#include <time.h>

#include "rhhstv.h"
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
		"RHHS TV",
		WS_OVERLAPPEDWINDOW,    // fullscreen values
		200, 200,
	// use this for full-screen:
	//	-GetSystemMetrics(SM_CXFRAME), -GetSystemMetrics(SM_CYCAPTION) - 
	//		GetSystemMetrics(SM_CYFRAME),    // the starting x and y positions should be 0
		1280, 760,    // 760 for now to account for title bar
		NULL,
		NULL,
		hInstance,
		NULL);
	
    ShowWindow(hWnd, nCmdShow);
	//URLDownloadToFile(NULL, "http://du.geekie.org/server/api/weather/current/?sys_name=asdf&sig=7181f0adafe3dd03cfc98249c17b5836639653b8&location=CAXX0401", "data\\weather\\weather_c.dat", 0, NULL);
	
	debug_init();
	sig = (char *) malloc(41);
	generate_sig();
	weather_update();
	

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
	case WM_PAINT:{
			PAINTSTRUCT ps;
			
			HDC hdc = BeginPaint(hWnd, &ps);
			
			//HDC hdcMem = CreateCompatibleDC(hdc);
			//HBITMAP hbmOld = SelectObject(hdcMem, pHeaderImage);

			// TODO: Add any drawing code here...
      OnPaint(hdc);
/*

			BITMAPINFO bmi;
			memset(&bmi, 0, sizeof(bmi));
			bmi.bmiHeader.biSize        =    sizeof(BITMAPINFOHEADER);
			bmi.bmiHeader.biWidth       = 1280;
			bmi.bmiHeader.biHeight      = -111; // top-down image
			bmi.bmiHeader.biPlanes      = 1;
			bmi.bmiHeader.biBitCount    = 0;
			bmi.bmiHeader.biCompression = BI_JPEG;
			bmi.bmiHeader.biSizeImage   = 56396;
			
			int i = StretchDIBits(hdc, 0, 0, 1280, 111, 0, 0, 1280, 111, pHeaderImage, &bmi, DIB_RGB_COLORS, SRCCOPY);
			
			if(i != 0){
				MessageBox(NULL, "aaaaaasdf", "a", MB_OK);
			}*/

			

			//RECT rt;
			//GetClientRect(hWnd, &rt);
			//DrawText(hdc, "asdf", 4, &rt, DT_CENTER);
			//SelectObject(hdcMem, hbmOld);
			//DeleteDC(hdcMem);
			EndPaint(hWnd, &ps);
			break;
		}
	case WM_DESTROY:
			Gdiplus::GdiplusShutdown(gdiplusToken);
			debug_close();
			free(sig);
			PostQuitMessage(0);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}


VOID OnPaint(HDC hdc){
	Gdiplus::Graphics graphics(hdc);
	
	// Draw header image
	Gdiplus::Image header(L"img\\header.jpg");
	Gdiplus::RectF destRect(0, 0, 1280, 111);
	graphics.DrawImage(&header, destRect);

	
	// Draw headlines bg image
	Gdiplus::Image headlines(L"img\\headlines.jpg");
	Gdiplus::RectF destRect2(0, 664, 1280, 56);
	graphics.DrawImage(&headlines, destRect2);
	
	// Draw a rectangle for the weather stuff
	Gdiplus::Pen pen (Gdiplus::Color(255, 0, 0, 255), 1);
	Gdiplus::RectF destRect3(980, 111, 300, 553);
	graphics.DrawRectangle(&pen, destRect3);



	// Display weather info
	weather_t *current = weather_getcurrent();
	
	
	Gdiplus::Image weather_bg(L"img\\partlycloudy.png");
	Gdiplus::RectF destRect4(983, 110, 297, 224);
	graphics.DrawImage(&weather_bg, destRect4);


	Gdiplus::SolidBrush  brush(Gdiplus::Color(255, 255, 255, 255));
	Gdiplus::FontFamily  fontFamily(L"Tahoma");
	Gdiplus::Font        font(&fontFamily, 40, Gdiplus::FontStyleBold, Gdiplus::UnitPixel);
	Gdiplus::Font        font2(&fontFamily, 32, Gdiplus::FontStyleBold, Gdiplus::UnitPixel);
	

	graphics.DrawString((unsigned short *) current->temp, -1, &font, Gdiplus::PointF(1155, 120), &brush);
	graphics.DrawString((unsigned short *) current->description, -1, &font2, Gdiplus::PointF(1000, 280), &brush);



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
