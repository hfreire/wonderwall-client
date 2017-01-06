/*
 * Copyright (c) 2016, Hugo Freire <hugo@exec.sh>.
 *
 * This source code is licensed under the license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <SDL2/SDL_video.h>
#include <stdbool.h>
#include "display.h"

#include "SDL2/SDL.h"
#include "SDL2/SDL_image.h"

int display_width;
int display_height;

SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;

void print_available_renders() {
    int i = 0;
    int renders = SDL_GetNumRenderDrivers();

    fprintf(stdout, "Available display render drivers: %d\n", renders);

    while (i < renders) {
        SDL_RendererInfo rendererInfo;
        SDL_GetRenderDriverInfo(i, &rendererInfo);
        printf(" %d ", i);

        printf("%s (software=%d accelerated=%d, presentvsync=%d targettexture=%d)\n",
               rendererInfo.name,
               (rendererInfo.flags & SDL_RENDERER_SOFTWARE) != 0,
               (rendererInfo.flags & SDL_RENDERER_ACCELERATED) != 0,
               (rendererInfo.flags & SDL_RENDERER_PRESENTVSYNC) != 0,
               (rendererInfo.flags & SDL_RENDERER_TARGETTEXTURE) != 0);

        i++;
    }
}

void print_display_info() {
    SDL_DisplayMode displayMode;
    SDL_RendererInfo rendererInfo;

    if (SDL_GetWindowDisplayMode(window, &displayMode) != 0) {
        fprintf(stderr, "SDL_GetWindowDisplayMode failed: %s\n", SDL_GetError());

        return;
    }

    if (SDL_GetRendererInfo(renderer, &rendererInfo) != 0) {
        fprintf(stderr, "SDL_GetRendererInfo failed: %s\n", SDL_GetError());

        return;
    }

    fprintf(stdout, "Display using %s render with video mode %dx%d@%dHz\n",
            rendererInfo.name, display_width, display_height, displayMode.refresh_rate);
}

void show_black_screen() {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

    SDL_RenderClear(renderer);
    SDL_RenderPresent(renderer);
}

void init_display(int width, int height) {
    bool fullscreen = width == 0 && height == 0;
    SDL_DisplayMode displayMode;

    if (fullscreen) {
        fprintf(stdout, "Opening display in fullscreen\n");
    } else {
        fprintf(stdout, "Opening display with %d width and %d height\n", width, height);
    }

    SDL_SetHint(SDL_HINT_RENDER_DRIVER, "opengles2");
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");  // make the scaled rendering look smoother.

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "SDL_Init failed - %s\n", SDL_GetError());

        exit(1);
    }

    SDL_ShowCursor(SDL_DISABLE);

    if (SDL_CreateWindowAndRenderer(0, 0, SDL_WINDOW_FULLSCREEN_DESKTOP, &window, &renderer) !=
        0) { // opengles2 will always use window in fullscreen.
        print_available_renders();

        fprintf(stderr, "SDL_CreateWindowAndRenderer failed - %s\n", SDL_GetError());
        SDL_Quit();

        exit(1);
    }

    if (SDL_GetWindowDisplayMode(window, &displayMode) != 0) {
        fprintf(stderr, "SDL_GetWindowDisplayMode failed: %s\n", SDL_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();

        exit(1);
    }

    if (fullscreen) {
        display_width = displayMode.w;
        display_height = displayMode.h;
    } else {
        display_width = width;
        display_height = height;
    }

    show_black_screen();

    print_display_info();

    return;
}

void destroy_display() {
    if (renderer) {
        SDL_DestroyRenderer(renderer);
    }

    if (window) {
        SDL_DestroyWindow(window);
    }

    SDL_Quit();
}

void scale_image(SDL_Texture *texture, SDL_Rect *rectangle) {
    int width;
    int height;

    SDL_QueryTexture(texture, NULL, NULL, &width, &height);

    float vScale = (float) display_width / (float) width;
    float hScale = (float) display_height / (float) height;

    if (hScale < vScale) {
        rectangle->w = (int) (width * hScale);
        rectangle->h = (int) (height * hScale);

        rectangle->x = (int) (((float) display_width / 2) - ((float) rectangle->w / 2));
        rectangle->y = 0;
    } else {
        rectangle->w = (int) (width * vScale);
        rectangle->h = (int) (height * vScale);

        rectangle->x = 0;
        rectangle->y = (int) (((float) display_height / 2) - ((float) rectangle->h / 2));
    }
}

void show_image(const char *filename, const unsigned int timeout) {
    SDL_Rect rectangle;
    SDL_Texture *texture;

    texture = IMG_LoadTexture(renderer, filename);
    if (!texture) {
        fprintf(stderr, "IMG_Load failed - %s\n", IMG_GetError());
        IMG_Quit();

        return;
    }

    scale_image(texture, &rectangle);

    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, texture, NULL, &rectangle);
    SDL_RenderPresent(renderer);

    fprintf(stdout, "Showing %dx%d image for %u sec\n", rectangle.w, rectangle.h, timeout);

    SDL_Delay(timeout * 1000);

    show_black_screen();

    SDL_DestroyTexture(texture);
    IMG_Quit();

    return;
}