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

#include "inc/rtc-lk8563.h"

//static int lk8563_init_flag =-1; // 0<==>OK -1 <==> fail

struct lk8563_rtc {
	struct i2c_client *i2c;
	struct rtc_device	*rtc_dev;
	//int			irq;
	//u32			addr_base;
	spinlock_t	lock;
	//struct regmap		*regmap;
};

static int offset_rtc_year = 0;
static struct lk8563_rtc *lk_rtc;
//static DEFINE_SPINLOCK(lk_rtc_lock);
static int lk_rtc_show_time;
static int lk_rtc_show_alarm = 1;
module_param(lk_rtc_show_time, int, 0644);
module_param(lk_rtc_show_alarm, int, 0644);

static u16 lk8563_bcd2o(u16 value, u8 type)
{
	char lowBit, highBit;
	u16 ret;
	switch (type)
	{
		case I2C_RTC_LK8563_SEC:
		case I2C_RTC_LK8563_MIN:
			lowBit = value & 0x0f;
			highBit = (value & 0x70) >> 4;
			break;
		case I2C_RTC_LK8563_HOUR:
		case I2C_RTC_LK8563_DAY:
			lowBit = value & 0x0f;
			highBit = (value & 0x30) >> 4;
			break;
		case I2C_RTC_LK8563_WEEK:
			lowBit = value & 0x07;
			highBit = 0;
			break;
		case I2C_RTC_LK8563_MON:
			lowBit = value & 0x0f;
			highBit = (value & 0x10) >> 4;
			break;
		case I2C_RTC_LK8563_YEAR:
			lowBit = value & 0x0f;
			highBit = (value & 0xf0) >> 4;
		default:
			break;
	}
	ret = highBit*10+lowBit;
	return ret;
}

static u16 lk8563_o2bcd(u16 value)
{
	u16 ret;
	ret = (value / 10) * 16 + (value % 10);

	return ret;
}

static int lk8563_read(u8 reg, u8 *rt_value, struct i2c_client *client)
{
	int ret;
	u8 read_cmd[3] = {0};
	u8 cmd_len = 0;
	
	read_cmd[0] = reg;
	cmd_len = 1;
	//printk("%s enter lk8563_read = %x\n",__func__,reg);
	if (client->adapter == NULL)
		pr_err("lk8563_read client->adapter==NULL\n");
	
	ret = i2c_master_send(client, read_cmd, cmd_len);
	if (ret != cmd_len) {
		pr_err("lk8563_read error1\n");
		return -1;
	}
	//printk("%s client->addr =%x\n",__func__,client->addr);
	ret = i2c_master_recv(client, rt_value, 1);
	if (ret != 1) {
		pr_err("lk8563_read error2, ret = %d.\n", ret);
		return -1;
	}
	
	return 0;
}

static int lk8563_write(u8 reg, unsigned char value, struct i2c_client *client)
{
	int ret = 0;
	u8 write_cmd[2] = {0};
	
	write_cmd[0] = reg;
	write_cmd[1] = value;

	ret = i2c_master_send(client, write_cmd, 2);
	printk("lk8563_write cilent addr 0x%x complete->[REG-0x%02x,val-0x%02x] ret %d\n",client->addr,reg,value,ret);
	
	if (ret != 2) {
		pr_err("lk8563_write cilent addr 0x%x error->[REG-0x%02x,val-0x%02x] ret %d\n",client->addr,reg,value,ret);
		return -1;
	}
	
	return 0;
}
#if 0
u16 lk8563_rtc_read(u16 addr)
{
	u32 rdata = 0;

	pwrap_read((u32) addr, &rdata);
	return (u16) rdata;
}

void lk8563_rtc_write(u16 addr, u16 data)
{
	pwrap_write((u32) addr, (u32) data);
}

void lk8563_rtc_busy_wait(void)
{
	unsigned long timeout = /*sched_clock()*/jiffies + 500000000;

	do {
		if ((lk8563_rtc_read(RTC_BBPU) & RTC_BBPU_CBUSY) == 0)
			break;
		if (time_after(jiffies, timeout)) {
			pr_err("%s, wait cbusy timeout, %x, %x, %x, %d\n",
				__func__,
				lk8563_rtc_read(RTC_BBPU), lk8563_rtc_read(RTC_POWERKEY1),
				lk8563_rtc_read(RTC_POWERKEY2), lk8563_rtc_read(I2C_RTC_LK8563_SEC));
			break;
		}
		cpu_relax();
	} while (1);
}

void lk8563_rtc_write_trigger(void)
{
	lk8563_rtc_write(RTC_WRTGR, 1);
	lk8563_rtc_busy_wait();
}

static void lk_rtc_set_lp_irq(void)
{
	LK8563_LOG("%s\n", __func__);
	u16 irqen;

	irqen = lk8563_rtc_read(RTC_IRQ_EN) | RTC_IRQ_EN_LP;

	lk8563_rtc_write(RTC_IRQ_EN, irqen);
	lk8563_rtc_write_trigger();
}

static void lk_rtc_irq_handler(void)
{
	LK8563_LOG("%s\n", __func__);
}

static void lk8563_rtc_read_time(struct i2c_client *client, struct rtc_time *tm)
{
	u16 bbpu;
	LK8563_LOG("%s\n", __func__);
	bbpu = lk8563_rtc_read(RTC_BBPU) | RTC_BBPU_KEY | RTC_BBPU_RELOAD;
	lk8563_rtc_write(RTC_BBPU, bbpu);
	lk8563_rtc_write_trigger();
	lk8563_rtc_get_tick(client, tm);
	bbpu = lk8563_rtc_read(RTC_BBPU) | RTC_BBPU_KEY | RTC_BBPU_RELOAD;
	lk8563_rtc_write(RTC_BBPU, bbpu);
	lk8563_rtc_write_trigger();
	/*
	if (lk8563_rtc_read(I2C_RTC_LK8563_SEC) < tm->tm_sec) {
		lk8563_rtc_get_tick(client, tm);
	}*/
}

#endif
static void lk8563_convert(struct rtc_time *tm)
{
	//LK8563_LOG("%s\n", __func__);
	if(!(tm->tm_mon & 0x80)) //mon reg bit7=0,the century is 20xx
	{
		tm->tm_year = lk8563_bcd2o(tm->tm_year, I2C_RTC_LK8563_YEAR) + 2000 + offset_rtc_year;
		//LK8563_LOG("MON BIT 7 = 0, offset_rtc_year = %d\n", offset_rtc_year);
	}
	else
	{
		tm->tm_year = lk8563_bcd2o(tm->tm_year, I2C_RTC_LK8563_YEAR) + 1900 - offset_rtc_year;
		//LK8563_LOG("MON BIT 7 = 1, offset_rtc_year = %d\n", offset_rtc_year);
	}

	//VL=0：保证准确的时钟/日历数据
	//VL=1：不保证准确的时钟/日历数据
	//LK8563_LOG("first read VL=%d",(tm->tm_sec & 0x80) >> 7);
	
	//世纪位：C=0 指定世纪数为 20XX；C=1 指定世纪数为 19XX，“XX”为年寄存器中的值
	//LK8563_LOG("first read C=%d",(tm->tm_mon & 0x80) >> 7);

	tm->tm_sec = lk8563_bcd2o(tm->tm_sec, I2C_RTC_LK8563_SEC);
	tm->tm_min = lk8563_bcd2o(tm->tm_min, I2C_RTC_LK8563_MIN);
	tm->tm_hour = lk8563_bcd2o(tm->tm_hour, I2C_RTC_LK8563_HOUR);
	tm->tm_wday = lk8563_bcd2o(tm->tm_wday, I2C_RTC_LK8563_WEEK);
	tm->tm_mday = lk8563_bcd2o(tm->tm_mday, I2C_RTC_LK8563_DAY);
	tm->tm_mon = lk8563_bcd2o(tm->tm_mon, I2C_RTC_LK8563_MON);
	
/*	LK8563_LOG("lk8563_convert read tm-> = %04d/%02d/%02d (%d) %02d:%02d:%02d\n",
	  tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
	  tm->tm_wday, tm->tm_hour, tm->tm_min, tm->tm_sec);*/
}

static void lk8563_convert_bcd(struct i2c_client *client, struct rtc_time *tm)
{
	//LK8563_LOG("%s\n", __func__);
	u8 th = 0, hu = 0;
	tm->tm_year += I2C_RTC_BASE_YEAR;
	tm->tm_mon += 1;

	if(tm->tm_year >= 2000 && tm->tm_year < 10000 ) //mon reg bit7=0,the century is 20xx, 最大设置不超过10000年
	{
		tm->tm_mon = lk8563_o2bcd(tm->tm_mon);
		tm->tm_year -= 2000;
		if((tm->tm_year / 1000) > 0)
		{
			th = tm->tm_year / 1000;
			tm->tm_year = tm->tm_year % 1000;
		}
		if((tm->tm_year / 100) > 0)
		{
			hu = tm->tm_year / 100;
			tm->tm_year = tm->tm_year % 100;
		}
	}
	else if(tm->tm_year >=0 && tm->tm_year < 2000) //最小设置为0年，不能为负数
	{
		tm->tm_mon = lk8563_o2bcd(tm->tm_mon) + 0x80;
		tm->tm_year -= 1900;
		if(tm->tm_year < 0) //设置的年份小于1900年
		{
			tm->tm_year += 1900;
			if((tm->tm_year / 100) > 10)
			{
				th = 0;
				hu = (2000 - tm->tm_year) / 100;
				tm->tm_year = tm->tm_year % 100;
				if(tm->tm_year == 0)
					hu -= 1;
			}
			else //设置的年份小于1000年
			{
				th = 1;
				hu = (1000 - tm->tm_year) / 100;
				tm->tm_year = tm->tm_year % 100;
				if(tm->tm_year == 0)
					hu -= 1;
			}
		}
	}
	else
	{
		LK8563_LOG("set time is invalid!!");
		return;
	}
	offset_rtc_year = th * 1000 + hu * 100;

	tm->tm_sec = lk8563_o2bcd(tm->tm_sec);
	tm->tm_min = lk8563_o2bcd(tm->tm_min);
	tm->tm_hour = lk8563_o2bcd(tm->tm_hour);
	tm->tm_wday = lk8563_o2bcd(tm->tm_wday);
	tm->tm_mday = lk8563_o2bcd(tm->tm_mday);
	tm->tm_year = lk8563_o2bcd(tm->tm_year);
	if(lk_rtc_show_time)
	{
		LK8563_LOG("lk8563_convert_bcd will set tm-> = 0x%02x/0x%02x/0x%02x (0x%x) 0x%02x:0x%02x:0x%02x\n",
		  tm->tm_year, tm->tm_mon, tm->tm_mday,
		  tm->tm_wday, tm->tm_hour, tm->tm_min, tm->tm_sec);
	}
}

static int lk8563_rtc_get_tick(struct i2c_client *client, struct rtc_time *tm)
{
	//LK8563_LOG("%s\n", __func__);
	if(lk8563_read(I2C_RTC_LK8563_SEC, &tm->tm_sec, client) ||
	   lk8563_read(I2C_RTC_LK8563_MIN, &tm->tm_min, client) ||
	   lk8563_read(I2C_RTC_LK8563_HOUR, &tm->tm_hour, client) ||
	   lk8563_read(I2C_RTC_LK8563_DAY, &tm->tm_mday, client) ||
	   lk8563_read(I2C_RTC_LK8563_WEEK, &tm->tm_wday, client) ||
	   lk8563_read(I2C_RTC_LK8563_MON, &tm->tm_mon, client) ||
	   lk8563_read(I2C_RTC_LK8563_YEAR, &tm->tm_year, client))
    {
	   	LK8563_LOG("lk8563_read in lk8563_rtc_get_tick failed!!");
		return -1;
    }
	
	lk8563_convert(tm);

	return 0;
}

static int lk8563_rtc_set_tick(struct i2c_client *client, struct rtc_time *tm)
{
	LK8563_LOG("%s\n", __func__);
	
	lk8563_convert_bcd(client, tm);
	
	if(lk8563_write(I2C_RTC_LK8563_YEAR, tm->tm_year, client) ||
	   lk8563_write(I2C_RTC_LK8563_MON, tm->tm_mon, client) ||
	   lk8563_write(I2C_RTC_LK8563_WEEK, tm->tm_wday, client) ||
	   lk8563_write(I2C_RTC_LK8563_DAY, tm->tm_mday, client) ||
	   lk8563_write(I2C_RTC_LK8563_HOUR, tm->tm_hour, client) ||
	   lk8563_write(I2C_RTC_LK8563_MIN, tm->tm_min, client) ||
	   lk8563_write(I2C_RTC_LK8563_SEC, tm->tm_sec, client))
	{
		LK8563_LOG("lk8563_write in lk8563_rtc_set_tick failed!!");
		return -1;
	}

	return 0;
}

static int rtc_ops_read_time(struct i2c_client *client, struct rtc_time *tm)
{
	//LK8563_LOG("rtc_ops_read_time\n");

	if(lk8563_rtc_get_tick(lk_rtc->i2c, tm))
	{
		LK8563_LOG("%s lk8563_rtc_get_tick set failed!\n", __func__);
		return -1;
	}

	tm->tm_year -= I2C_RTC_BASE_YEAR;
	tm->tm_mon--;

	if(lk_rtc_show_time)
	{
		LK8563_LOG("read tc time = %04d/%02d/%02d (%d) %02d:%02d:%02d\n",
		  tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
		  tm->tm_wday, tm->tm_hour, tm->tm_min, tm->tm_sec);
	}

	return 0;
}

static int rtc_ops_set_time(struct i2c_client *client, struct rtc_time *tm)
{
	LK8563_LOG("rtc_ops_set_time\n");
	
	unsigned long time;
	
	rtc_tm_to_time(tm, &time);
	LK8563_LOG("time = %ld, LONG_MAX = %ld\n", time, LONG_MAX);
	if (time > (unsigned long)LONG_MAX)
		return -EINVAL;
	if(lk_rtc_show_time)
	{
		LK8563_LOG("set tc time = %04d/%02d/%02d (%d) %02d:%02d:%02d\n",
		  tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
		  tm->tm_wday, tm->tm_hour, tm->tm_min, tm->tm_sec);
	}
	if(lk8563_rtc_set_tick(lk_rtc->i2c, tm))
	{
		LK8563_LOG("%s lk8563_rtc_set_tick set failed!\n", __func__);
		return -1;
	}

	return 0;
}

static int rtc_ops_read_alarm(struct i2c_client *client, struct rtc_wkalrm *alm)
{
	LK8563_LOG("rtc_ops_read_alarm\n");
	return 0;
}

static int rtc_ops_set_alarm(struct i2c_client *client, struct rtc_wkalrm *alm)
{
	LK8563_LOG("rtc_ops_set_alarm\n");
	return 0;
}

static const struct rtc_class_ops rtc_ops = {
	.read_time = rtc_ops_read_time,
	.set_time = rtc_ops_set_time,
	.read_alarm = rtc_ops_read_alarm,
	.set_alarm = rtc_ops_set_alarm,
};

//初始化寄存器
static int lk8563_init_client(struct i2c_client *client)  
{
	LK8563_LOG("%s\n",__func__);
	
	time64_t now_time = 0;
	struct rtc_time nowtm;

	if(lk8563_rtc_get_tick(client, &nowtm))
	{
		LK8563_LOG("lk8563_rtc_get_tick in lk8563_init_client failed!!");
		return -1;
	}
	if(lk_rtc_show_time)
	{
		LK8563_LOG("lk8563_init_client read time = %04d/%02d/%02d (%d) %02d:%02d:%02d\n",
		  nowtm.tm_year + 1900, nowtm.tm_mon + 1, nowtm.tm_mday,
		  nowtm.tm_wday, nowtm.tm_hour, nowtm.tm_min, nowtm.tm_sec);
	}

	if(nowtm.tm_year || nowtm.tm_mon || nowtm.tm_mday ||
	   nowtm.tm_wday || nowtm.tm_hour || nowtm.tm_min || nowtm.tm_sec)
	{
		return 0;
	}
	else
	{
		if(lk8563_write(I2C_RTC_LK8563_YEAR, 0x23, client) ||
		   lk8563_write(I2C_RTC_LK8563_MON, 0x11, client) ||
		   lk8563_write(I2C_RTC_LK8563_WEEK, 0x02, client) ||
		   lk8563_write(I2C_RTC_LK8563_DAY, 0x14, client) ||
		   lk8563_write(I2C_RTC_LK8563_HOUR, 0x17, client) ||
		   lk8563_write(I2C_RTC_LK8563_MIN, 0x30, client) ||
		   lk8563_write(I2C_RTC_LK8563_SEC, 0x59, client))
		{
			LK8563_LOG("lk8563_write in lk8563_init_client failed!!");
			return -1;
		}

		now_time =
			mktime(nowtm.tm_year, nowtm.tm_mon, nowtm.tm_mday,
			   nowtm.tm_hour, nowtm.tm_min, nowtm.tm_sec);
		  
		LK8563_LOG("now_time = %lld",now_time);
	}
	return 0;
}

/*=====================================i2c probe================================================*/
static int  LK8563_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	struct lk8563_rtc *rtc;
	int err = 0;
	unsigned long flags;

	LK8563_LOG("begin->>>>>>>>>>%s!\n",__func__);
	
	rtc = devm_kzalloc(&client->dev, sizeof(struct lk8563_rtc), GFP_KERNEL);
	if (rtc == NULL)
		return -ENOMEM;
	
  	rtc->i2c = client;
	lk_rtc = rtc;
	dev_set_drvdata(&client->dev, rtc);
	err = lk8563_init_client(client);

	if(err < 0){
		LK8563_LOG("LK8563_init_client is err err = %d\n",err);
		return -1; 
	}

	//spin_lock_irqsave(&rtc->lock, flags);
	//lk_rtc_set_lp_irq();
	//spin_unlock_irqrestore(&rtc->lock, flags);
	
	//device_init_wakeup(&client->dev, 1);
	
		/* register rtc device (/dev/rtc0) */
	rtc->rtc_dev = rtc_device_register(RTC_NAME,
					&client->dev, &rtc_ops, THIS_MODULE);
	if (IS_ERR(rtc)) {
		pr_err("register rtc device failed (%ld)\n", PTR_ERR(rtc));
		LK8563_LOG("rtc_device_register failed!");
		return PTR_ERR(rtc);
	}

	//pmic_register_interrupt_callback(INT_RTC, lk_rtc_irq_handler);
	//pmic_enable_interrupt(INT_RTC, 1, "RTC");
	return 0;
}

static int  LK8563_remove(struct i2c_client *client) 
{ 
	//stop_kthread();
	LK8563_LOG("LK8563_remove\n");
	kfree(i2c_get_clientdata(client)); 
	return 0; 
} 

static struct i2c_device_id LK8563_id[] = { 
	{"LK8563", 0 }, 
	{} 
}; 
MODULE_DEVICE_TABLE(i2c, LK8563_id); 

// MODULE_DEVICE_TABLE的第一个参数是设备的类型,后面一个参数是设备表，这个设备表的最后一个元素是空的，用于标识结束。

static const struct of_device_id LK8563_of_match[] = {
	{ .compatible = "mediatek,LK8563", },
	{},
};

MODULE_DEVICE_TABLE(of, LK8563_of_match);


static struct i2c_driver LK8563_driver = { 
	.driver = { 
		.name = RTC_NAME, 
		.of_match_table = LK8563_of_match,
		.owner = THIS_MODULE, 
	}, 
	.probe = LK8563_probe, 
	.remove = LK8563_remove, 
	.id_table = LK8563_id, 
};

static int  LK8563_init(void) 
{ 

	if(i2c_add_driver(&LK8563_driver))
	{
		LK8563_LOG("=== Brilla LK8563_init  failed === \n");
		return -1;
	}
	LK8563_LOG("=== Brilla LK8563_init  === \n");

	return  0;
}

static void __exit LK8563_exit(void) 
{ 	
	LK8563_LOG("=== Brilla LK8563_exit  === \n");
	i2c_del_driver(&LK8563_driver); 
}

module_init(LK8563_init);
module_exit(LK8563_exit);


MODULE_AUTHOR("Brilla"); 
MODULE_DESCRIPTION("RTC Driver for LK8653"); 
MODULE_LICENSE("GPL");

