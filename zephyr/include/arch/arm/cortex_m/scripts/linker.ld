/*
 * Copyright (c) 2013-2014 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief Linker command/script file
 *
 * Linker script for the Cortex-M platforms.
 */

#define _LINKER
#define _ASMLANGUAGE

#include <autoconf.h>
#include <linker/sections.h>
#include <generated_dts_board.h>

#include <linker/linker-defs.h>
#include <linker/linker-tool.h>

/* physical address of RAM */
#ifdef CONFIG_XIP
  #define ROMABLE_REGION FLASH
  #define RAMABLE_REGION SRAM
#else
  #define ROMABLE_REGION SRAM
  #define RAMABLE_REGION SRAM
#endif

#if defined(CONFIG_XIP)
  #define _DATA_IN_ROM __data_rom_start
#else
  #define _DATA_IN_ROM
#endif

#if !defined(SKIP_TO_KINETIS_FLASH_CONFIG)
  #define SKIP_TO_KINETIS_FLASH_CONFIG
#endif

#define ROM_ADDR (CONFIG_FLASH_BASE_ADDRESS + CONFIG_FLASH_LOAD_OFFSET)
#ifdef CONFIG_TI_CCFG_PRESENT
  #define CCFG_SIZE 88
  #define ROM_SIZE (CONFIG_FLASH_SIZE*1K - CONFIG_FLASH_LOAD_OFFSET - \
		    CCFG_SIZE)
  #define CCFG_ADDR (ROM_ADDR + ROM_SIZE)
#else
#if CONFIG_FLASH_LOAD_SIZE > 0
  #define ROM_SIZE CONFIG_FLASH_LOAD_SIZE
#else
  #define ROM_SIZE (CONFIG_FLASH_SIZE*1K - CONFIG_FLASH_LOAD_OFFSET)
#endif
#endif

#if defined(CONFIG_XIP)
	#if defined(CONFIG_IS_BOOTLOADER)
		#define RAM_SIZE (CONFIG_BOOTLOADER_SRAM_SIZE * 1K)
		#define RAM_ADDR (CONFIG_SRAM_BASE_ADDRESS + \
			(CONFIG_SRAM_SIZE * 1K - RAM_SIZE))
	#else
		#define RAM_SIZE (CONFIG_SRAM_SIZE * 1K)
		#define RAM_ADDR CONFIG_SRAM_BASE_ADDRESS
	#endif
#else
	#define RAM_SIZE (CONFIG_SRAM_SIZE * 1K - CONFIG_BOOTLOADER_SRAM_SIZE * 1K)
	#define RAM_ADDR CONFIG_SRAM_BASE_ADDRESS
#endif

MEMORY
    {
    FLASH                 (rx) : ORIGIN = ROM_ADDR, LENGTH = ROM_SIZE
#ifdef CONFIG_TI_CCFG_PRESENT
    FLASH_CCFG            (rwx): ORIGIN = CCFG_ADDR, LENGTH = CCFG_SIZE
#endif
#ifdef CONFIG_CCM_BASE_ADDRESS
    CCM                   (rw) : ORIGIN = CONFIG_CCM_BASE_ADDRESS, LENGTH = CONFIG_CCM_SIZE * 1K
#endif
    SRAM                  (wx) : ORIGIN = RAM_ADDR, LENGTH = RAM_SIZE

    /* Used by and documented in include/linker/intlist.ld */
    IDT_LIST  (wx)      : ORIGIN = (RAM_ADDR + RAM_SIZE), LENGTH = 2K
    }

ENTRY(CONFIG_KERNEL_ENTRY)

SECTIONS
    {
    GROUP_START(ROMABLE_REGION)

	_image_rom_start = ROM_ADDR;

    SECTION_PROLOGUE(_TEXT_SECTION_NAME,,)
	{
#ifdef CONFIG_CC3220SF_DEBUG
	/* Add CC3220SF flash header to disable flash verification */
	. = 0x0;
	KEEP(*(.dbghdr))
	KEEP(*(".dbghdr.*"))
#endif
	. = CONFIG_TEXT_SECTION_OFFSET;

#if defined(CONFIG_SW_VECTOR_RELAY)
	KEEP(*(.vector_relay_table))
	KEEP(*(".vector_relay_table.*"))
	KEEP(*(.vector_relay_handler))
	KEEP(*(".vector_relay_handler.*"))
#endif

	_vector_start = .;
	KEEP(*(.exc_vector_table))
	KEEP(*(".exc_vector_table.*"))

	KEEP(*(IRQ_VECTOR_TABLE))

	KEEP(*(.openocd_dbg))
	KEEP(*(".openocd_dbg.*"))

	/* Kinetis has to write 16 bytes at 0x400 */
	SKIP_TO_KINETIS_FLASH_CONFIG
	KEEP(*(.kinetis_flash_config))
	KEEP(*(".kinetis_flash_config.*"))

#ifdef CONFIG_GEN_SW_ISR_TABLE
	KEEP(*(SW_ISR_TABLE))
#endif
	_vector_end = .;
	_image_text_start = .;
	*(.text)
	*(".text.*")
	*(.gnu.linkonce.t.*)

#include <linker/priv_stacks-text.ld>
#include <linker/kobject-text.ld>

	} GROUP_LINK_IN(ROMABLE_REGION)

	_image_text_end = .;

	SECTION_PROLOGUE(.ARM.exidx,,)
	{
	/*
	 * This section, related to stack and exception unwinding, is placed
	 * explicitly to prevent it from being shared between multiple regions.
	 * It  must be defined for gcc to support 64-bit math and avoid
	 * section overlap.
	 */
	__exidx_start = .;
#if defined (__GCC_LINKER_CMD__)
	*(.ARM.exidx* gnu.linkonce.armexidx.*)
#endif
	__exidx_end = .;
	} GROUP_LINK_IN(ROMABLE_REGION)

	_image_rodata_start = .;

#include <linker/common-rom.ld>

    SECTION_PROLOGUE(_RODATA_SECTION_NAME,,)
	{
	*(.rodata)
	*(".rodata.*")
	*(.gnu.linkonce.r.*)

#ifdef CONFIG_CUSTOM_RODATA_LD
/* Located in project source directory */
#include <custom-rodata.ld>
#endif

#include <linker/priv_stacks-rom.ld>
#include <linker/kobject-rom.ld>

	/*
	 * For XIP images, in order to avoid the situation when __data_rom_start
	 * is 32-bit aligned, but the actual data is placed right after rodata
	 * section, which may not end exactly at 32-bit border, pad rodata
	 * section, so __data_rom_start points at data and it is 32-bit aligned.
	 *
	 * On non-XIP images this may enlarge image size up to 3 bytes. This
	 * generally is not an issue, since modern ROM and FLASH memory is
	 * usually 4k aligned.
	 */
	. = ALIGN(4);
	} GROUP_LINK_IN(ROMABLE_REGION)

#if defined CONFIG_ARM_FIRMWARE_HAS_SECURE_ENTRY_FUNCS
#if CONFIG_ARM_NSC_REGION_BASE_ADDRESS != 0
       SECTION_PROLOGUE(.gnu.sgstubs,CONFIG_ARM_NSC_REGION_BASE_ADDRESS,)
#else
       SECTION_PROLOGUE(.gnu.sgstubs,,)
#endif /* CONFIG_ARM_NSC_REGION_BASE_ADDRESS != 0 */
       {
       . = ALIGN(4);
       __sg_start = .;
       *(.gnu*)
       . = ALIGN(4);
       __sg_end = .;
       __sg_size = __sg_end - __sg_start;
       } GROUP_LINK_IN(ROMABLE_REGION)
#endif /* CONFIG_ARM_FIRMWARE_HAS_SECURE_ENTRY_FUNCS */

	_image_rodata_end = .;
	_image_rom_end = .;

    GROUP_END(ROMABLE_REGION)

/* Some TI SoCs have a special configuration footer, at the end of flash. */
#ifdef CONFIG_TI_CCFG_PRESENT
    SECTION_PROLOGUE(.ti_ccfg,,)
    {
    KEEP(*(TI_CCFG))
    } > FLASH_CCFG
#endif

    GROUP_START(RAMABLE_REGION)


#if defined(CONFIG_SOC_SERIES_STM32F0X) && !defined(CONFIG_IS_BOOTLOADER)
    /* Must be first in ramable region */
	SECTION_PROLOGUE(.st_stm32f0x_vt,(NOLOAD),)
	{
        _ram_vector_start = .;
        . += _vector_end - _vector_start;
        _ram_vector_end = .;
	} GROUP_DATA_LINK_IN(RAMABLE_REGION, RAMABLE_REGION)
#endif

#if defined(CONFIG_SW_VECTOR_RELAY)
	/* Reserved 4 bytes to save vector table base address */
	SECTION_PROLOGUE(.vt_pointer,(NOLOAD),)
	{
		*(.vt_pointer_section)
		*(".vt_pointer_section.*")
	}
#endif

#include <app_data_alignment.ld>

#if defined(CONFIG_APP_SHARED_MEM)
#if defined(CONFIG_MPU_REQUIRES_POWER_OF_TWO_ALIGNMENT)
#define APP_SHARED_ALIGN . = ALIGN(4);
		#include <app_smem.ld>
#else
	SECTION_PROLOGUE(_APP_SMEM_SECTION_NAME, (OPTIONAL),)
	{
		. = ALIGN(4);
		_image_ram_start = .;
		_app_smem_start = .;
		APP_SMEM_SECTION()
		_app_smem_end = .;
		. = ALIGN(4);
	} GROUP_DATA_LINK_IN(RAMABLE_REGION, ROMABLE_REGION)
#endif  /* CONFIG_MPU_REQUIRES_POWER_OF_TWO_ALIGNMENT */

	_app_smem_size = _app_smem_end - _app_smem_start;
	_app_smem_rom_start = LOADADDR(_APP_SMEM_SECTION_NAME);
#endif  /* CONFIG_APP_SHARED_MEM */

#ifdef CONFIG_APPLICATION_MEMORY
	SECTION_DATA_PROLOGUE(_APP_DATA_SECTION_NAME, (OPTIONAL),)
	{

		__app_ram_start = .;
		__app_data_ram_start = .;
		APP_INPUT_SECTION(.data)
		APP_INPUT_SECTION(".data.*")
		__app_data_ram_end = .;
	} GROUP_DATA_LINK_IN(RAMABLE_REGION, ROMABLE_REGION)

	__app_data_rom_start = LOADADDR(_APP_DATA_SECTION_NAME);

	SECTION_PROLOGUE(_APP_BSS_SECTION_NAME, (NOLOAD OPTIONAL),)
	{
		__app_bss_start = .;
		APP_INPUT_SECTION(.bss)
		APP_INPUT_SECTION(".bss.*")
		APP_INPUT_SECTION(COMMON)
		__app_bss_end = .;
	} GROUP_DATA_LINK_IN(RAMABLE_REGION, RAMABLE_REGION)

	__app_bss_num_words = (__app_bss_end - __app_bss_start) >> 2;

	SECTION_PROLOGUE(_APP_NOINIT_SECTION_NAME, (NOLOAD OPTIONAL),)
	{
		APP_INPUT_SECTION(.noinit)
		APP_INPUT_SECTION(".noinit.*")
	} GROUP_DATA_LINK_IN(RAMABLE_REGION, RAMABLE_REGION)

	__app_last_address_used = .;

	/* Pad out application ram area to make MPU friendly */
	SECTION_PROLOGUE(app_pad, (NOLOAD OPTIONAL),)
	{
		. = ALIGN(_app_data_align);
	} GROUP_DATA_LINK_IN(RAMABLE_REGION, RAMABLE_REGION)
	__app_ram_end = .;
	__app_ram_size = __app_ram_end - __app_ram_start;
#endif /* CONFIG_APPLICATION_MEMORY */

    SECTION_DATA_PROLOGUE(_BSS_SECTION_NAME,(NOLOAD),)
	{
        /*
         * For performance, BSS section is assumed to be 4 byte aligned and
         * a multiple of 4 bytes
         */
        . = ALIGN(4);
	__bss_start = .;
	__kernel_ram_start = .;

	KERNEL_INPUT_SECTION(.bss)
	KERNEL_INPUT_SECTION(".bss.*")
	KERNEL_INPUT_SECTION(COMMON)
	*(".kernel_bss.*")

        /*
         * As memory is cleared in words only, it is simpler to ensure the BSS
         * section ends on a 4 byte boundary. This wastes a maximum of 3 bytes.
		 */
	__bss_end = ALIGN(4);
	} GROUP_DATA_LINK_IN(RAMABLE_REGION, RAMABLE_REGION)

    SECTION_PROLOGUE(_NOINIT_SECTION_NAME,(NOLOAD),)
        {
        /*
         * This section is used for non-initialized objects that
         * will not be cleared during the boot process.
         */
        KERNEL_INPUT_SECTION(.noinit)
        KERNEL_INPUT_SECTION(".noinit.*")
	*(".kernel_noinit.*")

        } GROUP_LINK_IN(RAMABLE_REGION)

    SECTION_DATA_PROLOGUE(_DATA_SECTION_NAME,,)
	{
	__data_ram_start = .;
	KERNEL_INPUT_SECTION(.data)
	KERNEL_INPUT_SECTION(".data.*")
	*(".kernel.*")

#ifdef CONFIG_CUSTOM_RWDATA_LD
/* Located in project source directory */
#include <custom-rwdata.ld>
#endif

	} GROUP_DATA_LINK_IN(RAMABLE_REGION, ROMABLE_REGION)

    __data_rom_start = LOADADDR(_DATA_SECTION_NAME);

#include <linker/common-ram.ld>
#include <linker/priv_stacks.ld>
#include <linker/kobject.ld>

    __data_ram_end = .;


    /* Define linker symbols */

    _image_ram_end = .;
    _end = .; /* end of image */

    __kernel_ram_end = RAM_ADDR + RAM_SIZE;
    __kernel_ram_size = __kernel_ram_end - __kernel_ram_start;

    GROUP_END(RAMABLE_REGION)

#ifdef CONFIG_CCM_BASE_ADDRESS

    GROUP_START(CCM)

	SECTION_PROLOGUE(_CCM_BSS_SECTION_NAME, (NOLOAD OPTIONAL),SUBALIGN(4))
	{
		__ccm_start = .;
		__ccm_bss_start = .;
		*(.ccm_bss)
		*(".ccm_bss.*")
	} GROUP_LINK_IN(CCM)

	__ccm_bss_end = .;

	SECTION_PROLOGUE(_CCM_NOINIT_SECTION_NAME, (NOLOAD OPTIONAL),SUBALIGN(4))
	{
		__ccm_noinit_start = .;
		*(.ccm_noinit)
		*(".ccm_noinit.*")
	} GROUP_LINK_IN(CCM)

	__ccm_noinit_end = .;

	SECTION_PROLOGUE(_CCM_DATA_SECTION_NAME, (OPTIONAL),SUBALIGN(4))
	{
		__ccm_data_start = .;
		*(.ccm_data)
		*(".ccm_data.*")
	} GROUP_LINK_IN(CCM AT> ROMABLE_REGION)

	__ccm_data_end = .;
	__ccm_end = .;

	__ccm_data_rom_start = LOADADDR(_CCM_DATA_SECTION_NAME);

    GROUP_END(CCM)

#endif /* CONFIG_CCM_BASE_ADDRESS */

#ifdef CONFIG_CUSTOM_SECTIONS_LD
/* Located in project source directory */
#include <custom-sections.ld>
#endif

#ifdef CONFIG_GEN_ISR_TABLES
#include <linker/intlist.ld>
#endif

    }
