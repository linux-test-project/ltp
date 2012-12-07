#include <math.h>
#include <stdlib.h>

#define M_2PI (M_PI*2)

int box_muler(int min, int max)
{
	double u1, u2, z;
	int i;
	int ave;
	int range;
	int ZZ;
	if (min >= max) {
		return (-1);
	}
	range = max - min;
	ave = range / 2;
	for (i = 0; i < 10; i++) {
		u1 = ((double)(random() % 1000000)) / 1000000;
		u2 = ((double)(random() % 1000000)) / 1000000;
		z = sqrt(-2.0 * log(u1)) * cos(M_2PI * u2);
		ZZ = min + (ave + (z * (ave / 4)));
		if (ZZ >= min && ZZ < max) {
			return (ZZ);
		}
	}
	return (-1);
}
