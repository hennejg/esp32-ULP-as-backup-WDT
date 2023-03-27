#include "driver/rtc_io.h"
#include "esp_sleep.h"
#include "soc/rtc_cntl_reg.h"
#include "soc/rtc_periph.h"
#include "soc/sens_reg.h"
#include <stdarg.h>
#include <stdio.h>

#if CONFIG_IDF_TARGET_ESP32
#include "esp32/ulp.h"
#elif CONFIG_IDF_TARGET_ESP32S2
#include "esp32s2/ulp.h"
#elif CONFIG_IDF_TARGET_ESP32S3
#include "esp32s3/ulp.h"
#endif

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "ulp_main.h"

extern const uint8_t ulp_main_bin_start[] asm("_binary_ulp_main_bin_start");
extern const uint8_t ulp_main_bin_end[] asm("_binary_ulp_main_bin_end");

void reset_ulp_wdt() {
  ulp_wdt_counter = 100; // 10 s
}

static void init_ulp_program(void) {
  esp_err_t err = ulp_load_binary(0, ulp_main_bin_start,
                                  (ulp_main_bin_end - ulp_main_bin_start) /
                                      sizeof(uint32_t));
  ESP_ERROR_CHECK(err);

  reset_ulp_wdt();

  /*
   * Set ULP wake up period to T = 20ms.
   * Minimum pulse width has to be T * (ulp_debounce_counter + 1) = 100ms.
   */
  ulp_set_wakeup_period(0, 100 * 1000);

  /* Start the program */
  err = ulp_run(&ulp_entry - RTC_SLOW_MEM);
  ESP_ERROR_CHECK(err);
}

int log_printf(const char *format, ...) {
  int len;
  va_list arg;
  va_list copy;
  va_start(arg, format);
  va_copy(copy, arg);
  len = vprintf(format, copy);
  va_end(copy);
  return len;
}

void app_main(void) {
  init_ulp_program();

  // be well-behaved for 10s
  for (int i = 0; i < 10; i++) {
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    log_printf("Resetting aux wdt\n");
    reset_ulp_wdt();
  }

  // now fail to reset the aux WDT
  while (true) {
    log_printf("No longer resetting aux wdt, counter is: %d\n",
               (int16_t)(ulp_wdt_counter & UINT16_MAX));
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}
