#include "sdStorage.h"

// TAG used for sd storage
static const char TAG[] = "sdcard";

sdmmc_card_t *card;
sdmmc_host_t host = SDSPI_HOST_DEFAULT();

esp_err_t sd_card_config(void)
{
    esp_err_t ret;

    // Options for mounting the filesystem.
    // If format_if_mount_failed is set to true, SD card will be partitioned and
    // formatted in case when mounting fails.
    esp_vfs_fat_mount_config_t mount_config = {
#ifdef CONFIG_EXAMPLE_FORMAT_IF_MOUNT_FAILED
        .format_if_mount_failed = true,
#else
        .format_if_mount_failed = false,
#endif // EXAMPLE_FORMAT_IF_MOUNT_FAILED
        .max_files = 5,
        .allocation_unit_size = 8 * 1024 // 8192
    };

    ESP_LOGI(TAG, "Initializing SD card");

    // Use settings defined above to initialize SD card and mount FAT filesystem.
    // Note: esp_vfs_fat_sdmmc/sdspi_mount is all-in-one convenience functions.
    // Please check its source code and implement error recovery when developing
    // production applications.
    ESP_LOGI(TAG, "Using SPI peripheral");

    host.max_freq_khz = SDMMC_FREQ_PROBING;
    host.slot = SPI3_HOST;

    spi_dma_chan_t dma_ch = SPI_DMA_CH_AUTO;

    spi_bus_config_t bus_cfg = {
        .mosi_io_num = PIN_NUM_MOSI,
        .miso_io_num = PIN_NUM_MISO,
        .sclk_io_num = PIN_NUM_CLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 4000,
    };
    ret = spi_bus_initialize(host.slot, &bus_cfg, dma_ch);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to initialize bus.");
        return ret;
    }

    // This initializes the slot without card detect (CD) and write protect (WP) signals.
    // Modify slot_config.gpio_cd and slot_config.gpio_wp if your board has these signals.
    sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
    slot_config.gpio_cs = PIN_NUM_CS;
    slot_config.host_id = host.slot;

    ESP_LOGI(TAG, "Mounting filesystem");
    ret = esp_vfs_fat_sdspi_mount(MOUNT_POINT, &host, &slot_config, &mount_config, &card);

    if (ret != ESP_OK)
    {
        if (ret == ESP_FAIL)
        {
            ESP_LOGE(TAG, "Failed to mount filesystem "
                          "If you want the card to be formatted, set the CONFIG_EXAMPLE_FORMAT_IF_MOUNT_FAILED menuconfig option");
        }
        else
        {
            ESP_LOGE(TAG, "Failed to initialize the card (%s). "
                          "Make sure SD card lines have pull-up resistors in place.",
                     esp_err_to_name(ret));
        }
        return ret;
    }
    ESP_LOGI(TAG, "Filesystem mounted");

    // Card has been initialized, print its properties
    sdmmc_card_print_info(stdout, card);

    return ESP_OK;
}

esp_err_t create_file(const char *file)
{
    ESP_LOGI(TAG, "Creating file %s", file);

    FILE *f = fopen(file, "w");
    if (f == NULL)
    {
        ESP_LOGE(TAG, "Failed to create file");
        return ESP_FAIL;
    }
    // Write header
    fprintf(f, "Angle X,");
    fprintf(f, "Angle Y,");
    fprintf(f, "Angle Z,");
    fprintf(f, "Temperature,");
    fprintf(f, "\n");
    fclose(f);
    ESP_LOGI(TAG, "File created");

    return ESP_OK;
}

char* getContent(const char *file){

    ESP_LOGI(TAG, "Reading file %s", file);

    char *data[];
    char ch;
    int i = 0;
    // Opening file in reading mode
    FILE *f = fopen(file, "r");
    /*
    if (NULL == ptr) {
        printf("file can't be opened \n");
    }
    */
 
    // Printing what is written in file
    // character by character using loop.
    do {
        ch = fgetc(f);
        data[i] = ch;
        i++;
 
        // Checking if character is not EOF.
        // If it is EOF stop eading.
    } while (ch != '\0');

    // Closing the file
    fclose(f);

    return data;
}

esp_err_t write_data(const char *file, const char *data)
{
    ESP_LOGI(TAG, "Writing file %s", file);

    char file_path[50];

    sprintf(file_path,"%s%s", MOUNT_POINT, file);

    esp_err_t ret;

    ESP_LOGI(TAG, "Opening file %s", file_path);

    // Check if destination file exists before
    struct stat st;
    if (stat(file_path, &st) != 0)
    {
        // Create a new file
        ret = create_file(file_path);
        if (ret != ESP_OK)
            return ret;
    }
    char *content = getContent(file_path);

    FILE *f = fopen(file_path, "w");
    if (f == NULL)
    {
        ESP_LOGE(TAG, "Failed to open file for writing");
        return ESP_FAIL;
    }
    fprintf(f, "%s", content);
    fprintf(f, "%s", data);
    fclose(f);
    ESP_LOGI(TAG, "File written");
    
    // All done, unmount partition and disable SPI peripheral
    esp_vfs_fat_sdcard_unmount(MOUNT_POINT, card);
    ESP_LOGI(TAG, "Card unmounted");

    // deinitialize the bus after all devices are removed
    spi_bus_free(host.slot);

    return ESP_OK;
}

void card_test(void)
{

    vTaskDelay(1000 / portTICK_PERIOD_MS);
    sd_card_config();
    const char data[] = "24,45,2,22.42\n";

    write_data(FILE_NAME, data);

    /*
    vTaskDelay( 1000 / portTICK_PERIOD_MS);
    esp_err_t ret;

    // Options for mounting the filesystem.
    // If format_if_mount_failed is set to true, SD card will be partitioned and
    // formatted in case when mounting fails.
    // esp_vfs_fat_sdmmc_mount_config_t mount_config = {
    esp_vfs_fat_mount_config_t mount_config = {
        .format_if_mount_failed = false,
        .max_files = 5,
        .allocation_unit_size = 8 * 1024 // 8192
    };
    sdmmc_card_t *card;
    sdmmc_host_t host = SDSPI_HOST_DEFAULT();
    host.max_freq_khz = SDMMC_FREQ_PROBING;
    host.slot = SPI3_HOST;

    spi_dma_chan_t dma_chan = SPI_DMA_CH_AUTO;

    const char mount_point[] = MOUNT_POINT;
    ESP_LOGI(TAG, "Initializing SD card");

    // Use settings defined above to initialize SD card and mount FAT filesystem.
    // Note: esp_vfs_fat_sdmmc/sdspi_mount is all-in-one convenience functions.
    // Please check its source code and implement error recovery when developing
    // production applications.
    ESP_LOGI(TAG, "Using SPI peripheral");

    spi_bus_config_t bus_cfg = {
        .mosi_io_num = PIN_NUM_MOSI,
        .miso_io_num = PIN_NUM_MISO,
        .sclk_io_num = PIN_NUM_CLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 4000,
    };
    ret = spi_bus_initialize(host.slot, &bus_cfg, dma_chan);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize bus.");
        return;
    }

    // This initializes the slot without card detect (CD) and write protect (WP) signals.
    // Modify slot_config.gpio_cd and slot_config.gpio_wp if your board has these signals.
    sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
    slot_config.gpio_cs = PIN_NUM_CS;
    slot_config.host_id = host.slot;

    ESP_LOGI(TAG, "Mounting filesystem");
    ret = esp_vfs_fat_sdspi_mount(MOUNT_POINT, &host, &slot_config, &mount_config, &card);

    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE(TAG, "Failed to mount filesystem. "
                     "If you want the card to be formatted, set the CONFIG_EXAMPLE_FORMAT_IF_MOUNT_FAILED menuconfig option.");
        } else {
            ESP_LOGE(TAG, "Failed to initialize the card (%s). "
                     "Make sure SD card lines have pull-up resistors in place.", esp_err_to_name(ret));
        }
        return;
    }
    ESP_LOGI(TAG, "Filesystem mounted");

    // Card has been initialized, print its properties
    sdmmc_card_print_info(stdout, card);

    // Use POSIX and C standard library functions to work with files.

    // First create a file.
    const char *file_hello = MOUNT_POINT"/hello.txt";

    ESP_LOGI(TAG, "Opening file %s", file_hello);
    FILE *f = fopen(file_hello, "w");
    if (f == NULL) {
        ESP_LOGE(TAG, "Failed to open file for writing");
        return;
    }
    fprintf(f, "Hello %s!\n", card->cid.name);
    fclose(f);
    ESP_LOGI(TAG, "File written");

    const char *file_foo = MOUNT_POINT"/foo.txt";

    // Check if destination file exists before renaming
    struct stat st;
    if (stat(file_foo, &st) == 0) {
        // Delete it if it exists
        unlink(file_foo);
    }

    // Rename original file
    ESP_LOGI(TAG, "Renaming file %s to %s", file_hello, file_foo);
    if (rename(file_hello, file_foo) != 0) {
        ESP_LOGE(TAG, "Rename failed");
        return;
    }

    // Open renamed file for reading
    ESP_LOGI(TAG, "Reading file %s", file_foo);
    f = fopen(file_foo, "r");
    if (f == NULL) {
        ESP_LOGE(TAG, "Failed to open file for reading");
        return;
    }

    // Read a line from file
    char line[64];
    fgets(line, sizeof(line), f);
    fclose(f);

    // Strip newline
    char *pos = strchr(line, '\n');
    if (pos) {
        *pos = '\0';
    }
    ESP_LOGI(TAG, "Read from file: '%s'", line);

    // All done, unmount partition and disable SPI peripheral
    esp_vfs_fat_sdcard_unmount(mount_point, card);
    ESP_LOGI(TAG, "Card unmounted");

    //deinitialize the bus after all devices are removed
    spi_bus_free(host.slot);
    */
}
