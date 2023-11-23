Place where I found a very similar issue but it still didn't help:
https://github.com/zephyrproject-rtos/zephyr/issues/1464

Original project:
https://github.com/diegosueiro/rpmsg-lite/wiki

cmake -DBOARD=pico_pi_m4 ../..


-- Found PythonInterp: /usr/bin/python3 (found suitable version "3.8.10", minimum required is "3.4") 
-- Selected BOARD pico_pi_m4
Zephyr version: 1.13.99
Parsing Kconfig tree in /home/neuberfran/diegosueiro3/rpmsg-lite/zephyr/samples/subsys/ipc/rpmsg_lite/remote_echo/Kconfig
Loading /home/neuberfran/diegosueiro3/zephyr/boards/arm/pico_pi_m4/pico_pi_m4_defconfig as base
Merging /home/neuberfran/diegosueiro3/rpmsg-lite/zephyr/samples/subsys/ipc/rpmsg_lite/remote_echo/prj_pico_pi_m4.conf

warning: FLASH_SIZE (defined at /home/neuberfran/diegosueiro3/zephyr/boards/arm/pico_pi_m4/Kconfig.defconfig:12) defined without a type

warning: FLASH_BASE_ADDRESS (defined at /home/neuberfran/diegosueiro3/zephyr/boards/arm/pico_pi_m4/Kconfig.defconfig:14) defined without a type

/home/neuberfran/diegosueiro3/zephyr/boards/arm/pico_pi_m4/pico_pi_m4_defconfig:16: warning: attempt to assign the value "y" to the undefined symbol PINCTRL

warning: CPU_CORTEX_M4 (defined at arch/arm/core/cortex_m/Kconfig:39) has direct dependencies ARM with value n, but is currently being y-selected by the following symbols:
 - SOC_SERIES_IMX7_M4 (defined at soc/arm/nxp_imx/mcimx7_m4/Kconfig.series:8), with value y, direct dependencies <choice> (value: y), and select condition <choice> (value: y)

warning: CPU_CORTEX_M (defined at arch/arm/core/Kconfig:15) has direct dependencies ARM with value n, but is currently being y-selected by the following symbols:
 - SOC_SERIES_IMX7_M4 (defined at soc/arm/nxp_imx/mcimx7_m4/Kconfig.series:8), with value y, direct dependencies <choice> (value: y), and select condition <choice> (value: y)

warning: CPU_HAS_SYSTICK (defined at arch/arm/core/Kconfig:28) has direct dependencies ARM with value n, but is currently being y-selected by the following symbols:
 - SOC_SERIES_IMX7_M4 (defined at soc/arm/nxp_imx/mcimx7_m4/Kconfig.series:8), with value y, direct dependencies <choice> (value: y), and select condition <choice> (value: y)

Error: Aborting due to non-whitelisted Kconfig warning 'warning: FLASH_SIZE (defined at
/home/neuberfran/diegosueiro3/zephyr/boards/arm/pico_pi_m4/Kconfig.defconfig:12) defined without a
type'. Note: If this warning doesn't point to an actual problem, you can add it to the whitelist at
the top of /home/neuberfran/diegosueiro3/zephyr/scripts/kconfig/kconfig.py.

CMake Error at /home/neuberfran/diegosueiro3/zephyr/cmake/kconfig.cmake:158 (message):
  command failed with return code: 1
Call Stack (most recent call first):
  /home/neuberfran/diegosueiro3/zephyr/cmake/app/boilerplate.cmake:257 (include)





