/diegosueiro3/rpmsg-lite/zephyr/samples/subsys/ipc/rpmsg_lite/remote_echo/build/pico_pi_m4$ cmake -DBOARD=pico_pi_m4 ../..
-- Selected BOARD pico_pi_m4
Zephyr version: 1.13.99
Parsing Kconfig tree in /home/neuberfran/diegosueiro3/rpmsg-lite/zephyr/samples/subsys/ipc/rpmsg_lite/remote_echo/Kconfig
Loading /home/neuberfran/diegosueiro3/rpmsg-lite/zephyr/samples/subsys/ipc/rpmsg_lite/remote_echo/build/pico_pi_m4/zephyr/.config as base

warning: CPU_CORTEX_M4 (defined at arch/arm/core/cortex_m/Kconfig:39) has direct dependencies ARM with value n, but is currently being y-selected by the following symbols:
 - SOC_SERIES_IMX7_M4 (defined at soc/arm/nxp_imx/mcimx7_m4/Kconfig.series:8), with value y, direct dependencies <choice> (value: y), and select condition <choice> (value: y)

warning: CPU_CORTEX_M (defined at arch/arm/core/Kconfig:15) has direct dependencies ARM with value n, but is currently being y-selected by the following symbols:
 - SOC_SERIES_IMX7_M4 (defined at soc/arm/nxp_imx/mcimx7_m4/Kconfig.series:8), with value y, direct dependencies <choice> (value: y), and select condition <choice> (value: y)

warning: CPU_HAS_SYSTICK (defined at arch/arm/core/Kconfig:28) has direct dependencies ARM with value n, but is currently being y-selected by the following symbols:
 - SOC_SERIES_IMX7_M4 (defined at soc/arm/nxp_imx/mcimx7_m4/Kconfig.series:8), with value y, direct dependencies <choice> (value: y), and select condition <choice> (value: y)
CMake Warning at /home/neuberfran/diegosueiro3/zephyr/cmake/toolchain/gnuarmemb.cmake:4 (message):
  GCCARMEMB_TOOLCHAIN_PATH is deprecated, please use GNUARMEMB_TOOLCHAIN_PATH
  instead
Call Stack (most recent call first):
  /home/neuberfran/diegosueiro3/zephyr/cmake/toolchain.cmake:52 (include)
  /home/neuberfran/diegosueiro3/zephyr/cmake/app/boilerplate.cmake:258 (include)
  CMakeLists.txt:4 (include)


-- Generating zephyr/include/generated/generated_dts_board.h
-- Cache files will be written to: /home/neuberfran/.cache/zephyr
-- 
*******************************
*** NOTE TO KCONFIG AUTHORS ***
*******************************

The behavior of Kconfig 'default' properties in Zephyr has changed. The 
earliest default with a satisfied condition is now used, instead of the 
last one. This is standard Kconfig behavior.

See http://docs.zephyrproject.org/latest/porting/board_porting.html#old-zephyr-kconfig-behavior-for-defaults.

To get rid of this note, create a file called 'hide-defaults-note' in the 
Zephyr root directory. An empty file is fine.
-- Configuring done
-- Generating done
-- Build files have been written to: /home/neuberfran/diegosueiro3/rpmsg-lite/zephyr/samples/subsys/ipc/rpmsg_lite/remote_echo/build/pico_pi_m4
neuberfran@neuberfran2004:~/diegosueiro3/rpmsg-lite/zephyr/samples/subsys/ipc/rpmsg_lite/remote_echo/build/pico_pi_m4$ make
[  1%] Built target kobj_types_h_target
[  2%] Built target syscall_macros_h_target
[  3%] Generating misc/generated/syscalls.json
[  6%] Built target syscall_list_h_target
[  7%] Built target driver_validation_h_target
[  8%] Building C object zephyr/CMakeFiles/offsets.dir/arch/arm/core/offsets/offsets.c.obj
In file included from /home/neuberfran/diegosueiro3/zephyr/include/arch/x86/irq_controller.h:23,
                 from /home/neuberfran/diegosueiro3/zephyr/include/arch/x86/arch.h:18,
                 from /home/neuberfran/diegosueiro3/zephyr/include/arch/cpu.h:13,
                 from /home/neuberfran/diegosueiro3/zephyr/include/kernel_includes.h:34,
                 from /home/neuberfran/diegosueiro3/zephyr/include/kernel.h:17,
                 from /home/neuberfran/diegosueiro3/zephyr/kernel/include/kernel_structs.h:10,
                 from /home/neuberfran/diegosueiro3/zephyr/arch/arm/core/offsets/offsets.c:26:
/home/neuberfran/diegosueiro3/zephyr/include/drivers/sysapic.h: In function '__irq_controller_eoi':
/home/neuberfran/diegosueiro3/zephyr/include/drivers/sysapic.h:39:20: error: 'CONFIG_LOAPIC_BASE_ADDRESS' undeclared (first use in this function)
   39 |  *(volatile int *)(CONFIG_LOAPIC_BASE_ADDRESS + LOAPIC_EOI) = 0;
      |                    ^~~~~~~~~~~~~~~~~~~~~~~~~~
/home/neuberfran/diegosueiro3/zephyr/include/drivers/sysapic.h:39:20: note: each undeclared identifier is reported only once for each function it appears in
In file included from /home/neuberfran/diegosueiro3/zephyr/include/arch/cpu.h:13,
                 from /home/neuberfran/diegosueiro3/zephyr/include/kernel_includes.h:34,
                 from /home/neuberfran/diegosueiro3/zephyr/include/kernel.h:17,
                 from /home/neuberfran/diegosueiro3/zephyr/kernel/include/kernel_structs.h:10,
                 from /home/neuberfran/diegosueiro3/zephyr/arch/arm/core/offsets/offsets.c:26:
/home/neuberfran/diegosueiro3/zephyr/include/arch/x86/arch.h: At top level:
/home/neuberfran/diegosueiro3/zephyr/include/arch/x86/arch.h:21:10: fatal error: mmustructs.h: No such file or directory
   21 | #include <mmustructs.h>
      |          ^~~~~~~~~~~~~~
compilation terminated.
make[2]: *** [zephyr/CMakeFiles/offsets.dir/build.make:76: zephyr/CMakeFiles/offsets.dir/arch/arm/core/offsets/offsets.c.obj] Erro 1
make[1]: *** [CMakeFiles/Makefile2:1392: zephyr/CMakeFiles/offsets.dir/all] Erro 2
make: *** [Makefile:91: all] Erro 2
neuberfran@neuberfran2004:~/diegosueiro3/rpmsg-lite/zephyr/samples/subsys/ipc/rpmsg_lite/remote_echo/build/pico_pi_m4$ make
[  1%] Built target kobj_types_h_target
[  2%] Built target syscall_macros_h_target
[  3%] Generating misc/generated/syscalls.json
[  6%] Built target syscall_list_h_target
[  7%] Built target driver_validation_h_target
[  8%] Building C object zephyr/CMakeFiles/offsets.dir/arch/arm/core/offsets/offsets.c.obj
In file included from /home/neuberfran/diegosueiro3/zephyr/include/arch/x86/irq_controller.h:23,
                 from /home/neuberfran/diegosueiro3/zephyr/include/arch/x86/arch.h:18,
                 from /home/neuberfran/diegosueiro3/zephyr/include/arch/cpu.h:13,
                 from /home/neuberfran/diegosueiro3/zephyr/include/kernel_includes.h:34,
                 from /home/neuberfran/diegosueiro3/zephyr/include/kernel.h:17,
                 from /home/neuberfran/diegosueiro3/zephyr/kernel/include/kernel_structs.h:10,
                 from /home/neuberfran/diegosueiro3/zephyr/arch/arm/core/offsets/offsets.c:26:
/home/neuberfran/diegosueiro3/zephyr/include/drivers/sysapic.h: In function '__irq_controller_eoi':
/home/neuberfran/diegosueiro3/zephyr/include/drivers/sysapic.h:39:20: error: 'CONFIG_LOAPIC_BASE_ADDRESS' undeclared (first use in this function)
   39 |  *(volatile int *)(CONFIG_LOAPIC_BASE_ADDRESS + LOAPIC_EOI) = 0;
      |                    ^~~~~~~~~~~~~~~~~~~~~~~~~~
/home/neuberfran/diegosueiro3/zephyr/include/drivers/sysapic.h:39:20: note: each undeclared identifier is reported only once for each function it appears in
In file included from /home/neuberfran/diegosueiro3/zephyr/include/arch/cpu.h:13,
                 from /home/neuberfran/diegosueiro3/zephyr/include/kernel_includes.h:34,
                 from /home/neuberfran/diegosueiro3/zephyr/include/kernel.h:17,
                 from /home/neuberfran/diegosueiro3/zephyr/kernel/include/kernel_structs.h:10,
                 from /home/neuberfran/diegosueiro3/zephyr/arch/arm/core/offsets/offsets.c:26:
/home/neuberfran/diegosueiro3/zephyr/include/arch/x86/arch.h: At top level:
/home/neuberfran/diegosueiro3/zephyr/include/arch/x86/arch.h:21:10: fatal error: mmustructs.h: No such file or directory
   21 | #include <mmustructs.h>
      |          ^~~~~~~~~~~~~~
compilation terminated.
make[2]: *** [zephyr/CMakeFiles/offsets.dir/build.make:76: zephyr/CMakeFiles/offsets.dir/arch/arm/core/offsets/offsets.c.obj] Erro 1
make[1]: *** [CMakeFiles/Makefile2:1392: zephyr/CMakeFiles/offsets.dir/all] Erro 2
make: *** [Makefile:91: all] Erro 2

![Captura de tela de 2023-06-30 17-57-38](https://github.com/neuberfran/zephyr_old_version/assets/38701397/b2ed3981-de8e-4173-bb69-e3d9210f3858)




