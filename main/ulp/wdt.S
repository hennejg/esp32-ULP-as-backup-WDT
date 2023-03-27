#include "sdkconfig.h"
#include "soc/rtc_cntl_reg.h"
#include "soc/rtc_io_reg.h"
#include "soc/soc_ulp.h"
#include "soc/sens_reg.h"

      .bss
      .global wdt_counter
wdt_counter:
      .long 0

      .global wdt_reset_occurred
wdt_reset_occurred:
      .long 0

      .text
      .global entry
entry:
      // decrement WDT counter
      move r0, wdt_counter
      ld r1, r0, 0
      sub r1, r1, 1
      st r1, r0, 0

      // jump if zero
      jump bark, EQ

      halt

// reset main CPU
bark:
      // track the fact that a reset was triggered
      move r0, wdt_reset_occurred
      move r1, 0xDEAD
      st r1, r0, 0x01

      WRITE_RTC_REG(RTC_CNTL_OPTIONS0_REG, RTC_CNTL_SW_SYS_RST_S, 1, 1)
      halt