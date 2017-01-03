/*
 * Copyright (c) 2016, Hugo Freire <hugo@exec.sh>.
 *
 * This source code is licensed under the license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "display.h"

#include "SDL.h"
#include "SDL_image.h"

void display_image(const char *filename, const unsigned int timeout) {
    SDL_Surface *screen = NULL;
    SDL_Surface *image = NULL;
    const SDL_VideoInfo *videoInfo = NULL;

    fprintf(stdout, "Displaying image for %u sec\n", timeout);

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "SDL_Init failed - %s\n", SDL_GetError());

        return;
    }

    videoInfo = SDL_GetVideoInfo();

    if (videoInfo == 0) {
        fprintf(stderr, "SDL_GetVideoInfo failed - %s\n", SDL_GetError());
        SDL_Quit();

        return;
    }

    image = IMG_Load(filename);

    if (!image) {
        fprintf(stderr, "IMG_Load failed - %s\n", IMG_GetError());
        SDL_Quit();

        return;
    }

    screen = SDL_SetVideoMode(image->w, image->h, videoInfo->vfmt->BitsPerPixel, SDL_HWSURFACE);

    if (!screen) {
        fprintf(stderr, "SetVideoMode failed - %s\n", SDL_GetError());
        SDL_FreeSurface(image);
        SDL_Quit();

        return;
    }

    SDL_BlitSurface(image, 0, screen, 0);

    SDL_Delay(timeout * 1000);

    SDL_FreeSurface(image);

    SDL_Quit();

    return;
}