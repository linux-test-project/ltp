#include <stdio.h>
#include <test.h>

int kernel_support_available(void) {
    if (tst_kvercmp(2,6,22) < 0)
        return 1;
    return 0;
}

int main() {
	exit(kernel_support_available());
}
