/*
 * Copyright (c) 2016, Hugo Freire <hugo@exec.sh>.
 *
 * This source code is licensed under the license found in the
 * LICENSE file in the root directory of this source tree.
 */

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

void show_black_screen() {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

    SDL_RenderClear(renderer);
    SDL_RenderPresent(renderer);
}

void init_display(int width, int height) {
    print_available_renders();

    SDL_SetHint(SDL_HINT_RENDER_DRIVER, "software");
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");  // make the scaled rendering look smoother.

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "SDL_Init failed - %s\n", SDL_GetError());

        exit(1);
    }

    window = SDL_CreateWindow("Wonderwall", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height,
                              width == 0 && height == 0 ?
                              SDL_WINDOW_FULLSCREEN_DESKTOP :
                              SDL_WINDOW_SHOWN);
    if (!window) {
        fprintf(stderr, "SDL_CreateWindow failed - %s\n", SDL_GetError());
        SDL_Quit();

        exit(1);
    }

    SDL_GetWindowSize(window, &display_width, &display_height);

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) {
        fprintf(stderr, "SDL_CreateRenderer failed - %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();

        exit(1);
    }

    SDL_RendererInfo rendererInfo;
    if (SDL_GetRendererInfo(renderer, &rendererInfo) != 0) {
        fprintf(stderr, "SDL_GetRendererInfo failed: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();

        exit(1);
    }
    fprintf(stdout, "Display using %s for rendering\n", rendererInfo.name);

    SDL_RenderSetLogicalSize(renderer, width, height);

    show_black_screen();

    return;
}

void destroy_display() {
    SDL_DestroyRenderer(renderer);
    //SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(window);
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

    fprintf(stdout, "Showing image for %u sec\n", timeout);

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

    SDL_Delay(timeout * 1000);

    show_black_screen();

    SDL_DestroyTexture(texture);
    IMG_Quit();

    return;
}