#ifndef __LIBBATTMATH_H
#define __LIBBATTMATH_H
#include "libbattmath-devices.h"


struct lbm_battery_device lbm_device_init(enum lbm_device device);

int lbm_calculate_rbatt(int batt_temp, struct lbm_battery_device battery);

int lbm_calculate_ocv(int vbat, int ibat, int batt_temp,
		      struct lbm_battery_device battery);

int lbm_calculate_capacity(int vbat, int ibat, int batt_temp,
			   struct lbm_battery_device battery);

#endif
