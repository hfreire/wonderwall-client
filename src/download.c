//
// Created by Hugo Freire on 2016-12-12.
//

#include "download.h"

#include <curl/curl.h>

int download_image(const char *url, const char *filename) {
    FILE *file;
    CURL *curl;
    CURLcode code;

    if (!(file = fopen(filename, "wb"))) {
        fprintf(stderr, "error: could not open temporary file\n");

        return -1;
    }

    curl = curl_easy_init();
    if (!curl) {
        fprintf(stderr, "error: curl failed to initialize\n");

        return -1;
    }

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, NULL);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, file);

    code = curl_easy_perform(curl);
    if (code) {
        fprintf(stderr, "error: curl failed to download URL with error code %d\n", code);

        curl_easy_cleanup(curl);
        fclose(file);

        return -1;
    }

    curl_easy_cleanup(curl);
    fclose(file);

    return 0;
}