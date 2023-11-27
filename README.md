Place where I found a very similar issue but it still didn't help:
https://github.com/zephyrproject-rtos/zephyr/issues/1464

Original project:
https://github.com/diegosueiro/rpmsg-lite/wiki

# make
[  1%] Built target kobj_types_h_target
[  2%] Built target syscall_macros_h_target
[  3%] Generating misc/generated/syscalls.json
[  6%] Built target syscall_list_h_target
[  7%] Built target driver_validation_h_target
[  8%] Building C object zephyr/CMakeFiles/offsets.dir/arch/arm/core/offsets/offsets.c.o
cc: warning: ‘-mcpu=’ is deprecated; use ‘-mtune=’ or ‘-march=’ instead
cc: error: unrecognized argument in option ‘-mabi=aapcs’
cc: note: valid arguments to ‘-mabi=’ are: ms sysv
cc: error: unrecognized command line option ‘-mthumb’
make[2]: *** [zephyr/CMakeFiles/offsets.dir/build.make:76: zephyr/CMakeFiles/offsets.dir/arch/arm/core/offsets/offsets.c.o] Erro 1
make[1]: *** [CMakeFiles/Makefile2:1392: zephyr/CMakeFiles/offsets.dir/all] Erro 2
make: *** [Makefile:91: all] Erro 2

![Captura de tela de 2023-11-27 20-23-59](https://github.com/neuberfran/zephyr_old_version/assets/38701397/eb293061-495c-42ec-81fc-4b71ae33f2d7)
