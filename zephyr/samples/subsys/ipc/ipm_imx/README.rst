.. _ipm_imx:

i.MX IPM example
################

Overview
********
This simple example can be used with multicore i.MX SoCs containing a
Messaging Unit peripheral. It reads the received data from the Messaging Unit
using the IPM and transmits it back unchanged. The information about
the received data is printed to the console. When sending the data back,
it blocks until the data are read by the remote side.

The i.MX Messaging Unit peripheral consists of four 32-bit transmit and receive
registers. The sample uses the option :option:`CONFIG_IPM_IMX_MAX_DATA_SIZE_4`,
which effectively creates four IPM channels. Selecting the option
:option:`CONFIG_IPM_IMX_MAX_DATA_SIZE_8` or
:option:`CONFIG_IPM_IMX_MAX_DATA_SIZE_16` would result in two 64-bit channels
or a single 128-bit channel respectively.

Note that this is just a simple demo to demonstrate the i.MX IPM functionality
and blocking sending of the data back is done in the interrupt handler, which
would not be appropriate for a real world application.

Building and Running the Zephyr Code
************************************

The sample requires that data are being sent by the remote core so it can echo
it back. The other core is usually not running Zephyr but Linux on i.MX
multicore processors. This remote code is in the linux subfolder.

This project outputs data to the console.
It can be built as follows:

.. zephyr-app-commands::
   :zephyr-app: samples/subsys/ipc/ipm_imx
   :board: udoo_neo_full_m4
   :goals: build flash
   :compact:

Follow the instructions in the :ref:`udoo_neo_full_m4` board documentation
for how to load the Zephyr binary to the desired core and execute it.

Building and Running the Linux Code
***********************************

The remote code is in the form of a loadable Linux kernel module. It creates
four character devices /dev/mu0 till /dev/mu3, which are used to read data from
and write data into the Messaging Unit.

The example has been tested on :ref:`udoo_neo_full_m4` board with the Linux
distribution UDOObuntu 2.2.0 running on the A9 core. The remote project can
be built on a Linux system as follows:

Clone the GIT repository with the UDOObuntu version of the Linux kernel:

.. code-block:: console

    $ git clone https://github.com/UDOOboard/linux_kernel.git

Navigate into the linux_kernel folder and remember its location:

.. code-block:: console

    $ cd linux_kernel
    $ export KERNEL_PATH=`pwd`

Checkout at SHA 5111ea94bbd0818035351fce39c83f1dbf81457b:

.. code-block:: console

    $ git checkout 5111ea94bbd0818035351fce39c83f1dbf81457b

Copy the file linux/zephyr_mu.patch into the linux_kernel folder and apply
the patch to disable kernel's own handling of Messaging Unit:

.. code-block:: console

    $ cp ${ZEPHYR_BASE}/samples/subsys/ipc/ipm_imx/linux/zephyr_mu.patch .
    $ git apply zephyr_mu.patch

Define the kernel architecture:

.. code-block:: console

    $ export ARCH=arm

Provide path to the compiler toolchain:

.. code-block:: console

    $ export CROSS_COMPILE=~/bin/gcc-arm-none-eabi-4_9-2015q3/bin/arm-none-eabi-

Create .config file for the UDOObuntu Linux kernel:

.. code-block:: console

    $ make udoo_neo_defconfig

Build the Linux kernel:

.. code-block:: console

    $ make

Create a folder for Linux kernel modules and build them:

.. code-block:: console

    $ mkdir /tmp/modpath/
    $ export INSTALL_MOD_PATH=/tmp/modpath/
    $ make modules_install

Boot the UDOO Neo board.

Replace the file /boot/zImage in the UDOObuntu filesystem with the file
arch/arm/boot/zImage from the build filesystem.

Replace the .dtb files in the UDOObuntu filesystem with the .dtb files
from the arch/arm/boot/dts/ folder in the build filesystem.

Copy the contents of the ${INSTALL_MOD_PATH}/lib from the build filesystem
into the /lib folder in the UDOObuntu filesystem.

Reboot the UDOO Neo board to apply the modified kernel.

Go to the folder with example Linux code:

.. code-block:: console

    $ cd ${ZEPHYR_BASE}/samples/subsys/ipc/ipm_imx/linux/

Make sure the LINUX_KERNEL and CROSS_COMPILE variables have been set
in the previous steps and build the kernel module:

.. code-block:: console

    $ make

Copy the Linux kernel module file mu.ko into the UDOObuntu filesystem.

Use serial console or SSH to log in the UDOObuntu system.

Load the Zephyr sample code to run on the M4 core.

Load the kernel module:

.. code-block:: console

    # insmod mu.ko

It creates four character devices:

.. code-block:: console

    # ls -la /dev/mu*
    crw------- 1 root root 247, 0 Jan  1 05:33 /dev/mu0
    crw------- 1 root root 247, 1 Jan  1 05:33 /dev/mu1
    crw------- 1 root root 247, 2 Jan  1 05:33 /dev/mu2
    crw------- 1 root root 247, 3 Jan  1 05:33 /dev/mu3

Each of the devices uses one pair of the Messaging Unit's receive and transmit
registers to communicate with the Zephyr application. One byte of the register
is used for payload length, other three for the payload. The Zephyr application
does not interpret this and just echoes the data back.

You can open any of the devices from the console for writing the payload:

.. code-block:: console

     # cat > /dev/mu0
     abcdefgh
     ^D

Open the device from another console for reading and observe the data echoed
back using the Zephyr application:

.. code-block:: console

     # cat /dev/mu0
     abcdefgh
     ^C

Close all open devices and remove the kernel module:

.. code-block:: console

    # rmmod mu

You can check the debug output of the kernel module:

.. code-block:: console

    # dmesg
    [ 1154.944467] mu_probe
    [ 1162.583453] open /dev/mu0, fd=0xa87a3e40
    [ 1167.700685] open /dev/mu0, fd=0xa85e6000
    [ 1167.700821] read /dev/mu0, fd=0xa85e6000, length=65536
    [ 1172.331693] write /dev/mu0, fd=0xa87a3e40, length=9
    [ 1172.331802] write /dev/mu0, fd=0xa87a3e40, written=9
    [ 1172.331832] mu_isr /dev/mu0 tx 3 bytes: 0x03636261
    [ 1172.331858] mu_isr /dev/mu0 tx 3 bytes: 0x03666564
    [ 1172.335270] mu_isr /dev/mu0 rx 3 bytes: 0x03636261
    [ 1172.335334] mu_isr /dev/mu0 tx 3 bytes: 0x030a6867
    [ 1172.335413] read /dev/mu0, fd=0xa85e6000, read 3 bytes
    [ 1172.335519] read /dev/mu0, fd=0xa85e6000, length=65536
    [ 1172.338824] mu_isr /dev/mu0 rx 3 bytes: 0x03666564
    [ 1172.338895] read /dev/mu0, fd=0xa85e6000, read 3 bytes
    [ 1172.338964] read /dev/mu0, fd=0xa85e6000, length=65536
    [ 1172.342390] mu_isr /dev/mu0 rx 3 bytes: 0x030a6867
    [ 1172.342804] read /dev/mu0, fd=0xa85e6000, read 3 bytes
    [ 1172.342893] read /dev/mu0, fd=0xa85e6000, length=65536
    [ 1173.526658] close /dev/mu0, fd=0xa87a3e40
    [ 1176.765493] read /dev/mu0, fd=0xa85e6000, read 0 bytes
    [ 1176.765893] close /dev/mu0, fd=0xa85e6000
    [ 1186.156877] mu_remove

Sample Output
=============

.. code-block:: console

    ***** Booting Zephyr OS v1.12.0-291-g8cc508b *****
    IPM initialized, data size = 4
    ipm_callback: id = 0, data = 0x03323130
    ipm_callback: id = 0, data = 0x03353433
    ipm_callback: id = 0, data = 0x03383736
    ipm_callback: id = 0, data = 0x02000a39
