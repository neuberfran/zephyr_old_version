/*
 * Copyright (c) 2018, NXP
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef _RPMSG_CONFIG_H
#define _RPMSG_CONFIG_H

/*
 * RPMsg config values.
 * See $RPMSG_BASE/lib/ipc/rpmsg_lite/lib/include/rpmsg_default_config.h
 * for the list of all config items.
 */

#define RL_MS_PER_INTERVAL (1)

#define RL_BUFFER_PAYLOAD_SIZE (496)

#define RL_API_HAS_ZEROCOPY (1)

#define RL_USE_STATIC_API (0)

#ifdef CONFIG_SOC_SERIES_IMX7_M4
#define RL_BUFFER_COUNT (256)
#endif

#endif /* _RPMSG_CONFIG_H */
