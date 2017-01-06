/*
 * Copyright (c) 2016, Hugo Freire <hugo@exec.sh>.
 *
 * This source code is licensed under the license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef WONDERWALLD_DISPLAY_H
#define WONDERWALLD_DISPLAY_H

#define WONDERWALLD_DISPLAY_MAX_WIDTH 0;
#define WONDERWALLD_DISPLAY_MAX_HEIGHT 0;

void init_display(int width, int height);

void show_image(const char *filename, const unsigned int timeout);

void destroy_display();

#endif //WONDERWALLD_DISPLAY_H
