#ifndef __LIBRETRO_CORE_OPTIONS_H__
#define __LIBRETRO_CORE_OPTIONS_H__

#include <stdlib.h>
#include <string.h>

#include <libretro.h>
#include <retro_inline.h>

static const char enabled_value[] = "enabled";
static const char disabled_value[] = "disabled";

static struct retro_core_option_definition *option_defs_us = NULL;

static const struct retro_core_option_definition var_empty = {NULL, NULL, NULL, {{0}}, NULL};

static const struct retro_core_option_definition var_fba_allow_depth_32 = {
	"fba-allow-depth-32",
	"32位色深 (如果可用)",
	"Change pixel format, some games require this to render properly, it could impact performances on some platforms",
	{
		{"disabled", "否"},
		{"enabled", "是"},
		{NULL, NULL},
	},
	"enabled",
};

static const struct retro_core_option_definition var_fba_sound_out = {
	"fba-sound_out",
	"输出声音",
	NULL,
	{
		{"disabled", "否"},
		{"enabled", "是"},
		{NULL, NULL},
	},
	"enabled",
};

static const struct retro_core_option_definition var_fba_frameskip = {
	"fba-frameskip",
	"跳帧",
	NULL,
	{
		{"0", NULL},
		{"1", NULL},
		{"2", NULL},
		{"3", NULL},
		{"4", NULL},
		{"5", NULL},
		{NULL, NULL},
	},
	"0",
};

static const struct retro_core_option_definition var_fba_cpu_speed_adjust = {
	"fba-cpu-speed-adjust",
	"模拟CPU频率",
	"Change emulated cpu frequency for various systems, by increasing you can fix native slowdowns in some games, by decreasing you can help performance on low-end devices",
	{
		{"100", "100%"},
		{"110", "110%"},
		{"120", "120%"},
		{"130", "130%"},
		{"140", "140%"},
		{"150", "150%"},
		{"160", "160%"},
		{"170", "170%"},
		{"180", "180%"},
		{"190", "190%"},
		{"200", "200%"},
		{NULL, NULL},
	},
	"100",
};

static const struct retro_core_option_definition var_fba_hiscores = {
	"fba-hiscores",
	"排名",
	"Enable high scores support, you also need the file hiscore.dat in your system/fbneo/ folder",
	{
		{"disabled", "否"},
		{"enabled", "是"},
		{NULL, NULL},
	},
	"enabled",
};

static const struct retro_core_option_definition var_fba_samplerate = {
	"fba-samplerate",
	"音频采样率",
	"Configure samplerate, it could impact performances, closing & starting game again is required",
	{
		{"11025", NULL},
		{"22050", NULL},
		{"44100", NULL},
		{"48000", NULL},
		{NULL, NULL},
	},
	"48000",
};

static const struct retro_core_option_definition var_fba_sample_interpolation = {
	"fba-sample-interpolation",
	"样本插值 (会影响性能)",
	"Configure sample interpolation, it could impact performances",
	{
		{"disabled", "否"},
		{"2-point 1st order", "两点一次"},
		{"4-point 3rd order", "四点三次"},
		{NULL, NULL},
	},
	"disabled",
};

static const struct retro_core_option_definition var_fba_fm_interpolation = {
	"fba-fm-interpolation",
	"FM插值 (会影响性能)",
	"Configure FM interpolation, it could impact performances",
	{
		{"disabled", "否"},
		{"4-point 3rd order", "四点三次"},
		{NULL, NULL},
	},
	"disabled",
};

static const struct retro_core_option_definition var_fba_analog_speed = {
	"fba-analog-speed",
	"模似控制器移动速度",
	"Mitigate analog controls speed, some games might require low values to be playable",
	{
		{"0", NULL},
		{"1", NULL},
		{"2", NULL},
		{"3", NULL},
		{"4", NULL},
		{"5", NULL},
		{"6", NULL},
		{"7", NULL},
		{"8", NULL},
		{"9", NULL},
		{"10", NULL},
		{NULL, NULL},
	},
	"10",
};

#ifdef USE_CYCLONE
static struct retro_core_option_definition var_fba_cyclone = {
	"fba-cyclone",
	"使用cyclone (可提升68k速度)",
	"使用时风险自负，它可以提高低端设备的某些模拟系统的性能，但也存在已知的副作用：保存状态与普通解释器不兼容，某些系统无法工作",
	{
		{disabled_value, "否"},
		{enabled_value, "是"},
		{NULL, NULL},
	},
	disabled_value,
};
#endif

static const struct retro_core_option_definition var_fba_diagnostic_input = {
	"fba-diagnostic-input",
	"诊断模式开启键",
	"Configure button combination to enter cabinet service menu",
	{
		{"None", "无"},
		{"Hold Start", "按住Start"},
		{"Start + A + B", "Start+A+B"},
		{"Hold Start + A + B", "按住Start+A+B"},
		{"Start + L + R", "Start+L+R"},
		{"Hold Start + L + R", "按住Start+L+R"},
		{"Hold Select", "按住Select"},
		{"Select + A + B", "Select+A+B"},
		{"Hold Select + A + B", "按住Select+A+B"},
		{"Select + L + R", "Select+L+R"},
		{"Hold Select + L + R", "按住Select+L+R"},
		{NULL, NULL},
	},
	"Hold Select",
};

// Neo Geo core options
static const struct retro_core_option_definition var_fba_neogeo_mode = {
	"fba-neogeo-mode",
	"Neo-Geo模式",
	"Load appropriate bios depending on your choice, under the condition such a bios is compatible with the running game",
	{
		{"DIPSWITCH", "由dipswitch设置"},
		{"MVS_EUR_ASI", "街机 (欧/亚洲)"},
		{"MVS_USA", "街机 (美国)"},
		{"MVS_ASI", "街机 (亚洲)"},
		{"MVS_JAP", "街机 (日本)"},
		{"AES_ASI", "家用机 (亚洲)"},
		{"AES_JAP", "家用机 (日本)"},
		{"UNIBIOS", "UNIBIOS"},
		{NULL, NULL},
	},
	"DIPSWITCH",
};

#endif

static INLINE void libretro_set_core_options(retro_environment_t environ_cb)
{
	unsigned version = 0;

	if (!environ_cb || !option_defs_us)
		return;

	if (!environ_cb(RETRO_ENVIRONMENT_GET_CORE_OPTIONS_VERSION, &version))
		version = 0;

	if (version >= 1)
	{
		environ_cb(RETRO_ENVIRONMENT_SET_CORE_OPTIONS, (void *)option_defs_us);
	}
	else
	{
		size_t i, j;
		size_t num_options = 0;
		struct retro_variable *variables = NULL;
		char **values_buf = NULL;

		while (true)
		{
			if (option_defs_us[num_options].key)
				num_options++;
			else
				break;
		}

		/* Allocate arrays */
		variables = (struct retro_variable *)calloc(num_options + 1, sizeof(struct retro_variable));
		values_buf = (char **)calloc(num_options, sizeof(char *));

		if (!variables || !values_buf)
			goto error;

		/* Copy parameters from option_defs_us array */
		for (i = 0; i < num_options; i++)
		{
			const char *key = option_defs_us[i].key;
			const char *desc = option_defs_us[i].desc;
			const char *default_value = option_defs_us[i].default_value;
			struct retro_core_option_value *values = option_defs_us[i].values;
			size_t buf_len = 3;
			size_t default_index = 0;

			values_buf[i] = NULL;

			if (desc)
			{
				size_t num_values = 0;

				/* Determine number of values */
				while (true)
				{
					if (values[num_values].value)
					{
						/* Check if this is the default value */
						if (default_value)
							if (strcmp(values[num_values].value, default_value) == 0)
								default_index = num_values;

						buf_len += strlen(values[num_values].value);
						num_values++;
					}
					else
						break;
				}

				if (num_values > 0)
				{

					buf_len += num_values - 1;
					buf_len += strlen(desc);

					values_buf[i] = (char *)calloc(buf_len, sizeof(char));
					if (!values_buf[i])
						goto error;

					strcpy(values_buf[i], desc);
					strcat(values_buf[i], "; ");

					/* Default value goes first */
					strcat(values_buf[i], values[default_index].value);

					/* Add remaining values */
					for (j = 0; j < num_values; j++)
					{
						if (j != default_index)
						{
							strcat(values_buf[i], "|");
							strcat(values_buf[i], values[j].value);
						}
					}
				}
			}

			variables[i].key = key;
			variables[i].value = values_buf[i];
		}
	error:
		if (values_buf)
		{
			for (i = 0; i < num_options; i++)
			{
				if (values_buf[i])
				{
					free(values_buf[i]);
					values_buf[i] = NULL;
				}
			}

			free(values_buf);
			values_buf = NULL;
		}

		if (variables)
		{
			free(variables);
			variables = NULL;
		}
	}
}