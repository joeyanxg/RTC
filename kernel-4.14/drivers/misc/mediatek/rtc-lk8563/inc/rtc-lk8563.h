/*
* Copyright (C) 2016 MediaTek Inc.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License version 2 as
* published by the Free Software Foundation.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
* See http://www.gnu.org/licenses/gpl-2.0.html for more details.
*/


#ifndef __RTC_LK8653_H__
#define __RTC_LK8653_H__

#include <linux/i2c.h>
#include <linux/irq.h>
#include <linux/uaccess.h>
#include <linux/kobject.h>
#include <linux/types.h>
#include <linux/atomic.h>
#include <linux/io.h>
#include <linux/sched.h>
//#include <linux/wakelock.h>
#include <linux/interrupt.h>
#include <linux/miscdevice.h>
#include <linux/platform_device.h>
#include <linux/input.h>
#include <linux/workqueue.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/module.h>
#include <linux/poll.h>
#include <linux/kernel.h>
#include <linux/pm.h>
#include <linux/pm_wakeup.h>
#include <linux/reboot.h>
#include <linux/regmap.h>
#include <linux/rtc.h>
#include <linux/sched/clock.h>
#include <linux/spinlock.h>
#include <linux/types.h>
#include <linux/bitops.h>
#include <linux/init.h>


#include <mach/upmu_hw.h>

//#include <mtk_rtc_hw.h>
//#include <mtk_rtc_hal_common.h>
#include <mach/mtk_pmic_wrap.h>
#include <upmu_common.h>

#define LK8563_TAG                  "[LK8563]"
#define LK8563_LOGLEVEL 1

#if ((LK8563_LOGLEVEL) >= 1)
#define LK8563_LOG(fmt, args...)    printk(LK8563_TAG fmt, ##args)
#else
#define LK8563_LOG(fmt, args...)    printk(LK8563_TAG fmt, ##args)
#endif

#define RTC_NAME	"LK8563"

/* we map HW YEA 0 (2000) to 1968 not 1970 because 2000 is the leap year */
#define RTC_MIN_YEAR		1968
#define RTC_NUM_YEARS		128
/* #define RTC_MAX_YEAR		(RTC_MIN_YEAR + RTC_NUM_YEARS - 1) */
/*
 * Reset to default date if RTC time is over 2038/1/19 3:14:7
 * Year (YEA)        : 1970 ~ 2037
 * Month (MTH)       : 1 ~ 12
 * Day of Month (DOM): 1 ~ 31
 */

#define I2C_RTC_BASE_YEAR		1900

#define I2C_RTC_LK8563_CONSTA1           0x0 //contorl/state R1
#define I2C_RTC_LK8563_CONSTA1           0x1 //contorl/state R2
#define I2C_RTC_LK8563_CLKOUT            0xD //CLKOUT freq Reg
#define I2C_RTC_LK8563_TIMER             0xE //Timer control Reg
#define I2C_RTC_LK8563_TIMER_COUNTDOWN   0xF //Timer countdown Reg

#define I2C_RTC_LK8563_SEC               0x2 //rtc second Reg
#define I2C_RTC_LK8563_MIN               0x3 //rtc minute Reg
#define I2C_RTC_LK8563_HOUR              0x4 //rtc hour Reg
#define I2C_RTC_LK8563_DAY               0x5 //rtc day Reg
#define I2C_RTC_LK8563_WEEK              0x6 //rtc week Reg
#define I2C_RTC_LK8563_MON               0x7 //rtc month and century Reg
#define I2C_RTC_LK8563_YEAR              0x8 //rtc year Reg

#define I2C_RTC_LK8563_MIN_ALARM         0x9 //rtc minute alarm Reg
#define I2C_RTC_LK8563_HOUR_ALARM        0xA //rtc minute alarm Reg
#define I2C_RTC_LK8563_DAY_ALARM         0xB //rtc minute alarm Reg
#define I2C_RTC_LK8563_WEEK_ALARM        0xC //rtc minute alarm Reg


#define RTC_BBPU		(PMIC_PWREN_ADDR)
#define RTC_BBPU_PWREN		(PMIC_PWREN_MASK << PMIC_PWREN_SHIFT)
#define RTC_BBPU_CLR		(PMIC_BBPU_CLR_MASK << PMIC_BBPU_CLR_SHIFT)
#define RTC_BBPU_INIT		(PMIC_BBPU_INIT_MASK << PMIC_BBPU_INIT_SHIFT)
#define RTC_BBPU_AUTO		(PMIC_AUTO_MASK << PMIC_AUTO_SHIFT)
#define RTC_BBPU_CLRPKY		(PMIC_CLRPKY_MASK << PMIC_CLRPKY_SHIFT)
#define RTC_BBPU_RELOAD		(PMIC_RELOAD_MASK << PMIC_RELOAD_SHIFT)
#define RTC_BBPU_CBUSY		(PMIC_CBUSY_MASK << PMIC_CBUSY_SHIFT)
#define RTC_BBPU_KEY		(0x43 << PMIC_KEY_BBPU_SHIFT)

#define RTC_WRTGR		(PMIC_WRTGR_ADDR)

#define RTC_POWERKEY1		(PMIC_RTC_POWERKEY1_ADDR)
#define RTC_POWERKEY2		(PMIC_RTC_POWERKEY2_ADDR)
#define RTC_POWERKEY1_KEY	0xa357
#define RTC_POWERKEY2_KEY	0x67d2

#define RTC_IRQ_EN		(PMIC_AL_EN_ADDR)
#define RTC_IRQ_EN_AL		(PMIC_AL_EN_MASK << PMIC_AL_EN_SHIFT)
#define RTC_IRQ_EN_TC		(PMIC_TC_EN_MASK << PMIC_TC_EN_SHIFT)
#define RTC_IRQ_EN_ONESHOT	(PMIC_ONESHOT_MASK << PMIC_ONESHOT_SHIFT)
#define RTC_IRQ_EN_LP		(PMIC_LP_EN_MASK << PMIC_LP_EN_SHIFT)
#define RTC_IRQ_EN_ONESHOT_AL	(RTC_IRQ_EN_ONESHOT | RTC_IRQ_EN_AL)

#endif