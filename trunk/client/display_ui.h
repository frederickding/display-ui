#ifndef display_ui_header
#define display_ui_header

// NOTE: If you change the following values, you must supply header and footer images
// of the appropriate size in the img directory! Necessary files:
//    - header.jpg (SCREEN_WIDTH x 112)
//    - headlines-2.jpg (SCREEN_WIDTH - 160 x 56)
//    - content-bg.jpg (CONTENT_WIDTH x CONTENT_HEIGHT)

#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 768

#define DISPLAY_WIDTH 1280
#define DISPLAY_HEIGHT 720

#define TOP_SPACER 24
#define BOTTOM_SPACER 24

#define CONTENT_WIDTH (DISPLAY_WIDTH - 300)
#define CONTENT_HEIGHT (DISPLAY_HEIGHT - 112 - 56)

#define CONTENT_TOP TOP_SPACER + 112
#define CONTENT_BOTTOM (CONTENT_TOP + CONTENT_HEIGHT)

void generate_sig();

// use the first functions if sig needs to be appended
int dui_download(char *url, char *dest_file);
int dui_download_retry(char *url, char *dest_file);
int dui_download(char *url, char *dest_file, bool show_progress, void *callback, void *arg);

int download_curl(char *url, char *dest_file);
int download_curl(char *url, char *dest_file, bool show_progress, void *callback, void *arg);

#endif
