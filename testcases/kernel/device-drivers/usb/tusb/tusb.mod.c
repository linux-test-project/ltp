#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

const char vermagic[]
__attribute__((section("__vermagic"))) =
VERMAGIC_STRING;

static const char __module_depends[]
__attribute_used__
__attribute__((section(".modinfo"))) =
"depends=";

MODULE_ALIAS("usb:v*p*dl*dh*dc*dsc*dp*ic03isc01ip01*");
MODULE_ALIAS("usb:v*p*dl*dh*dc*dsc*dp*ic03isc01ip02*");
