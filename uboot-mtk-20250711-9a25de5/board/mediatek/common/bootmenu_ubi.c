// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2025 MediaTek Inc. All Rights Reserved.
 *
 * Author: Weijie Gao <weijie.gao@mediatek.com>
 */

#include "bootmenu_common.h"
#include "autoboot_helper.h"
#include "mtd_helper.h"
#include "bsp_conf.h"

static const struct data_part_entry mtd_parts[] = {
	{
		.name = "ATF BL2",
		.abbr = "bl2",
		.env_name = "bootfile.bl2",
		.validate = generic_validate_bl2,
		.write = generic_mtd_write_bl2_redund,
	},
	{
		.name = "ATF FIP",
		.abbr = "fip",
		.env_name = "bootfile.fip",
		.validate = generic_validate_fip,
#ifdef CONFIG_MTK_DUAL_FIP
		.write = generic_ubi_write_dual_fip,
#else
		.write = generic_ubi_write_fip,
#endif
		.post_action = UPGRADE_ACTION_CUSTOM,
		.do_post_action = generic_invalidate_env,
	},
#if defined(CONFIG_MTK_FIP_SUPPORT) && !defined(CONFIG_MTK_DUAL_FIP)
	{
		.name = "BL31 of ATF FIP",
		.abbr = "bl31",
		.env_name = "bootfile.bl31",
		.validate = generic_validate_bl31,
		.write = generic_ubi_update_bl31,
		.post_action = UPGRADE_ACTION_CUSTOM,
	},
	{
		.name = "BL33 of ATF FIP",
		.abbr = "bl33",
		.env_name = "bootfile.bl33",
		.validate = generic_validate_bl33,
		.write = generic_ubi_update_bl33,
		.post_action = UPGRADE_ACTION_CUSTOM,
		.do_post_action = generic_invalidate_env,
	},
#endif
	{
		.name = "Firmware",
		.abbr = "fw",
		.env_name = "bootfile",
		.post_action = UPGRADE_ACTION_BOOT,
		.validate = generic_mtd_validate_fw,
		.write = generic_mtd_write_fw,
	},
	{
		.name = "Single image",
		.abbr = "simg",
		.env_name = "bootfile.simg",
		.validate = generic_validate_simg,
		.write = generic_mtd_write_simg,
	},
};

void board_upgrade_data_parts(const struct data_part_entry **dpes, u32 *count)
{
	*dpes = mtd_parts;
	*count = ARRAY_SIZE(mtd_parts);
}

int board_boot_default(bool do_boot)
{
	return generic_mtd_boot_image(do_boot);
}

static const struct bootmenu_entry mtd_bootmenu_entries[] = {
	{
		.desc = "Startup system (Default)",
		.cmd = "mtkboardboot"
	},
	{
		.desc = "Upgrade firmware",
		.cmd = "mtkupgrade fw"
	},
	{
		.desc = "Upgrade ATF BL2",
		.cmd = "mtkupgrade bl2"
	},
	{
		.desc = "Upgrade ATF FIP",
		.cmd = "mtkupgrade fip"
	},
#if defined(CONFIG_MTK_FIP_SUPPORT) && !defined(CONFIG_MTK_DUAL_FIP)
	{
		.desc = "  Upgrade ATF BL31 only",
		.cmd = "mtkupgrade bl31"
	},
	{
		.desc = "  Upgrade bootloader only",
		.cmd = "mtkupgrade bl33"
	},
#endif
	{
		.desc = "Upgrade single image",
		.cmd = "mtkupgrade simg"
	},
	{
		.desc = "Load image",
		.cmd = "mtkload"
	},
#ifdef CONFIG_MTK_WEB_FAILSAFE
	{
		.desc = "Start Web failsafe",
		.cmd = "httpd"
	},
#endif
	{
		.desc = "Change boot configuration",
		.cmd = "mtkbootconf"
	},
};

void board_bootmenu_entries(const struct bootmenu_entry **menu, u32 *count)
{
	*menu = mtd_bootmenu_entries;
	*count = ARRAY_SIZE(mtd_bootmenu_entries);
}

void default_boot_set_defaults(void *fdt)
{
	mtd_boot_set_defaults(fdt);
}

#ifdef CONFIG_MTK_BSPCONF_SUPPORT
int board_save_bsp_conf(const struct mtk_bsp_conf_data *bspconf, uint32_t index)
{
	return ubi_update_bsp_conf(bspconf, index);
}
#endif

int board_late_init(void)
{
	if (IS_ENABLED(CONFIG_MTK_BSPCONF_SUPPORT)) {
		ubi_import_bsp_conf();
		import_bsp_conf_bl2(CONFIG_TEXT_BASE);
	}

	return 0;
}
