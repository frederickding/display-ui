#ifndef playlist_header
#define playlist_header


typedef struct _playlist_element_t{
	byte type;
	int secs;
	int trans;
	
	void *data;
	_playlist_element_t *next;
	_playlist_element_t *prev;
}playlist_element_t;

typedef struct _playlist_t{
	byte num_elements;
	_playlist_element_t *first;
}playlist_t;

typedef struct _image_element_t{
	int width, height;
	wchar_t *filename;
}image_element_t;


void playlist_load();

#endif

