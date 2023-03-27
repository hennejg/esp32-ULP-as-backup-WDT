#include "sdkconfig.h"
#include "soc/rtc_cntl_reg.h"
#include "soc/rtc_io_reg.h"
#include "soc/soc_ulp.h"
#include "soc/sens_reg.h"

      .bss
      .global wdt_counter
wdt_counter:
      .long 0 // init to zero - will underflow and thus not reset on the first invocation

      .text
      .global entry
entry:
      // decrement WDT counter
      move r0, wdt_counter
      ld r1, r0, 0
      sub r1, r1, 1
      st r1, r0, 0

      // dummy add to set ALU flags. jump if zero
      add r1, r1, 0
      jump bark, EQ

      halt

// reset main CPU
bark:
      // there is a macro for RTC_CNTL_SW_SYS_RST but we need the bit number
      // here, not the mask...
      WRITE_RTC_REG(RTC_CNTL_OPTIONS0_REG, 31, 1, 1)
      halt