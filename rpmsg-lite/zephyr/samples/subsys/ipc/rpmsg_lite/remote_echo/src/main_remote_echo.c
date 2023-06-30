/*
 * Copyright (c) 2018, NXP
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr.h>
#include <misc/printk.h>
#include <device.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "rpmsg_lite.h"
#include "rpmsg_queue.h"
#include "rpmsg_ns.h"

#define APP_TASK_STACK_SIZE (1024)
#define LOCAL_EPT_ADDR (30)

#ifdef CONFIG_SOC_SERIES_IMX7_M4
/* Settings bellow aligned with Linux i.MX RPMsg side */
#define RPMSG_MAX_SIZE	256
#define RPMSG_LITE_LINK_ID (RL_PLATFORM_IMX7D_M4_LINK_ID)
#define RPMSG_LITE_SHMEM_BASE (0x9FFF0000)
#define RPMSG_LITE_NS_USED (1)
#define RPMSG_LITE_NS_ANNOUNCE_STRING "rpmsg-openamp-demo-channel"
#else
#error Please define RPMSG_MAX_SIZE, RPMSG_LITE_LINK_ID, RPMSG_LITE_SHMEM_BASE,\
 RPMSG_LITE_NS_USED and RPMSG_LITE_NS_ANNOUNCE_STRING values for the CPU used.
#endif

K_THREAD_STACK_DEFINE(thread_stack, APP_TASK_STACK_SIZE);
static struct k_thread thread_data;

void app_nameservice_isr_cb(unsigned int new_ept, const char *new_ept_name,
			    unsigned long flags, void *user_data)
{
}

void app_task(void *arg1, void *arg2, void *arg3)
{
	ARG_UNUSED(arg1);
	ARG_UNUSED(arg2);
	ARG_UNUSED(arg3);
	int recved = 0;
    	char buf[RPMSG_MAX_SIZE];
    	char rsp[RPMSG_MAX_SIZE];
    	int len;
	volatile unsigned long remote_addr;
	struct rpmsg_lite_endpoint *volatile rl_endpoint;
	volatile rpmsg_queue_handle rl_queue;
	struct rpmsg_lite_instance *volatile rl_instance;
#ifdef RPMSG_LITE_NS_USED
	volatile rpmsg_ns_handle ns_handle;
#endif /*RPMSG_LITE_NS_USED*/

	/* Initialize RPMsg Core and create virtqueues */
	rl_instance = rpmsg_lite_remote_init((void *)RPMSG_LITE_SHMEM_BASE,
					     RPMSG_LITE_LINK_ID, RL_NO_FLAGS);

	printk("Waiting for Master.\r\n");
	while (!rpmsg_lite_is_link_up(rl_instance)) {
	}

	rl_queue = rpmsg_queue_create(rl_instance);
	rl_endpoint = rpmsg_lite_create_ept(rl_instance, LOCAL_EPT_ADDR,
				       rpmsg_queue_rx_cb, rl_queue);

#ifdef RPMSG_LITE_NS_USED
	ns_handle = rpmsg_ns_bind(rl_instance, app_nameservice_isr_cb, NULL);
	rpmsg_ns_announce(rl_instance, rl_endpoint,
			  RPMSG_LITE_NS_ANNOUNCE_STRING, RL_NS_CREATE);
	printk("Nameservice announce sent.\r\n");
#endif /*RPMSG_LITE_NS_USED*/

	while(1)
	{
		rpmsg_queue_recv(rl_instance, rl_queue,
				 (unsigned long*)&remote_addr, (char*)buf,
				 sizeof(buf), &recved, RL_BLOCK);
		printk("\nFrom endpoint 0x%X received %d bytes:\n",
		       (unsigned int)remote_addr, recved);
		buf[recved] = '\0';
		printk("%s\n",buf);

		/* Format the echo response */
		len = snprintf(rsp, sizeof(rsp), "echo: %s\r\n", buf);
		printk("Sending %d bytes to endpoint 0x%X:\n", len,
		       (int)remote_addr);
		printk("%s",rsp);
		rpmsg_lite_send(rl_instance, rl_endpoint, remote_addr, rsp, len,
				RL_BLOCK);
	}
}

void main(void)
{
	printk("======== rpmsg_lite remote_echo ========\n");

	k_thread_create(&thread_data, thread_stack, APP_TASK_STACK_SIZE,
			(k_thread_entry_t)app_task,
			NULL, NULL, NULL, K_PRIO_COOP(7), 0, 0);
}
