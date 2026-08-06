#include <linux/module.h>
#include <linux/export-internal.h>
#include <linux/compiler.h>

MODULE_INFO(name, KBUILD_MODNAME);

__visible struct module __this_module
__section(".gnu.linkonce.this_module") = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};



static const struct modversion_info ____versions[]
__used __section("__versions") = {
	{ 0x2ed3eb28, "platform_get_irq" },
	{ 0xc2ccdd1e, "__init_swait_queue_head" },
	{ 0xb730487b, "finish_wait" },
	{ 0xc281f1fb, "prepare_to_wait_event" },
	{ 0x5e505530, "kthread_should_stop" },
	{ 0x68a1b6c6, "__wake_up" },
	{ 0x11f4259a, "_raw_spin_lock_irqsave" },
	{ 0xd272d446, "__fentry__" },
	{ 0xe64f49d7, "wake_up_process" },
	{ 0xe8213e80, "_printk" },
	{ 0xbd03ed67, "__ref_stack_chk_guard" },
	{ 0x6ac784f4, "schedule_timeout" },
	{ 0xd272d446, "__stack_chk_fail" },
	{ 0xb485f613, "_dev_info" },
	{ 0x90a48d82, "__ubsan_handle_out_of_bounds" },
	{ 0xdb375fb3, "cdev_add" },
	{ 0x7a5ffe84, "init_wait_entry" },
	{ 0xb485f613, "_dev_err" },
	{ 0x160b81b4, "device_create" },
	{ 0x7ed256c3, "noop_llseek" },
	{ 0x9aa6980d, "mutex_lock" },
	{ 0xc609ff70, "strncpy" },
	{ 0x4f1e5fd0, "__list_del_entry_valid_or_report" },
	{ 0x07a5cde6, "class_unregister" },
	{ 0x7cec824a, "kthread_stop" },
	{ 0x444885a7, "_raw_spin_unlock_irqrestore" },
	{ 0xd272d446, "__x86_return_thunk" },
	{ 0x092a35a2, "_copy_to_user" },
	{ 0xe804603d, "__init_waitqueue_head" },
	{ 0x62cbec20, "complete_all" },
	{ 0x058c185a, "jiffies" },
	{ 0xfb598b3a, "__platform_driver_register" },
	{ 0x772c91c5, "kthread_create_on_node" },
	{ 0x1ab27905, "__devm_reset_control_get" },
	{ 0x0bc5fb0d, "unregister_chrdev_region" },
	{ 0x9aa6980d, "mutex_unlock" },
	{ 0xd17123e4, "device_destroy" },
	{ 0xdd093a51, "devm_clk_bulk_get_all" },
	{ 0x1a29d1ea, "dev_err_probe" },
	{ 0x189ec92d, "class_register" },
	{ 0x67628f51, "msleep" },
	{ 0xd2554727, "cdev_init" },
	{ 0x7851be11, "__SCT__might_resched" },
	{ 0x2e921116, "cdev_del" },
	{ 0x9aa6980d, "mutex_init_generic" },
	{ 0x9f222e1e, "alloc_chrdev_region" },
	{ 0x52ec67ed, "devm_platform_ioremap_resource" },
	{ 0xdc352a3b, "__list_add_valid_or_report" },
	{ 0x092a35a2, "_copy_from_user" },
	{ 0x5ce224b8, "devm_kmalloc" },
	{ 0x534ed5f3, "__msecs_to_jiffies" },
	{ 0x0064884b, "platform_driver_unregister" },
	{ 0xd954c786, "module_layout" },
};

static const u32 ____version_ext_crcs[]
__used __section("__version_ext_crcs") = {
	0x2ed3eb28,
	0xc2ccdd1e,
	0xb730487b,
	0xc281f1fb,
	0x5e505530,
	0x68a1b6c6,
	0x11f4259a,
	0xd272d446,
	0xe64f49d7,
	0xe8213e80,
	0xbd03ed67,
	0x6ac784f4,
	0xd272d446,
	0xb485f613,
	0x90a48d82,
	0xdb375fb3,
	0x7a5ffe84,
	0xb485f613,
	0x160b81b4,
	0x7ed256c3,
	0x9aa6980d,
	0xc609ff70,
	0x4f1e5fd0,
	0x07a5cde6,
	0x7cec824a,
	0x444885a7,
	0xd272d446,
	0x092a35a2,
	0xe804603d,
	0x62cbec20,
	0x058c185a,
	0xfb598b3a,
	0x772c91c5,
	0x1ab27905,
	0x0bc5fb0d,
	0x9aa6980d,
	0xd17123e4,
	0xdd093a51,
	0x1a29d1ea,
	0x189ec92d,
	0x67628f51,
	0xd2554727,
	0x7851be11,
	0x2e921116,
	0x9aa6980d,
	0x9f222e1e,
	0x52ec67ed,
	0xdc352a3b,
	0x092a35a2,
	0x5ce224b8,
	0x534ed5f3,
	0x0064884b,
	0xd954c786,
};
static const char ____version_ext_names[]
__used __section("__version_ext_names") =
	"platform_get_irq\0"
	"__init_swait_queue_head\0"
	"finish_wait\0"
	"prepare_to_wait_event\0"
	"kthread_should_stop\0"
	"__wake_up\0"
	"_raw_spin_lock_irqsave\0"
	"__fentry__\0"
	"wake_up_process\0"
	"_printk\0"
	"__ref_stack_chk_guard\0"
	"schedule_timeout\0"
	"__stack_chk_fail\0"
	"_dev_info\0"
	"__ubsan_handle_out_of_bounds\0"
	"cdev_add\0"
	"init_wait_entry\0"
	"_dev_err\0"
	"device_create\0"
	"noop_llseek\0"
	"mutex_lock\0"
	"strncpy\0"
	"__list_del_entry_valid_or_report\0"
	"class_unregister\0"
	"kthread_stop\0"
	"_raw_spin_unlock_irqrestore\0"
	"__x86_return_thunk\0"
	"_copy_to_user\0"
	"__init_waitqueue_head\0"
	"complete_all\0"
	"jiffies\0"
	"__platform_driver_register\0"
	"kthread_create_on_node\0"
	"__devm_reset_control_get\0"
	"unregister_chrdev_region\0"
	"mutex_unlock\0"
	"device_destroy\0"
	"devm_clk_bulk_get_all\0"
	"dev_err_probe\0"
	"class_register\0"
	"msleep\0"
	"cdev_init\0"
	"__SCT__might_resched\0"
	"cdev_del\0"
	"mutex_init_generic\0"
	"alloc_chrdev_region\0"
	"devm_platform_ioremap_resource\0"
	"__list_add_valid_or_report\0"
	"_copy_from_user\0"
	"devm_kmalloc\0"
	"__msecs_to_jiffies\0"
	"platform_driver_unregister\0"
	"module_layout\0"
;

MODULE_INFO(depends, "");

MODULE_ALIAS("of:N*T*Cnvidia,tegra234-cortex-forge");
MODULE_ALIAS("of:N*T*Cnvidia,tegra234-cortex-forgeC*");

MODULE_INFO(srcversion, "D0779626264D9D04221566D");
