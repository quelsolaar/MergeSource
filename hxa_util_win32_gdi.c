#define _CRT_SECURE_NO_WARNINGS
#pragma comment(lib, "gdiplus.lib")
#include <stdio.h>
#include <windows.h>
//#include <gdiplus.h>

typedef void GDIImage;

typedef struct {
  unsigned int  GdiplusVersion;
  void			*DebugEventCallback;
  int           SuppressBackgroundThread;
  int           SuppressExternalCodecs;
  void           (*GdiplusStartupInput)(
    void		   *debugEventCallback,
    int           suppressBackgroundThread,
    int           suppressExternalCodecs);
}GdiplusStartupInput;

typedef struct{
	void *notificatiion_hook;
	void *notificatiion_unhook;
}GdiplusStartupOutput;

extern int __stdcall GdipLoadImageFromFile(const char *file_name, GDIImage **image);
extern int __stdcall GdiplusStartup(void **token, const GdiplusStartupInput *input, GdiplusStartupOutput *output);
extern void __stdcall GdiplusShutdown(void *token);

void load_image_test()
{
	FILE *f;
	void *gdi_session;
	GDIImage *image = NULL;
	GdiplusStartupInput startup_input;
	GdiplusStartupOutput startup_output;
	unsigned int i;
	return;
	startup_input.GdiplusVersion = 1;
	startup_input.DebugEventCallback = NULL;
	startup_input.SuppressBackgroundThread = 0;
	startup_input.SuppressExternalCodecs = 0; 
	startup_input.GdiplusStartupInput = NULL;
	i = GdiplusStartup(&gdi_session, &startup_input, &startup_output);
	i = GdipLoadImageFromFile("test_image.jpg", &image);
	GdiplusShutdown(gdi_session);
}
/*
https://stackoverflow.com/questions/5114591/how-to-use-gdi-in-c
https://learn.microsoft.com/en-us/windows/win32/gdiplus/-gdiplus-graphics-flat?redirectedfrom=MSDN
https://learn.microsoft.com/en-us/windows/win32/api/gdiplusinit/ns-gdiplusinit-gdiplusstartupinput
*/