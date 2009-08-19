/**
 * playlist.cpp, playlist stuff
 *
 * Copyright 2009 Kirill Peretoltchine
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
 * $Id: weather.cpp 138 2009-07-30 01:56:07Z frederickjding $
 */

#include <windows.h>
#include <urlmon.h>
#include <stdio.h>
#include <time.h>

#include "display_ui.h"
#include "freeimage.h"
#include "playlist.h"
#include "debug.h"

#pragma pack(1)

typedef struct _playlist_f_t{
	byte num_elements;
	byte data[1];
}playlist_f_t;

typedef struct _playlist_element_f_t{
	byte type;
	byte secs;
	byte trans;
	int id;
	int len; // 12 + 8 + filename len * 2
	byte data[1];
}playlist_element_f_t;

typedef struct _image_element_f_t{
	byte type;
	short width, height;
	int filename_len;
	char filename[1];
}image_element_f_t;

#pragma pack()

playlist_t g_playlist;


void playlist_load(HWND hwnd){

	FILE *fp = fopen("data\\playlist.dat", "r");
	
	if(fp){
		fseek(fp, 0, SEEK_END);
		int size = ftell(fp);
		fseek(fp, 0, SEEK_SET);

		char *raw_playlist = (char *) malloc(size);

		fread(raw_playlist, 1, size, fp);
		
		playlist_f_t *playlist = (playlist_f_t *) raw_playlist;
		g_playlist.num_elements = playlist->num_elements;
		debug_print("number of elements: %d **\n", g_playlist.num_elements);
		
		playlist_element_f_t *current = (playlist_element_f_t *) playlist->data;		
		
		playlist_element_t *prev_elem = NULL;
		for(int i = 0; i < playlist->num_elements; i++){
			debug_print("element type: %d\nsecs: %d\nlen: %x\n", current->type, current->secs, current->len);
			playlist_element_t *new_elem = (playlist_element_t *) malloc(sizeof(playlist_element_t));
			new_elem->type = current->type;
			new_elem->secs = current->secs;
			new_elem->trans = current->trans;
			
			if(new_elem->type == 1){
				// image
				image_element_f_t *current_img = (image_element_f_t *) current->data;
				image_element_t *new_img = (image_element_t *) malloc(sizeof(image_element_t));
				memset(new_img, 0, sizeof(image_element_t));
				new_img->filename = (char *) malloc(current_img->filename_len + 1);
				memset(new_img->filename, 0, current_img->filename_len + 1);

				new_img->width = current_img->width;
				new_img->height = current_img->height;
				memcpy(new_img->filename, current_img->filename, current_img->filename_len);
				debug_print("width: %d, height: %d, filename: %s\n", new_img->width, new_img->height, new_img->filename);
				
				new_img->type = current_img->type;
				new_img->fbmp_image = FreeImage_Load((FREE_IMAGE_FORMAT)current_img->type, new_img->filename, NULL);
				if(current_img->type == FIF_PNG) FreeImage_PreMultiplyWithAlpha(new_img->fbmp_image);
				

				HDC hdcWin = GetDC(hwnd);
				new_img->hdc = CreateCompatibleDC(hdcWin);
				new_img->hbm = CreateCompatibleBitmap(hdcWin, FreeImage_GetWidth(new_img->fbmp_image), FreeImage_GetHeight(new_img->fbmp_image));
				SelectObject(new_img->hdc, new_img->hbm);
				
				// Draw the image once to the DC to avoid having to redraw every time a frame is rendered
				StretchDIBits(new_img->hdc, 0, 0, FreeImage_GetWidth(new_img->fbmp_image), FreeImage_GetHeight(new_img->fbmp_image), 
					0, 0, FreeImage_GetWidth(new_img->fbmp_image), FreeImage_GetHeight(new_img->fbmp_image), 
					FreeImage_GetBits(new_img->fbmp_image), FreeImage_GetInfo(new_img->fbmp_image), DIB_RGB_COLORS, SRCCOPY);
				ReleaseDC(hwnd, hdcWin);

				new_img->bf.BlendOp = AC_SRC_OVER;
				new_img->bf.BlendFlags = 0;
				new_img->bf.SourceConstantAlpha = 255;
				new_img->bf.AlphaFormat = (new_img->type == FIF_PNG ? AC_SRC_ALPHA : AC_SRC_OVER);

				new_elem->data = new_img;
			}
			
			if(i == 0) g_playlist.first = new_elem;
			if(prev_elem) prev_elem->next = new_elem;
			prev_elem = new_elem;
			new_elem->next = g_playlist.first;
			current = (playlist_element_f_t *) ((unsigned long)current + (unsigned long)current->len);

			debug_print("%08lX -- %08lX .. %d\n", raw_playlist, new_elem, new_elem->type);
		}

		free(raw_playlist);
		fclose(fp);
	}	
}

void playlist_unload(){
	playlist_element_t *current = g_playlist.first;

	for(int i = 0; i < g_playlist.num_elements; i++){
		debug_print("unload: %d -- %08lX\n", current->type, current);
		if(current->type == 1){
			image_element_t *current_img = (image_element_t *) current->data;
			debug_print("unload: %s\n", current_img->filename);
			if(current_img->fbmp_image) FreeImage_Unload(current_img->fbmp_image);
			if(current_img->filename) free(current_img->filename);
			//DeleteObject(current_img->hbm);
			DeleteDC(current_img->hdc);
			free(current_img);
		}
		playlist_element_t *tmp = current;
		current = current->next;
		free(tmp);
	}
}