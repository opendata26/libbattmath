/*
 * This is free and unencumbered software released into the public domain.
 *
 * Anyone is free to copy, modify, publish, use, compile, sell, or
 * distribute this software, either in source code form or as a compiled
 * binary, for any purpose, commercial or non-commercial, and by any
 * means.
 *
 * In jurisdictions that recognize copyright laws, the author or authors
 * of this software dedicate any and all copyright interest in the
 * software to the public domain. We make this dedication for the benefit
 * of the public at large and to the detriment of our heirs and
 * successors. We intend this dedication to be an overt act of
 * relinquishment in perpetuity of all present and future rights to this
 * software under copyright law.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * For more information, please refer to <http://unlicense.org/>
 */

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

static int __calculate_fcc(int batt_temp, struct lbm_fcc_table fcc_table)
{
	int i, fcc_mv;

	for (i = 0; i < sizeof(fcc_table.temp) /
	     sizeof(*fcc_table.temp); i++)
		if (batt_temp <= fcc_table.temp[i])
			break;

	if (batt_temp == fcc_table.temp[i]) 
		fcc_mv = fcc_table.lut[i];
	else 
		fcc_mv = __interpolate(
			fcc_table.lut[i - 1],
			fcc_table.temp[i - 1],
			fcc_table.lut[i],
			fcc_table.temp[i],
			batt_temp);

	return fcc_mv * 10000;
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

/*
 * Calculates fcc based off battery temprature
 */
int lbm_calculate_fcc(int batt_temp, struct lbm_battery_device battery)
{
	return __calculate_fcc(batt_temp, battery.fcc_table);
}

/*
 * Calculates a capacity from user supplied ocv and fcc
 */
int lbm_calculate_capacity(int batt_temp, int ocv, int cc, int fcc,
			   struct lbm_battery_device battery)
{
	int orig_capacity;

	orig_capacity = __calculate_capacity((ocv + 5) / 10, batt_temp,
					      battery.ocv_table);

	return ((fcc * orig_capacity / 100) - cc) * 100 / fcc;
}
