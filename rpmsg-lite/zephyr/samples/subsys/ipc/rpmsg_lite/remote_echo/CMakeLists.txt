cmake_minimum_required(VERSION 3.8)
set(RPMSG_BASE ../../../../../..)

include($ENV{ZEPHYR_BASE}/cmake/app/boilerplate.cmake NO_POLICY_SCOPE)
project(NONE)

if(CONFIG_SOC_SERIES_LPC54XXX)
set(SOC lpc5411x)
elseif(CONFIG_SOC_SERIES_IMX7_M4)
set(SOC imx7d_m4)
endif()

target_include_directories(app PRIVATE $ENV{ZEPHYR_BASE}/drivers)
target_include_directories(app PRIVATE ${RPMSG_BASE}/zephyr/samples/subsys/ipc/rpmsg_lite)
target_include_directories(app PRIVATE ${RPMSG_BASE}/lib/include)
target_include_directories(app PRIVATE ${RPMSG_BASE}/lib/include/platform/${SOC})

if(CONFIG_IPC_RPMSG_LITE)
target_sources(app PRIVATE ${RPMSG_BASE}/lib/common/llist.c)
target_sources(app PRIVATE ${RPMSG_BASE}/lib/rpmsg_lite/rpmsg_lite.c)
target_sources(app PRIVATE ${RPMSG_BASE}/lib/rpmsg_lite/rpmsg_ns.c)
target_sources(app PRIVATE ${RPMSG_BASE}/lib/rpmsg_lite/rpmsg_queue.c)
target_sources(app PRIVATE ${RPMSG_BASE}/lib/rpmsg_lite/porting/environment/rpmsg_env_zephyr.c)
target_sources(app PRIVATE ${RPMSG_BASE}/lib/rpmsg_lite/porting/platform/${SOC}/rpmsg_platform_zephyr_ipm.c)
target_sources(app PRIVATE ${RPMSG_BASE}/lib/virtio/virtqueue.c)
endif()

target_sources(app PRIVATE src/main_remote_echo.c)
