#include <math.h>
#include <errno.h>
#include <stdio.h>

#include "libbattmath.h"

static int __interpolate(int y0, int x0, int y1, int x1, int x)
{
	if (y0 == y1 || x == x0)
		return y0;
	if (x1 == x0 || x == x1)
		return y1;

	return y0 + ((y1 - y0) * (x - x0) / (x1 - x0));
}

static int __between(int left, int right, int val)
{
	if (left >= right && left >= val && val >= right)
		return 1;
	if (left <= right && left <= val && val <= right)
		return 1;

	return 0;
}

static int __calculate_rbatt(int batt_temp, struct lbm_rbatt_table rbatt_table)
{
	int scale, i;

	for (i = 0; i < sizeof(rbatt_table.temp) /
	     sizeof(*rbatt_table.temp); i++)
		if (batt_temp <= rbatt_table.temp[i])
			break;

	if (batt_temp == rbatt_table.temp[i]) 
		scale = rbatt_table.lut[i];
	else 
		scale = __interpolate(
			rbatt_table.lut[i - 1],
			rbatt_table.temp[i - 1],
			rbatt_table.lut[i],
			rbatt_table.temp[i],
			batt_temp);
	
	return ((rbatt_table.default_rbatt * scale) / 100)  - /*3*/0;
}

static int __calculate_ocv(int vbat, int ibat, int rbatt)
{
	return vbat + (ibat * rbatt);
}

static int __calculate_capacity(int ocv, int batt_temp,
			   struct lbm_ocv_table ocv_table)
{
	int i, j;
	int pcj_minus_one = 0;
	int pcj = 0;
	int rows = sizeof(ocv_table.capacity) / sizeof(*ocv_table.capacity);
	
	for (j = 0; j < sizeof(ocv_table.temp) /
	     sizeof(*ocv_table.temp); j++)
		if (batt_temp <= ocv_table.temp[j])
			break;
	
	if (batt_temp == ocv_table.temp[j]) {
		//Exact temprature match
		if (ocv >= ocv_table.lut[0][j])
			return ocv_table.capacity[0];

		if (ocv <= ocv_table.lut[rows - 1][j])
			return ocv_table.capacity[rows - 1];

		for (i = 0; i < rows; i++) {
			if (ocv >= ocv_table.lut[i][j]) {
				if (ocv == ocv_table.lut[i][j])
					return ocv_table.capacity[i];

				return __interpolate(
					ocv_table.capacity[i],
					ocv_table.lut[i][j],
					ocv_table.capacity[i - 1],
					ocv_table.lut[i - 1][j],
					ocv);
			}
		}
	}

	if (ocv >= ocv_table.lut[0][j]) //100%
		return ocv_table.capacity[0];
	
	if (ocv <= ocv_table.lut[rows - 1][j - 1])
		return ocv_table.capacity[rows - 1];

	for (i = 0; i < rows-1; i++) {
		if (pcj == 0
			&& __between(ocv_table.lut[i][j],
				ocv_table.lut[i+1][j], ocv)) {
			pcj = __interpolate(
				ocv_table.capacity[i],
				ocv_table.lut[i][j],
				ocv_table.capacity[i + 1],
				ocv_table.lut[i+1][j],
				ocv);
		}

		if (pcj_minus_one == 0
			&& __between(ocv_table.lut[i][j-1],
				ocv_table.lut[i+1][j-1], ocv)) {
			pcj_minus_one = __interpolate(
				ocv_table.capacity[i],
				ocv_table.lut[i][j-1],
				ocv_table.capacity[i + 1],
				ocv_table.lut[i+1][j-1],
				ocv);
		}

		if (pcj && pcj_minus_one) {
			return __interpolate(
				pcj_minus_one,
				ocv_table.temp[j-1],
				pcj,
				ocv_table.temp[j],
				batt_temp);
		}
	}

	if (pcj)
		return pcj;

	if (pcj_minus_one)
		return pcj_minus_one;

	return 100;
}

struct lbm_battery_device lbm_device_init(enum lbm_device device)
{
	struct lbm_battery_device battery;
	
	//TODO: find a better way to choose correct lbm_battery_device
	switch(device) {
		case castor:
			battery = lbm_battery_castor;
			break;
		default:
			errno = -EINVAL;
	}

	return battery;
}

int lbm_calculate_rbatt(int batt_temp, struct lbm_battery_device battery)
{
	return __calculate_rbatt(batt_temp, battery.rbatt_table);
}

int lbm_calculate_ocv(int vbat, int ibat, int batt_temp, 
		      struct lbm_battery_device battery)
{
	/*
	 * 1. Calculate rbatt
	 * 2. Calculate ocv based off that
	 */

	return __calculate_ocv(vbat, ibat, lbm_calculate_rbatt(batt_temp,
			       battery));
}

int lbm_calculate_capacity(int vbat, int ibat, int batt_temp,
			   struct lbm_battery_device battery)
{
	/*
	 * 1. Calculate rbatt
	 * 2. Calculate ocv based off rbatt and divide by 10 to work with
	 * downstream ocv luts
	 * 3. Finally calculate a capacity from that ocv
	 */

	return __calculate_capacity(round(lbm_calculate_ocv(vbat, ibat,
				    batt_temp, battery) / 10), batt_temp,
				    battery.ocv_table);
}
