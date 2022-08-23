#ifndef SDSTORAGE_H
#define SDSTORAGE_H

#include <string.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include "esp_vfs_fat.h"
#include "esp_log.h"
#include "sdmmc_cmd.h"

#define MOUNT_POINT "/logs"
#define FILE_NAME "/logs.csv"

// You can also change the pin assignments here by changing the following 4 lines.
#define PIN_NUM_MISO GPIO_NUM_19  // 19
#define PIN_NUM_MOSI GPIO_NUM_23 // 23
#define PIN_NUM_CLK GPIO_NUM_18  // 18
#define PIN_NUM_CS GPIO_NUM_5

esp_err_t sd_card_config(void);
esp_err_t create_file(const char *file);
esp_err_t write_data(const char *file, const char *data);

void card_test(void);

#endif