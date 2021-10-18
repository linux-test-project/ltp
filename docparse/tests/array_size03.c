static struct foo variants[] = {
#ifdef FOO
	{.bar = 11},
#endif
	{.bar = 10},
};

static struct tst_test test = {
	.test_variants = ARRAY_SIZE(variants),
};
