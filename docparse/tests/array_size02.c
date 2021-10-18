struct foo {
	int val;
};

static struct foo variants[] = {{1}, {2}, {3}};

static struct tst_test test = {
	.test_variants = ARRAY_SIZE(variants),
};
