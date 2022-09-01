#ifndef NVSSTORAGE_H
#define NVSSTORAGE_H

#include "esp_log.h"
#include "nvs_flash.h"

esp_err_t nvs_save_interval(const char *interval);
int16_t nvs_load_interval();

#endif