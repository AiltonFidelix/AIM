#include "sdStorage.h"

// TAG used for sd storage
static const char TAG[] = "sdcard";

// Card and Host global vars
sdmmc_card_t *card;
sdmmc_host_t host = SDSPI_HOST_DEFAULT();

/**
 * @brief Config the SPI bus and mount the sdcard
 * @param void
 * @return ESP_OK if everything is ok, otherwise ESP_FAIL
 */
esp_err_t sdcard_config(void)
{
    esp_err_t ret;

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
    
    return sdcard_mount();
}

/**
 * @brief Mount sdcard at device
 * @param void
 * @return ESP_OK if everything is ok, otherwise ESP_FAIL
 */
esp_err_t sdcard_mount(void)
{
    esp_err_t ret;

    ESP_LOGI(TAG, "Mounting filesystem");

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

    // This initializes the slot without card detect (CD) and write protect (WP) signals.
    // Modify slot_config.gpio_cd and slot_config.gpio_wp if your board has these signals.
    sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
    slot_config.gpio_cs = PIN_NUM_CS;
    slot_config.host_id = host.slot;

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

/**
 * @brief Unmount sdcard at device
 * @param void
 * @return ESP_OK if everything is ok, otherwise ESP_FAIL
 */
esp_err_t sdcard_unmount(void)
{
    esp_err_t ret;
    // All done, unmount partition and disable SPI peripheral
    ret = esp_vfs_fat_sdcard_unmount(MOUNT_POINT, card);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to unmounted. Error: %s", esp_err_to_name(ret));
        return ret;
    }
    ESP_LOGI(TAG, "Card unmounted");

    // deinitialize the bus after all devices are removed
    return spi_bus_free(host.slot);
}

/**
 * @brief Create a new file in the mount path
 * @param file name of the file
 * @return ESP_OK if everything is ok, otherwise ESP_FAIL
 */
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

/**
 * @brief Write data in the file
 * @param data to be write
 * @param file name of the file
 * @return ESP_OK if everything is ok, otherwise ESP_FAIL
 */
esp_err_t write_data(const char *file, const char *data)
{
    ESP_LOGI(TAG, "Writing file %s", file);

    char file_path[50];

    sprintf(file_path, "%s%s", MOUNT_POINT, file);

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
    // char *content = getContent(file_path);

    FILE *f = fopen(file_path, "a");
    if (f == NULL)
    {
        ESP_LOGE(TAG, "Failed to open file for writing");
        return ESP_FAIL;
    }
    fprintf(f, "%s", data);
    fclose(f);
    ESP_LOGI(TAG, "File written");

    return ESP_OK;
}

/**
 * @brief Function to test sdcard write
 */
void sdcard_test(void)
{
    ESP_LOGI(TAG, "Testing sdcard...");

    sdcard_config();

    char data[50] = "22,24,48,18\n";

    write_data(FILE_NAME, data);

    memset(data, 0x00, sizeof(data));

    for (int i = 0; i < 100; i++)
    {
        sprintf(data, "%d,%d,%d,%d\n", i + 4, i, i - 2, i + 15);
        write_data(FILE_NAME, data);
    }
}
