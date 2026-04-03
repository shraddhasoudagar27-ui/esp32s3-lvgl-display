#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "lvgl.h"

#include "driver/spi_master.h"
#include "driver/gpio.h"

#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"

/* ================= PIN CONFIG ================= */
#define PIN_NUM_MOSI 11
#define PIN_NUM_CLK  12
#define PIN_NUM_CS   10
#define PIN_NUM_DC   9
#define PIN_NUM_RST  8
#define PIN_NUM_BL   7

#define LCD_H_RES 172
#define LCD_V_RES 320

/* ================= GLOBAL ================= */
esp_lcd_panel_handle_t panel_handle = NULL;

/* ================= LCD INIT ================= */
void lcd_init()
{
    spi_bus_config_t buscfg = {
        .sclk_io_num = PIN_NUM_CLK,
        .mosi_io_num = PIN_NUM_MOSI,
        .miso_io_num = -1,
        .max_transfer_sz = LCD_H_RES * LCD_V_RES * 2 + 8,
    };

    ESP_ERROR_CHECK(spi_bus_initialize(SPI2_HOST, &buscfg, SPI_DMA_CH_AUTO));

    esp_lcd_panel_io_spi_config_t io_config = {
        .dc_gpio_num = PIN_NUM_DC,
        .cs_gpio_num = PIN_NUM_CS,
        .pclk_hz = 20 * 1000 * 1000,
        .lcd_cmd_bits = 8,
        .lcd_param_bits = 8,
        .spi_mode = 0,
        .trans_queue_depth = 10,
    };

    esp_lcd_panel_io_handle_t io_handle = NULL;
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi(
        (esp_lcd_spi_bus_handle_t)SPI2_HOST,
        &io_config,
        &io_handle));

    esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = PIN_NUM_RST,
        .bits_per_pixel = 16,
    };

    ESP_ERROR_CHECK(esp_lcd_new_panel_st7789(
        io_handle,
        &panel_config,
        &panel_handle));

    ESP_ERROR_CHECK(esp_lcd_panel_reset(panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_init(panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(panel_handle, true));

    // orientation and panel geometry for 172x320 ST7789
    esp_lcd_panel_swap_xy(panel_handle, false);
    esp_lcd_panel_mirror(panel_handle, false, false);
    esp_lcd_panel_set_gap(panel_handle, 34, 0);

    /* Backlight ON */
    gpio_set_direction(PIN_NUM_BL, GPIO_MODE_OUTPUT);
    gpio_set_level(PIN_NUM_BL, 1);
}

/* ================= LVGL FLUSH ================= */
void my_flush_cb(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map)
{
    esp_lcd_panel_draw_bitmap(panel_handle,
                              area->x1,
                              area->y1,
                              area->x2 + 1,
                              area->y2 + 1,
                              px_map);
    lv_display_flush_ready(disp);
}

/* ================= LVGL INIT ================= */
void lvgl_init_display()
{
    static lv_color_t buf[LCD_H_RES * LCD_V_RES];

    lv_display_t *disp = lv_display_create(LCD_H_RES, LCD_V_RES);
    lv_display_set_default(disp);
    lv_display_set_color_format(disp, LV_COLOR_FORMAT_RGB565_SWAPPED);
    lv_display_set_flush_cb(disp, my_flush_cb);
    lv_display_set_buffers(disp, buf, NULL,
                           sizeof(buf),
                           LV_DISPLAY_RENDER_MODE_FULL);
}

/* ================= UI ================= */
void create_ui()
{
    lv_obj_t *scr = lv_scr_act();
    lv_obj_set_style_bg_color(scr, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, 0);

    /* ===== BATTERY ===== */
    lv_obj_t *battery_label = lv_label_create(scr);
    lv_label_set_text(battery_label, "68%");
    lv_obj_set_style_text_color(battery_label, lv_color_white(), 0);
    lv_obj_set_style_text_font(battery_label, &lv_font_montserrat_14, 0);
    lv_obj_align(battery_label, LV_ALIGN_TOP_RIGHT, -12, 10);

    /* ===== TIME ===== */
    lv_obj_t *time_label = lv_label_create(scr);
    lv_label_set_text(time_label, "20\n34");
    lv_obj_set_style_text_color(time_label, lv_color_white(), 0);
    lv_obj_set_style_text_font(time_label, &lv_font_montserrat_48, 0);
    lv_obj_set_style_text_align(time_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_line_space(time_label, 4, 0);
    lv_obj_align(time_label, LV_ALIGN_TOP_MID, 0, 30);

    /* ===== DATE + DAY ROW ===== */
    lv_obj_t *date_row = lv_obj_create(scr);
    lv_obj_set_size(date_row, LCD_H_RES - 32, 30);
    lv_obj_set_style_bg_opa(date_row, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(date_row, 0, 0);
    lv_obj_set_style_pad_all(date_row, 0, 0);
    lv_obj_set_flex_flow(date_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(date_row, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_gap(date_row, 10, 0);
    lv_obj_align(date_row, LV_ALIGN_TOP_MID, 0, 148);

    lv_obj_t *date_label = lv_label_create(date_row);
    lv_label_set_text(date_label, "05/04");
    lv_obj_set_style_text_color(date_label, lv_color_white(), 0);
    lv_obj_set_style_text_font(date_label, &lv_font_montserrat_14, 0);

    lv_obj_t *day_label = lv_label_create(date_row);
    lv_label_set_text(day_label, "WED");
    lv_obj_set_style_text_color(day_label, lv_color_white(), 0);
    lv_obj_set_style_text_font(day_label, &lv_font_montserrat_14, 0);

    /* ===== STEPS CARD ===== */
    lv_obj_t *steps_card = lv_obj_create(scr);
    lv_obj_set_size(steps_card, LCD_H_RES - 24, 84);
    lv_obj_set_style_bg_color(steps_card, lv_color_hex(0x101820), 0);
    lv_obj_set_style_bg_opa(steps_card, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(steps_card, 0, 0);
    lv_obj_set_style_radius(steps_card, 22, 0);
    lv_obj_set_style_pad_all(steps_card, 14, 0);
    lv_obj_align(steps_card, LV_ALIGN_TOP_MID, 0, 176);

    lv_obj_t *steps_text = lv_label_create(steps_card);
    lv_label_set_text(steps_text, "5600 / 8000 steps");
    lv_obj_set_style_text_color(steps_text, lv_color_white(), 0);
    lv_obj_set_style_text_font(steps_text, &lv_font_montserrat_14, 0);
    lv_obj_align(steps_text, LV_ALIGN_TOP_LEFT, 0, 0);

    lv_obj_t *progress_bar = lv_bar_create(steps_card);
    lv_obj_set_width(progress_bar, LCD_H_RES - 52);
    lv_obj_set_height(progress_bar, 18);
    lv_obj_align(progress_bar, LV_ALIGN_BOTTOM_MID, 0, -6);
    lv_bar_set_range(progress_bar, 0, 8000);
    lv_bar_set_value(progress_bar, 5600, LV_ANIM_OFF);
    lv_obj_set_style_radius(progress_bar, 12, LV_PART_MAIN);
    lv_obj_set_style_bg_color(progress_bar, lv_color_hex(0x333333), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(progress_bar, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_radius(progress_bar, 12, LV_PART_INDICATOR);
    lv_obj_set_style_bg_color(progress_bar, lv_color_hex(0x00C853), LV_PART_INDICATOR);
    lv_obj_set_style_bg_opa(progress_bar, LV_OPA_COVER, LV_PART_INDICATOR);

    /* ===== BOTTOM STATS ROW ===== */
    lv_obj_t *stats_row = lv_obj_create(scr);
    lv_obj_set_size(stats_row, LCD_H_RES - 32, 30);
    lv_obj_set_style_bg_opa(stats_row, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(stats_row, 0, 0);
    lv_obj_set_style_pad_all(stats_row, 0, 0);
    lv_obj_set_flex_flow(stats_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(stats_row, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_gap(stats_row, 8, 0);
    lv_obj_align(stats_row, LV_ALIGN_BOTTOM_MID, 0, -16);

    lv_obj_t *left_stat = lv_label_create(stats_row);
    lv_label_set_text(left_stat, "418 kcal");
    lv_obj_set_style_text_color(left_stat, lv_color_white(), 0);
    lv_obj_set_style_text_font(left_stat, &lv_font_montserrat_14, 0);

    lv_obj_t *right_stat = lv_label_create(stats_row);
    lv_label_set_text(right_stat, "3.50 km");
    lv_obj_set_style_text_color(right_stat, lv_color_white(), 0);
    lv_obj_set_style_text_font(right_stat, &lv_font_montserrat_14, 0);
}

/* ================= LVGL TICK TASK ================= */
static void lvgl_tick_task(void *arg)
{
    (void)arg;
    while (1) {
        lv_tick_inc(10);
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

/* ================= LVGL APP TASK ================= */
static void lvgl_app_task(void *arg)
{
    (void)arg;

    lv_init();
    lcd_init();
    lvgl_init_display();
    create_ui();

    xTaskCreate(lvgl_tick_task, "lvgl_tick", 2048, NULL, 2, NULL);

    while (1) {
        lv_timer_handler();
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

/* ================= MAIN ================= */
void app_main()
{
    xTaskCreate(lvgl_app_task, "lvgl_app", 16384, NULL, 5, NULL);
    vTaskDelete(NULL);
}