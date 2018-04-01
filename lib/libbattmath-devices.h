#ifndef __LIBBATTMATH_DEVICES_H
#define __LIBBATTMATH_DEVICES_H

struct lbm_ocv_table {
	int temp[8];
	int capacity[31];
	int lut[31][8];
};

struct lbm_rbatt_table {
	int default_rbatt;
	int temp[8];
	int lut[8];
};

struct lbm_battery_device {
	struct lbm_ocv_table ocv_table;
	struct lbm_rbatt_table rbatt_table;
};

const char *lbm_device_strings[] = {
	"castor",
};

enum lbm_device {
	castor,
};

struct lbm_battery_device lbm_battery_castor = {
	.ocv_table = {
		.temp		= {-10, 0, 25, 50, 65},
		.capacity	= {100, 95, 90, 85, 80, 75, 70, 65, 60, 55, 50,
				   45, 40, 35, 30, 25, 20, 15, 10, 9, 8, 7, 6, 
				   5, 4, 3, 2, 1, 0},
		.lut		= {
					{4288, 4288, 4306, 4315, 4315},
					{4261, 4241, 4259, 4266, 4246},
					{4201, 4181, 4201, 4207, 4187},
					{4153, 4133, 4150, 4155, 4135},
					{4105, 4085, 4100, 4104, 4084},
					{4058, 4038, 4052, 4058, 4038},
					{4012, 3992, 4004, 4014, 3994},
					{3970, 3950, 3959, 3971, 3951},
					{3931, 3911, 3915, 3927, 3907},
					{3899, 3879, 3880, 3884, 3864},
					{3873, 3853, 3851, 3853, 3833},
					{3848, 3828, 3827, 3829, 3809},
					{3829, 3809, 3808, 3809, 3789},
					{3815, 3795, 3791, 3791, 3771},
					{3801, 3781, 3775, 3772, 3752},
					{3785, 3765, 3751, 3746, 3726},
					{3767, 3747, 3727, 3719, 3699},
					{3750, 3730, 3702, 3692, 3672},
					{3728, 3708, 3680, 3672, 3652},
					{3720, 3700, 3676, 3665, 3645},
					{3712, 3692, 3670, 3660, 3645},
					{3695, 3675, 3658, 3648, 3633},
					{3662, 3647, 3629, 3620, 3610},
					{3620, 3605, 3589, 3580, 3570},
					{3562, 3552, 3538, 3529, 3519},
					{3490, 3480, 3474, 3470, 3465},
					{3403, 3398, 3388, 3380, 3375},
					{3320, 3300, 3255, 3221, 3206},
					{3000, 3000, 3000, 3000, 3000},
				},
	},
	.rbatt_table = {
		.default_rbatt	= 100,
		.temp		= {-10, 0, 25, 40, 65},
		.lut		= {303, 190, 87, 78, 78},
	},
};

#endif