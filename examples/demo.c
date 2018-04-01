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

#include <assert.h>
#include <errno.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "../lib/libbattmath.h"

int main(int argc, char *argv[])
{
	enum lbm_device device;
	struct lbm_battery_device battery;
	int temp, ibat, vbat_sns, option, rbatt, ocv, capacity;
	
	while ((option = getopt(argc, argv, "s:i:t:d:")) != -1) {
		switch (option) {
			case 's':
				vbat_sns = atoi(optarg);
				break;
			case 'i':
				ibat = atoi(optarg);
				break;
			case 't':
				temp = round(atoi(optarg) / 1000);
				break;
			case 'd':
				for (int i = 0; i < sizeof(lbm_device_strings) / 
				     sizeof(*lbm_device_strings); i++){
					if (!strcmp(strdup(optarg), lbm_device_strings[i])){
						device = i;
						goto break2x;
					}
				}

				fprintf(stderr, "Invalid device: %s \n", optarg);
			default:
				return EINVAL;
				break;
		}
		break2x: ;
	}

	battery = lbm_device_init(device);
	if(errno)
		return EINVAL;

	rbatt = lbm_calculate_rbatt(temp, battery);
	ocv = lbm_calculate_ocv(vbat_sns, ibat, temp, battery);
	capacity = lbm_calculate_capacity(vbat_sns, ibat, temp, battery);

	printf("Calculated capacity of: %d, ocv of: %d and rbatt of: %d \n",
	       capacity, ocv, rbatt);
}
