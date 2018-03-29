/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2018 Ruslan V. Uss (https://github.com/UncleRus)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
#include "i2c_utils.h"

#include <freertos/FreeRTOS.h>
#include <esp_log.h>

static const char *TAG = "I2C";

esp_err_t i2c_write_register(i2c_port_t i2c_num, uint8_t addr, uint8_t reg, void *data, size_t size)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, addr << 1, true);
    i2c_master_write_byte(cmd, reg, true);
    i2c_master_write(cmd, data, size, true);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(i2c_num, cmd, CONFIG_I2C_TIMEOUT / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);

    if (size == 1)
    {
        ESP_LOGD(TAG, "i2c_write_register(PORT = %d, ADDR = 0x%02x, REG = 0x%02x, VAL = 0x%02x): 0x%02x", i2c_num, addr, reg, *((uint8_t *)data), ret)
    }
    else if (size == 2)
    {
        ESP_LOGD(TAG, "i2c_write_register(PORT = %d, ADDR = 0x%02x, REG = 0x%02x, VAL = 0x%04x): 0x%02x", i2c_num, addr, reg, *((uint16_t *)data), ret)
    }
    else
    {
        ESP_LOGD(TAG, "i2c_write_register(PORT = %d, ADDR = 0x%02x, REG = 0x%02x, SIZE = %d): 0x%02x", i2c_num, addr, reg, size, ret);
    }

    return ret;
}

esp_err_t i2c_read_register(i2c_port_t i2c_num, uint8_t addr, uint8_t reg, void *res, size_t size)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, addr << 1, true);
    i2c_master_write_byte(cmd, reg, true);
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (addr << 1) | 1, true);
    i2c_master_read(cmd, res, size, I2C_MASTER_LAST_NACK);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(i2c_num, cmd, CONFIG_I2C_TIMEOUT / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);

    if (size == 1)
    {
        ESP_LOGD(TAG, "i2c_read_register(PORT = %d, ADDR = 0x%02x, REG = 0x%02x, VAL = 0x%02x): 0x%02x", i2c_num, addr, reg, *((uint8_t *)res), ret);
    }
    else if (size == 2)
    {
        ESP_LOGD(TAG, "i2c_read_register(PORT = %d, ADDR = 0x%02x, REG = 0x%02x, VAL = 0x%04x): 0x%02x", i2c_num, addr, reg, *((uint16_t *)res), ret);
    }
    else
    {
        ESP_LOGD(TAG, "i2c_read_register(PORT = %d, ADDR = 0x%02x, REG = 0x%02x, SIZE = %d): 0x%02x", i2c_num, addr, reg, size, ret);
    }

    return ret;
}

esp_err_t i2c_setup_master(i2c_port_t i2c_num, gpio_num_t scl_pin, gpio_num_t sda_pin, uint32_t clk_freq)
{
    i2c_config_t conf;
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = sda_pin;
    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    conf.scl_io_num = scl_pin;
    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    conf.master.clk_speed = clk_freq;

    esp_err_t res = i2c_param_config(i2c_num, &conf);
    if (res != ESP_OK)
        return res;
    return i2c_driver_install(i2c_num, conf.mode, 0, 0, 0);
}