/*
 * Copyright (C) 2016 Unwired Devices [info@unwds.com]
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup    
 * @ingroup     
 * @brief       
 * @{
 * @file		umdk-lmt01.c
 * @brief       umdk-lmt01 module implementation
 * @author      Eugene Ponomarev
 */

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "lpm.h"
#include "periph/gpio.h"
#include "periph/adc.h"
#include "board.h"


#include "unwds-common.h"
#include "unwds-gpio.h"

#include "umdk-6adc.h"

#include "thread.h"
#include "xtimer.h"

static uwnds_cb_t *callback;

static kernel_pid_t timer_pid;
static char timer_stack[THREAD_STACKSIZE_MAIN];

static int publish_period_min;
static bool adc_lines_en[ADC_NUMOF] = { };
static gpio_t out_pin = UMDK_6ADC_OUT_PIN;

static msg_t timer_msg = {};
static xtimer_t timer;

static void init_gpio(void) {
	gpio_init(out_pin, GPIO_OUT);
	gpio_clear(out_pin);
}

static void init_adc(bool enable_all) {
	int i = 0;

	for (i = 0; i < ADC_NUMOF; i++) {
		if (adc_init(ADC_LINE(i)) < 0) {
			printf("[umdk-6adc] Failed to initialize adc line #%d\n", i);
			continue;
		}

		if (enable_all)
			adc_lines_en[i] = true;
	}
}

static void prepare_result(module_data_t *buf) {
	int i;

	uint16_t res[ADC_NUMOF] = { 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF };

	for (i = 0; i < ADC_NUMOF; i++) {
		if (!adc_lines_en[i]) {
			continue;
		}

		res[i] = adc_sample(ADC_LINE(i), UMDK_6ADC_ADC_RESOLUTION);

		printf("[umdk-6adc] Reading line #%d: %d\n", i, res[i]);

		/* Delay between sensor switching */
		xtimer_usleep(1e3 * 100);
	}

	buf->data[0] = UNWDS_6ADC_MODULE_ID;
	memcpy(buf->data + 1, (uint8_t *) res, sizeof(res));
	buf->length = sizeof(res) + 1;
}

static void *timer_thread(void *arg) {
    msg_t msg;
    msg_t msg_queue[8];
    msg_init_queue(msg_queue, 8);

    puts("[umdk-adc] Periodic publisher thread started");

    while (1) {
        msg_receive(&msg);

        lpm_prevent_sleep = 1;

        xtimer_remove(&timer);

        gpio_set(out_pin);

        module_data_t data = {};
        prepare_result(&data);

        gpio_clear(out_pin);

        /* Notify the application */
        callback(&data);

        lpm_prevent_sleep = 0;

        /* Restart after delay */
        xtimer_set_msg(&timer, 1e6 * 60 * publish_period_min, &timer_msg, timer_pid);
    }

    return NULL;
}

void umdk_6adc_init(uint32_t *non_gpio_pin_map, uwnds_cb_t *event_callback) {
	(void) non_gpio_pin_map;

	callback = event_callback;
	publish_period_min = UMDK_6ADC_PUBLISH_PERIOD_MIN; /* Set to default */

	init_gpio();
	init_adc(true);

	/* Create handler thread */
	timer_pid = thread_create(timer_stack, sizeof(timer_stack), THREAD_PRIORITY_MAIN - 1, 0, timer_thread, NULL, "6ADC publisher thread");

    /* Start publishing timer */
	xtimer_set_msg(&timer, 1e6 * 60 * publish_period_min, &timer_msg, timer_pid);
}

bool umdk_6adc_cmd(module_data_t *cmd, module_data_t *reply) {
	if (cmd->length < 1)
		return false;

	umdk_6adc_cmd_t c = cmd->data[0];
	switch (c) {
	case UMDK_6ADC_CMD_SET_PERIOD: {
		if (cmd->length != 2)
			return false;

		uint8_t period = cmd->data[1];
		xtimer_remove(&timer);

		publish_period_min = period;

		/* Don't restart timer if new period is zero */
		if (publish_period_min) {
			xtimer_set_msg(&timer, 1e6 * 60 * publish_period_min, &timer_msg, timer_pid);
			printf("[6adc] Period set to %d minutes\n", publish_period_min);
		} else
			puts("[6adc] Timer stopped");

		reply->length = 4;
		reply->data[0] = UNWDS_6ADC_MODULE_ID;
		reply->data[1] = 'o';
		reply->data[2] = 'k';
		reply->data[3] = '\0';

		break;
	}

	case UMDK_6ADC_CMD_POLL:
		/* Send signal to publisher thread */
		msg_send(&timer_msg, timer_pid);

		reply->length = 4;
		reply->data[0] = UNWDS_6ADC_MODULE_ID;
		reply->data[1] = 'o';
		reply->data[2] = 'k';
		reply->data[3] = '\0';

		return false; /* Don't reply */

		break;

	case UMDK_6ADC_CMD_SET_GPIO: {
		uint8_t gpio = cmd->data[1];

		out_pin = unwds_gpio_pin(gpio);

		/* Re-initialize GPIO */
		init_gpio();

		break;
	}

	case UMDK_6ADC_SET_LINES: {
			uint8_t lines = cmd->data[1];
			uint8_t line_num;

			for (line_num = 0; line_num < ADC_NUMOF; line_num++) {
				adc_lines_en[line_num] = (lines >> line_num) & 1;
			}

			/* Re-initialize ADC lines */
			init_adc(false);

			break;
		}

	default:
		break;
	}

	return true;
}

#ifdef __cplusplus
}
#endif
