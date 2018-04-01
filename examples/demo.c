#include <assert.h>
#include <errno.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "../lib/libbattmath.h"

const char *device_strings[] = {
	"castor",
};

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
				for (int i = 0; i < sizeof(device_strings) / 
				     sizeof(*device_strings); i++){
					if (!strcmp(strdup(optarg), device_strings[i])){
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
