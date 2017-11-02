/*
 * EFI Boot Guard
 *
 * Copyright (c) Siemens AG, 2017
 *
 * Authors:
 *  Andreas Reichel <andreas.reichel.ext@siemens.com>
 *
 * This work is licensed under the terms of the GNU GPL, version 2.  See
 * the COPYING file in the top-level directory.
 */

#include <stdlib.h>
#include <check.h>
#include <fff.h>
#include <env_api.h>
#include <env_config_file.h>
#include <env_config_partitions.h>

DEFINE_FFF_GLOBALS;

static char *devpath = "/dev/nobrain";

bool read_env(CONFIG_PART *part, BG_ENVDATA *env);

Suite *env_api_fat_suite(void);
bool probe_config_partitions_custom_fake(CONFIG_PART *cfgpart);
bool read_env_custom_fake(CONFIG_PART *cp, BG_ENVDATA *env);

Suite *ebg_test_suite(void);

bool probe_config_partitions_custom_fake(CONFIG_PART *cfgpart)
{
	for (int i = 0; i < ENV_NUM_CONFIG_PARTS; i++) {
		cfgpart[i].devpath = devpath;
	}
	return true;
}

bool read_env_custom_fake(CONFIG_PART *cp, BG_ENVDATA *env)
{
	if (!env) {
		return false;
	}
	memset(env, 0, sizeof(BG_ENVDATA));
	return true;
}

FAKE_VALUE_FUNC(bool, probe_config_partitions, CONFIG_PART *);
FAKE_VALUE_FUNC(bool, read_env, CONFIG_PART *, BG_ENVDATA *);

START_TEST(env_api_fat_test_bgenv_init_retval)
{
	bool result;
	/* In this unit test, contents of environment data are
	 * faked to be all zero
	 */

	/* Test if bgenv_init fails if no config partitions are found
	 */
	RESET_FAKE(probe_config_partitions);

	probe_config_partitions_fake.return_val = false;

	result = bgenv_init();

	ck_assert(probe_config_partitions_fake.call_count == 1);
	ck_assert(result == false);

	/* Test if bgenv_init succeeds if config partitions are found
	 */
	RESET_FAKE(probe_config_partitions);

	probe_config_partitions_fake.custom_fake = probe_config_partitions_custom_fake;
	read_env_fake.custom_fake = read_env_custom_fake;
	result = bgenv_init();

	ck_assert(probe_config_partitions_fake.call_count == 1);
	ck_assert(read_env_fake.call_count == ENV_NUM_CONFIG_PARTS);
	ck_assert(result == true);
}
END_TEST

Suite *ebg_test_suite(void)
{
	Suite *s;
	TCase *tc_core;

	s = suite_create("env_api_fat");

	tc_core = tcase_create("Core");
	tcase_add_test(tc_core, env_api_fat_test_bgenv_init_retval);
	suite_add_tcase(s, tc_core);

	return s;
}
