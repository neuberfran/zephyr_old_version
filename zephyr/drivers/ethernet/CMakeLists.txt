zephyr_library_include_directories(${ZEPHYR_BASE}/subsys/net/l2)

zephyr_sources_ifdef(CONFIG_ETH_SAM_GMAC
	eth_sam_gmac.c
	phy_sam_gmac.c
	)
zephyr_sources_ifdef(CONFIG_ETH_DW		eth_dw.c)
zephyr_sources_ifdef(CONFIG_ETH_ENC28J60	eth_enc28j60.c)
zephyr_sources_ifdef(CONFIG_ETH_MCUX		eth_mcux.c)
zephyr_sources_ifdef(CONFIG_ETH_STM32_HAL	eth_stm32_hal.c)

if(CONFIG_ETH_NATIVE_POSIX)
	zephyr_library()
	zephyr_library_include_directories(${ZEPHYR_BASE}/subsys/net/l2)
	zephyr_library_compile_definitions(NO_POSIX_CHEATS)
	zephyr_library_compile_definitions(_BSD_SOURCE)
	zephyr_library_compile_definitions(_DEFAULT_SOURCE)
	zephyr_library_sources(
		eth_native_posix.c
		eth_native_posix_adapt.c
		)
endif()
