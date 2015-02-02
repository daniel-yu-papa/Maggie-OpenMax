/*test the YUV picture rendering speed in SDL1.2*/
#include <stdlib.h>
#include <stdio.h>

#include "SDL.h"

/* Call this instead of exit(), so we can clean up SDL: atexit() is evil. */
static void
quit(int rc)
{
    SDL_Quit();
    exit(rc);
}

/* NOTE: These RGB conversion functions are not intended for speed,
         only as examples.
*/

void
RGBtoYUV(Uint8 * rgb, int *yuv, int monochrome, int luminance)
{
    if (monochrome) {
#if 1                           /* these are the two formulas that I found on the FourCC site... */
        yuv[0] = (int)(0.299 * rgb[0] + 0.587 * rgb[1] + 0.114 * rgb[2]);
        yuv[1] = 128;
        yuv[2] = 128;
#else
        yuv[0] = (int)(0.257 * rgb[0]) + (0.504 * rgb[1]) + (0.098 * rgb[2]) + 16;
        yuv[1] = 128;
        yuv[2] = 128;
#endif
    } else {
#if 1                           /* these are the two formulas that I found on the FourCC site... */
        yuv[0] = (int)(0.299 * rgb[0] + 0.587 * rgb[1] + 0.114 * rgb[2]);
        yuv[1] = (int)((rgb[2] - yuv[0]) * 0.565 + 128);
        yuv[2] = (int)((rgb[0] - yuv[0]) * 0.713 + 128);
#else
        yuv[0] = (0.257 * rgb[0]) + (0.504 * rgb[1]) + (0.098 * rgb[2]) + 16;
        yuv[1] = 128 - (0.148 * rgb[0]) - (0.291 * rgb[1]) + (0.439 * rgb[2]);
        yuv[2] = 128 + (0.439 * rgb[0]) - (0.368 * rgb[1]) - (0.071 * rgb[2]);
#endif
    }

    if (luminance != 100) {
        yuv[0] = yuv[0] * luminance / 100;
        if (yuv[0] > 255)
            yuv[0] = 255;
    }
}

void
ConvertRGBtoYV12(Uint8 *rgb, Uint8 *out, int w, int h,
                 int monochrome, int luminance, int rbg_bytes)
{
    int x, y;
    int yuv[3];
    Uint8 tmp;
    Uint8 *op[3];

    op[0] = out;
    op[1] = op[0] + w*h;
    op[2] = op[1] + w*h/4;
    for (y = 0; y < h; ++y) {
        for (x = 0; x < w; ++x) {
            /* Swap R and B */
            tmp = rgb[0];
            rgb[0] = rgb[2];
            rgb[2] = tmp;

            RGBtoYUV(rgb, yuv, monochrome, luminance);
            *(op[0]++) = yuv[0];
            if (x % 2 == 0 && y % 2 == 0) {
                *(op[1]++) = yuv[2];
                *(op[2]++) = yuv[1];
            }
            rgb += rbg_bytes;
        }
    }
}

void UpdateTexture(SDL_Overlay *o, Uint8 *imageYUV, Uint32 width, Uint32 height) 
{ 
    int x,y;
	int yuv[3];
	Uint8 *p[3],*op[3];
    Uint8 *pSrc;
    
	SDL_LockYUVOverlay(o);

    p[0]=imageYUV; 
    p[1]=imageYUV + width * height; 
    p[2]=imageYUV + width * height * 5 / 4; 
    
    //fprintf(stderr, "pitches[0]: %d, pitches[1]: %d, pitches[2]: %d\n", 
    //                 o->pitches[0], o->pitches[1], o->pitches[2]);
	memcpy(o->pixels[0], p[0], o->pitches[0] * height);
	memcpy(o->pixels[1], p[1], o->pitches[1] * height / 2);
	memcpy(o->pixels[2], p[2], o->pitches[2] * height / 2);
        
    SDL_UnlockYUVOverlay(o);
} 

int
main(int argc, char **argv)
{
    SDL_Surface *screen, *image;
    SDL_Overlay *overlay;

    Uint8 *imageYUV;
    Uint8 i;
    SDL_Event event;
    Uint32 then, now, frames;
    SDL_bool done = SDL_FALSE;
    SDL_Rect rect;
    
    if (!argv[1]) {
        fprintf(stderr, "Usage: %s file.bmp\n", argv[0]);
        return 1;
    }

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "Couldn't initialize SDL: %s\n", SDL_GetError());
        return 2;
    }

    image = SDL_LoadBMP(argv[1]);
    if (!image) {
        fprintf(stderr, "Couldn't load BMP file %s: %s\n", argv[1], SDL_GetError());
        quit(3);
    }
    /*if (image->format->BytesPerPixel != 3) {
        fprintf(stderr, "BMP must be 24-bit\n");
        quit(4);
    }*/

    /* wxh for the V plane, and then w/2xh/2 for the U and V planes */
    imageYUV = SDL_malloc(image->w*image->h+(image->w*image->h)/2);
    ConvertRGBtoYV12(image->pixels, imageYUV, image->w, image->h, 0, 100, image->format->BytesPerPixel);

    /* Initialize the display */
	screen = SDL_SetVideoMode(image->w, image->h, 0, 0);
	if ( screen == NULL ) {
		fprintf(stderr, "Couldn't set video mode: %s\n",
					SDL_GetError());
		quit(1);
	}
	printf("Set%s %dx%dx%d mode\n",
			screen->flags & SDL_FULLSCREEN ? " fullscreen" : "",
			screen->w, screen->h, screen->format->BitsPerPixel);
	printf("(video surface located in %s memory)\n",
			(screen->flags&SDL_HWSURFACE) ? "video" : "system");
	if ( screen->flags & SDL_DOUBLEBUF ) {
		printf("Double-buffering enabled\n");
	}
    
    /* Set the window manager title bar */
	SDL_WM_SetCaption("YUV speed test", "testframespeed");
	
	/* Create the overlay */
	overlay = SDL_CreateYUVOverlay(image->w, image->h, SDL_YV12_OVERLAY, screen);
	if ( overlay == NULL ) {
		fprintf(stderr, "Couldn't create overlay: %s\n", SDL_GetError());
		quit(1);
	}
	printf("Created %dx%dx%d %s %s overlay\n",overlay->w,overlay->h,overlay->planes,
			overlay->hw_overlay?"hardware":"software",
			overlay->format==SDL_YV12_OVERLAY?"YV12":
			overlay->format==SDL_IYUV_OVERLAY?"IYUV":
			overlay->format==SDL_YUY2_OVERLAY?"YUY2":
			overlay->format==SDL_UYVY_OVERLAY?"UYVY":
			overlay->format==SDL_YVYU_OVERLAY?"YVYU":
			"Unknown");
	for(i=0; i<overlay->planes; i++)
	{
		printf("  plane %d: pitch=%d\n", i, overlay->pitches[i]);
	}
	
	rect.w=overlay->w;
	rect.h=overlay->h;
	rect.x=0;
	rect.y=0;
		
    /* Main loop */
    frames = 0;
    then = SDL_GetTicks();
    while (!done) {
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
            case SDL_KEYDOWN:
                if (event.key.keysym.sym == SDLK_ESCAPE) {
                    done = SDL_TRUE;
                }
                break;
            case SDL_QUIT:
                done = SDL_TRUE;
                break;
            }
        }

        //SDL_UpdateTexture(texture, NULL, imageYUV, image->w);
        UpdateTexture(overlay, imageYUV, image->w, image->h);
		
		SDL_DisplayYUVOverlay(overlay,&rect);
        ++frames;
    }

    /* Print out some timing information */
    now = SDL_GetTicks();
    if (now > then) {
        double fps = ((double) frames * 1000) / (now - then);
        printf("%2.2f frames per second\n", fps);
    }

    quit(0);
    return 0;
}

/* vi: set ts=4 sw=4 expandtab: */
