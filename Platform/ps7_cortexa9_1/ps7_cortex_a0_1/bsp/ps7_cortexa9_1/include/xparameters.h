#ifndef XPARAMETERS_H   /* prevent circular inclusions */
#define XPARAMETERS_H   /* by using protection macros */

/* Definition for CPU ID */
#define XPAR_CPU_ID 1U

/* Definitions for peripheral PS7_CORTEXA9_1 */
#define XPAR_PS7_CORTEXA9_1_CPU_CLK_FREQ_HZ 800000000


/******************************************************************/

/* Canonical definitions for peripheral PS7_CORTEXA9_1 */
#define XPAR_CPU_CORTEXA9_0_CPU_CLK_FREQ_HZ 800000000


/******************************************************************/

#include "xparameters_ps.h"

#define STDIN_BASEADDRESS 0xE0000000
#define STDOUT_BASEADDRESS 0xE0000000

/******************************************************************/

/* Platform specific definitions */
#define PLATFORM_ZYNQ
 
/* Definitions for sleep timer configuration */
#define XSLEEP_TIMER_IS_DEFAULT_TIMER
 
 
/******************************************************************/
/* Definitions for driver AXIS_SWITCH */
#define XPAR_XAXIS_SWITCH_NUM_INSTANCES 3

/* Definitions for peripheral GTX_SWITCH */
#define XPAR_GTX_SWITCH_DEVICE_ID 0
#define XPAR_GTX_SWITCH_BASEADDR 0x40010000
#define XPAR_GTX_SWITCH_HIGHADDR 0x4001FFFF
#define XPAR_GTX_SWITCH_NUM_SI 3
#define XPAR_GTX_SWITCH_NUM_MI 2


/* Definitions for peripheral AXIS_SWITCH_0 */
#define XPAR_AXIS_SWITCH_0_DEVICE_ID 1
#define XPAR_AXIS_SWITCH_0_BASEADDR 0x400F0000
#define XPAR_AXIS_SWITCH_0_HIGHADDR 0x400FFFFF
#define XPAR_AXIS_SWITCH_0_NUM_SI 12
#define XPAR_AXIS_SWITCH_0_NUM_MI 10


/* Definitions for peripheral AXIS_SWITCH_1 */
#define XPAR_AXIS_SWITCH_1_DEVICE_ID 2
#define XPAR_AXIS_SWITCH_1_BASEADDR 0x40100000
#define XPAR_AXIS_SWITCH_1_HIGHADDR 0x4010FFFF
#define XPAR_AXIS_SWITCH_1_NUM_SI 1
#define XPAR_AXIS_SWITCH_1_NUM_MI 5


/******************************************************************/

/* Canonical definitions for peripheral GTX_SWITCH */
#define XPAR_AXIS_SWITCH_2_NUM_INSTANCES 0
#define XPAR_AXIS_SWITCH_2_DEVICE_ID XPAR_GTX_SWITCH_DEVICE_ID
#define XPAR_AXIS_SWITCH_2_BASEADDR 0x40010000
#define XPAR_AXIS_SWITCH_2_HIGHADDR 0x4001FFFF
#define XPAR_AXIS_SWITCH_2_NUM_SI 3
#define XPAR_AXIS_SWITCH_2_NUM_MI 2


/******************************************************************/


/* Definitions for peripheral PS7_DDR_0 */
#define XPAR_PS7_DDR_0_S_AXI_BASEADDR 0x00100000
#define XPAR_PS7_DDR_0_S_AXI_HIGHADDR 0x3FFFFFFF


/******************************************************************/

/* Definitions for driver DEVCFG */
#define XPAR_XDCFG_NUM_INSTANCES 1U

/* Definitions for peripheral PS7_DEV_CFG_0 */
#define XPAR_PS7_DEV_CFG_0_DEVICE_ID 0U
#define XPAR_PS7_DEV_CFG_0_BASEADDR 0xF8007000U
#define XPAR_PS7_DEV_CFG_0_HIGHADDR 0xF80070FFU


/******************************************************************/

/* Canonical definitions for peripheral PS7_DEV_CFG_0 */
#define XPAR_XDCFG_0_DEVICE_ID XPAR_PS7_DEV_CFG_0_DEVICE_ID
#define XPAR_XDCFG_0_BASEADDR 0xF8007000U
#define XPAR_XDCFG_0_HIGHADDR 0xF80070FFU


/******************************************************************/

/* Definitions for driver DMAPS */
#define XPAR_XDMAPS_NUM_INSTANCES 2

/* Definitions for peripheral PS7_DMA_NS */
#define XPAR_PS7_DMA_NS_DEVICE_ID 0
#define XPAR_PS7_DMA_NS_BASEADDR 0xF8004000
#define XPAR_PS7_DMA_NS_HIGHADDR 0xF8004FFF


/* Definitions for peripheral PS7_DMA_S */
#define XPAR_PS7_DMA_S_DEVICE_ID 1
#define XPAR_PS7_DMA_S_BASEADDR 0xF8003000
#define XPAR_PS7_DMA_S_HIGHADDR 0xF8003FFF


/******************************************************************/

/* Canonical definitions for peripheral PS7_DMA_NS */
#define XPAR_XDMAPS_0_DEVICE_ID XPAR_PS7_DMA_NS_DEVICE_ID
#define XPAR_XDMAPS_0_BASEADDR 0xF8004000
#define XPAR_XDMAPS_0_HIGHADDR 0xF8004FFF

/* Canonical definitions for peripheral PS7_DMA_S */
#define XPAR_XDMAPS_1_DEVICE_ID XPAR_PS7_DMA_S_DEVICE_ID
#define XPAR_XDMAPS_1_BASEADDR 0xF8003000
#define XPAR_XDMAPS_1_HIGHADDR 0xF8003FFF


/******************************************************************/

/* Definitions for driver EMACPS */
#define XPAR_XEMACPS_NUM_INSTANCES 1

/* Definitions for peripheral PS7_ETHERNET_0 */
#define XPAR_PS7_ETHERNET_0_DEVICE_ID 0
#define XPAR_PS7_ETHERNET_0_BASEADDR 0xE000B000
#define XPAR_PS7_ETHERNET_0_HIGHADDR 0xE000BFFF
#define XPAR_PS7_ETHERNET_0_ENET_CLK_FREQ_HZ 125000000
#define XPAR_PS7_ETHERNET_0_ENET_SLCR_1000MBPS_DIV0 8
#define XPAR_PS7_ETHERNET_0_ENET_SLCR_1000MBPS_DIV1 1
#define XPAR_PS7_ETHERNET_0_ENET_SLCR_100MBPS_DIV0 8
#define XPAR_PS7_ETHERNET_0_ENET_SLCR_100MBPS_DIV1 5
#define XPAR_PS7_ETHERNET_0_ENET_SLCR_10MBPS_DIV0 8
#define XPAR_PS7_ETHERNET_0_ENET_SLCR_10MBPS_DIV1 50
#define XPAR_PS7_ETHERNET_0_ENET_TSU_CLK_FREQ_HZ 0


/******************************************************************/

#define XPAR_PS7_ETHERNET_0_IS_CACHE_COHERENT 0
#define XPAR_XEMACPS_0_IS_CACHE_COHERENT 0
/* Canonical definitions for peripheral PS7_ETHERNET_0 */
#define XPAR_XEMACPS_0_DEVICE_ID XPAR_PS7_ETHERNET_0_DEVICE_ID
#define XPAR_XEMACPS_0_BASEADDR 0xE000B000
#define XPAR_XEMACPS_0_HIGHADDR 0xE000BFFF
#define XPAR_XEMACPS_0_ENET_CLK_FREQ_HZ 125000000
#define XPAR_XEMACPS_0_ENET_SLCR_1000Mbps_DIV0 8
#define XPAR_XEMACPS_0_ENET_SLCR_1000Mbps_DIV1 1
#define XPAR_XEMACPS_0_ENET_SLCR_100Mbps_DIV0 8
#define XPAR_XEMACPS_0_ENET_SLCR_100Mbps_DIV1 5
#define XPAR_XEMACPS_0_ENET_SLCR_10Mbps_DIV0 8
#define XPAR_XEMACPS_0_ENET_SLCR_10Mbps_DIV1 50
#define XPAR_XEMACPS_0_ENET_TSU_CLK_FREQ_HZ 0


/******************************************************************/


/* Peripheral Definitions for peripheral PS7_AFI_0 */
#define XPAR_PS7_AFI_0_S_AXI_BASEADDR 0xF8008000
#define XPAR_PS7_AFI_0_S_AXI_HIGHADDR 0xF8008FFF


/* Peripheral Definitions for peripheral PS7_AFI_1 */
#define XPAR_PS7_AFI_1_S_AXI_BASEADDR 0xF8009000
#define XPAR_PS7_AFI_1_S_AXI_HIGHADDR 0xF8009FFF


/* Peripheral Definitions for peripheral PS7_AFI_2 */
#define XPAR_PS7_AFI_2_S_AXI_BASEADDR 0xF800A000
#define XPAR_PS7_AFI_2_S_AXI_HIGHADDR 0xF800AFFF


/* Peripheral Definitions for peripheral PS7_AFI_3 */
#define XPAR_PS7_AFI_3_S_AXI_BASEADDR 0xF800B000
#define XPAR_PS7_AFI_3_S_AXI_HIGHADDR 0xF800BFFF


/* Peripheral Definitions for peripheral PS7_DDRC_0 */
#define XPAR_PS7_DDRC_0_S_AXI_BASEADDR 0xF8006000
#define XPAR_PS7_DDRC_0_S_AXI_HIGHADDR 0xF8006FFF


/* Peripheral Definitions for peripheral PS7_GLOBALTIMER_0 */
#define XPAR_PS7_GLOBALTIMER_0_S_AXI_BASEADDR 0xF8F00200
#define XPAR_PS7_GLOBALTIMER_0_S_AXI_HIGHADDR 0xF8F002FF


/* Peripheral Definitions for peripheral PS7_GPV_0 */
#define XPAR_PS7_GPV_0_S_AXI_BASEADDR 0xF8900000
#define XPAR_PS7_GPV_0_S_AXI_HIGHADDR 0xF89FFFFF


/* Peripheral Definitions for peripheral PS7_INTC_DIST_0 */
#define XPAR_PS7_INTC_DIST_0_S_AXI_BASEADDR 0xF8F01000
#define XPAR_PS7_INTC_DIST_0_S_AXI_HIGHADDR 0xF8F01FFF


/* Peripheral Definitions for peripheral PS7_IOP_BUS_CONFIG_0 */
#define XPAR_PS7_IOP_BUS_CONFIG_0_S_AXI_BASEADDR 0xE0200000
#define XPAR_PS7_IOP_BUS_CONFIG_0_S_AXI_HIGHADDR 0xE0200FFF


/* Peripheral Definitions for peripheral PS7_L2CACHEC_0 */
#define XPAR_PS7_L2CACHEC_0_S_AXI_BASEADDR 0xF8F02000
#define XPAR_PS7_L2CACHEC_0_S_AXI_HIGHADDR 0xF8F02FFF


/* Peripheral Definitions for peripheral PS7_OCMC_0 */
#define XPAR_PS7_OCMC_0_S_AXI_BASEADDR 0xF800C000
#define XPAR_PS7_OCMC_0_S_AXI_HIGHADDR 0xF800CFFF


/* Peripheral Definitions for peripheral PS7_PL310_0 */
#define XPAR_PS7_PL310_0_S_AXI_BASEADDR 0xF8F02000
#define XPAR_PS7_PL310_0_S_AXI_HIGHADDR 0xF8F02FFF


/* Peripheral Definitions for peripheral PS7_PMU_0 */
#define XPAR_PS7_PMU_0_S_AXI_BASEADDR 0xF8891000
#define XPAR_PS7_PMU_0_S_AXI_HIGHADDR 0xF8891FFF
#define XPAR_PS7_PMU_0_PMU1_S_AXI_BASEADDR 0xF8893000
#define XPAR_PS7_PMU_0_PMU1_S_AXI_HIGHADDR 0xF8893FFF


/* Peripheral Definitions for peripheral PS7_QSPI_LINEAR_0 */
#define XPAR_PS7_QSPI_LINEAR_0_S_AXI_BASEADDR 0xFC000000
#define XPAR_PS7_QSPI_LINEAR_0_S_AXI_HIGHADDR 0xFCFFFFFF


/* Peripheral Definitions for peripheral PS7_RAM_0 */
#define XPAR_PS7_RAM_0_S_AXI_BASEADDR 0x00000000
#define XPAR_PS7_RAM_0_S_AXI_HIGHADDR 0x0003FFFF


/* Peripheral Definitions for peripheral PS7_RAM_1 */
#define XPAR_PS7_RAM_1_S_AXI_BASEADDR 0xFFFC0000
#define XPAR_PS7_RAM_1_S_AXI_HIGHADDR 0xFFFFFFFF


/* Peripheral Definitions for peripheral PS7_SCUC_0 */
#define XPAR_PS7_SCUC_0_S_AXI_BASEADDR 0xF8F00000
#define XPAR_PS7_SCUC_0_S_AXI_HIGHADDR 0xF8F000FC


/* Peripheral Definitions for peripheral PS7_SLCR_0 */
#define XPAR_PS7_SLCR_0_S_AXI_BASEADDR 0xF8000000
#define XPAR_PS7_SLCR_0_S_AXI_HIGHADDR 0xF8000FFF


/* Peripheral Definitions for peripheral CTRL_DATAMOVER_DDR_PL */
#define XPAR_CTRL_DATAMOVER_DDR_PL_BASEADDR 0x40000000
#define XPAR_CTRL_DATAMOVER_DDR_PL_HIGHADDR 0x4000FFFF


/* Peripheral Definitions for peripheral TLV5626_AXI_LITE_TOP_0 */
#define XPAR_TLV5626_AXI_LITE_TOP_0_BASEADDR 0x40060000
#define XPAR_TLV5626_AXI_LITE_TOP_0_HIGHADDR 0x4006FFFF


/* Peripheral Definitions for peripheral UCI_AXI_MASTER */
#define XPAR_UCI_AXI_MASTER_BASEADDR 0x40070000
#define XPAR_UCI_AXI_MASTER_HIGHADDR 0x4007FFFF


/* Peripheral Definitions for peripheral CHANNEL_EXTRACT_AXIL_0 */
#define XPAR_CHANNEL_EXTRACT_AXIL_0_BASEADDR 0x40110000
#define XPAR_CHANNEL_EXTRACT_AXIL_0_HIGHADDR 0x4011FFFF


/* Peripheral Definitions for peripheral DATAMOVER_CTRL_AXI_LITE_0 */
#define XPAR_DATAMOVER_CTRL_AXI_LITE_0_BASEADDR 0x40120000
#define XPAR_DATAMOVER_CTRL_AXI_LITE_0_HIGHADDR 0x4012FFFF


/* Peripheral Definitions for peripheral ENCODER_TOP_AXILITE_1 */
#define XPAR_ENCODER_TOP_AXILITE_1_BASEADDR 0x40130000
#define XPAR_ENCODER_TOP_AXILITE_1_HIGHADDR 0x4013FFFF


/* Peripheral Definitions for peripheral ENCODER_TOP_AXILITE_2 */
#define XPAR_ENCODER_TOP_AXILITE_2_BASEADDR 0x40140000
#define XPAR_ENCODER_TOP_AXILITE_2_HIGHADDR 0x4014FFFF


/* Peripheral Definitions for peripheral ENCODER_TOP_AXILITE_3 */
#define XPAR_ENCODER_TOP_AXILITE_3_BASEADDR 0x40150000
#define XPAR_ENCODER_TOP_AXILITE_3_HIGHADDR 0x4015FFFF


/* Peripheral Definitions for peripheral ENCODER_TOP_AXILITE_4 */
#define XPAR_ENCODER_TOP_AXILITE_4_BASEADDR 0x40160000
#define XPAR_ENCODER_TOP_AXILITE_4_HIGHADDR 0x4016FFFF


/* Peripheral Definitions for peripheral TRIG_HW_AXILITE_0 */
#define XPAR_TRIG_HW_AXILITE_0_BASEADDR 0x401C0000
#define XPAR_TRIG_HW_AXILITE_0_HIGHADDR 0x401CFFFF


/******************************************************************/










































/* Canonical Definitions for peripheral UCI_AXI_MASTER */
#define XPAR_MCBCC_MST_AXI_TOP_0_BASEADDR 0x40070000
#define XPAR_MCBCC_MST_AXI_TOP_0_HIGHADDR 0x4007FFFF




/* Canonical Definitions for peripheral DATAMOVER_CTRL_AXI_LITE_0 */
#define XPAR_DATAMOVER_CTRL_AXI_LITE_1_BASEADDR 0x40120000
#define XPAR_DATAMOVER_CTRL_AXI_LITE_1_HIGHADDR 0x4012FFFF


/* Canonical Definitions for peripheral ENCODER_TOP_AXILITE_1 */
#define XPAR_ENCODER_TOP_AXILITE_0_BASEADDR 0x40130000
#define XPAR_ENCODER_TOP_AXILITE_0_HIGHADDR 0x4013FFFF










/******************************************************************/

/* Definitions for driver GPIO */
#define XPAR_XGPIO_NUM_INSTANCES 11

/* Definitions for peripheral MGT_CLK_GPIO */
#define XPAR_MGT_CLK_GPIO_BASEADDR 0x40020000
#define XPAR_MGT_CLK_GPIO_HIGHADDR 0x4002FFFF
#define XPAR_MGT_CLK_GPIO_DEVICE_ID 0
#define XPAR_MGT_CLK_GPIO_INTERRUPT_PRESENT 0
#define XPAR_MGT_CLK_GPIO_IS_DUAL 0


/* Definitions for peripheral PROG_GPIO */
#define XPAR_PROG_GPIO_BASEADDR 0x40030000
#define XPAR_PROG_GPIO_HIGHADDR 0x4003FFFF
#define XPAR_PROG_GPIO_DEVICE_ID 1
#define XPAR_PROG_GPIO_INTERRUPT_PRESENT 0
#define XPAR_PROG_GPIO_IS_DUAL 1


/* Definitions for peripheral PWR_GPIO */
#define XPAR_PWR_GPIO_BASEADDR 0x40040000
#define XPAR_PWR_GPIO_HIGHADDR 0x4004FFFF
#define XPAR_PWR_GPIO_DEVICE_ID 2
#define XPAR_PWR_GPIO_INTERRUPT_PRESENT 0
#define XPAR_PWR_GPIO_IS_DUAL 0


/* Definitions for peripheral VERSION_HW */
#define XPAR_VERSION_HW_BASEADDR 0x40080000
#define XPAR_VERSION_HW_HIGHADDR 0x4008FFFF
#define XPAR_VERSION_HW_DEVICE_ID 3
#define XPAR_VERSION_HW_INTERRUPT_PRESENT 0
#define XPAR_VERSION_HW_IS_DUAL 0


/* Definitions for peripheral AXI_GPIO_DSR */
#define XPAR_AXI_GPIO_DSR_BASEADDR 0x400A0000
#define XPAR_AXI_GPIO_DSR_HIGHADDR 0x400AFFFF
#define XPAR_AXI_GPIO_DSR_DEVICE_ID 4
#define XPAR_AXI_GPIO_DSR_INTERRUPT_PRESENT 0
#define XPAR_AXI_GPIO_DSR_IS_DUAL 1


/* Definitions for peripheral AXI_GPIO_LINK_PRO */
#define XPAR_AXI_GPIO_LINK_PRO_BASEADDR 0x400B0000
#define XPAR_AXI_GPIO_LINK_PRO_HIGHADDR 0x400BFFFF
#define XPAR_AXI_GPIO_LINK_PRO_DEVICE_ID 5
#define XPAR_AXI_GPIO_LINK_PRO_INTERRUPT_PRESENT 1
#define XPAR_AXI_GPIO_LINK_PRO_IS_DUAL 1


/* Definitions for peripheral LED */
#define XPAR_LED_BASEADDR 0x40170000
#define XPAR_LED_HIGHADDR 0x4017FFFF
#define XPAR_LED_DEVICE_ID 6
#define XPAR_LED_INTERRUPT_PRESENT 0
#define XPAR_LED_IS_DUAL 0


/* Definitions for peripheral LOG_AXI_GPIO_0 */
#define XPAR_LOG_AXI_GPIO_0_BASEADDR 0x40180000
#define XPAR_LOG_AXI_GPIO_0_HIGHADDR 0x4018FFFF
#define XPAR_LOG_AXI_GPIO_0_DEVICE_ID 7
#define XPAR_LOG_AXI_GPIO_0_INTERRUPT_PRESENT 0
#define XPAR_LOG_AXI_GPIO_0_IS_DUAL 0


/* Definitions for peripheral MISCELLANEOUS_GPIO_0 */
#define XPAR_MISCELLANEOUS_GPIO_0_BASEADDR 0x40190000
#define XPAR_MISCELLANEOUS_GPIO_0_HIGHADDR 0x4019FFFF
#define XPAR_MISCELLANEOUS_GPIO_0_DEVICE_ID 8
#define XPAR_MISCELLANEOUS_GPIO_0_INTERRUPT_PRESENT 1
#define XPAR_MISCELLANEOUS_GPIO_0_IS_DUAL 1


/* Definitions for peripheral ZERO_INJECTER_COUNT */
#define XPAR_ZERO_INJECTER_COUNT_BASEADDR 0x401E0000
#define XPAR_ZERO_INJECTER_COUNT_HIGHADDR 0x401EFFFF
#define XPAR_ZERO_INJECTER_COUNT_DEVICE_ID 9
#define XPAR_ZERO_INJECTER_COUNT_INTERRUPT_PRESENT 0
#define XPAR_ZERO_INJECTER_COUNT_IS_DUAL 1


/* Definitions for peripheral ZERO_INJECTER_START */
#define XPAR_ZERO_INJECTER_START_BASEADDR 0x401F0000
#define XPAR_ZERO_INJECTER_START_HIGHADDR 0x401FFFFF
#define XPAR_ZERO_INJECTER_START_DEVICE_ID 10
#define XPAR_ZERO_INJECTER_START_INTERRUPT_PRESENT 0
#define XPAR_ZERO_INJECTER_START_IS_DUAL 0


/******************************************************************/

/* Canonical definitions for peripheral MGT_CLK_GPIO */
#define XPAR_GPIO_0_BASEADDR 0x40020000
#define XPAR_GPIO_0_HIGHADDR 0x4002FFFF
#define XPAR_GPIO_0_DEVICE_ID XPAR_MGT_CLK_GPIO_DEVICE_ID
#define XPAR_GPIO_0_INTERRUPT_PRESENT 0
#define XPAR_GPIO_0_IS_DUAL 0

/* Canonical definitions for peripheral PROG_GPIO */
#define XPAR_GPIO_1_BASEADDR 0x40030000
#define XPAR_GPIO_1_HIGHADDR 0x4003FFFF
#define XPAR_GPIO_1_DEVICE_ID XPAR_PROG_GPIO_DEVICE_ID
#define XPAR_GPIO_1_INTERRUPT_PRESENT 0
#define XPAR_GPIO_1_IS_DUAL 1

/* Canonical definitions for peripheral PWR_GPIO */
#define XPAR_GPIO_2_BASEADDR 0x40040000
#define XPAR_GPIO_2_HIGHADDR 0x4004FFFF
#define XPAR_GPIO_2_DEVICE_ID XPAR_PWR_GPIO_DEVICE_ID
#define XPAR_GPIO_2_INTERRUPT_PRESENT 0
#define XPAR_GPIO_2_IS_DUAL 0

/* Canonical definitions for peripheral VERSION_HW */
#define XPAR_GPIO_3_BASEADDR 0x40080000
#define XPAR_GPIO_3_HIGHADDR 0x4008FFFF
#define XPAR_GPIO_3_DEVICE_ID XPAR_VERSION_HW_DEVICE_ID
#define XPAR_GPIO_3_INTERRUPT_PRESENT 0
#define XPAR_GPIO_3_IS_DUAL 0

/* Canonical definitions for peripheral AXI_GPIO_DSR */
#define XPAR_GPIO_4_BASEADDR 0x400A0000
#define XPAR_GPIO_4_HIGHADDR 0x400AFFFF
#define XPAR_GPIO_4_DEVICE_ID XPAR_AXI_GPIO_DSR_DEVICE_ID
#define XPAR_GPIO_4_INTERRUPT_PRESENT 0
#define XPAR_GPIO_4_IS_DUAL 1

/* Canonical definitions for peripheral AXI_GPIO_LINK_PRO */
#define XPAR_GPIO_5_BASEADDR 0x400B0000
#define XPAR_GPIO_5_HIGHADDR 0x400BFFFF
#define XPAR_GPIO_5_DEVICE_ID XPAR_AXI_GPIO_LINK_PRO_DEVICE_ID
#define XPAR_GPIO_5_INTERRUPT_PRESENT 1
#define XPAR_GPIO_5_IS_DUAL 1

/* Canonical definitions for peripheral LED */
#define XPAR_GPIO_6_BASEADDR 0x40170000
#define XPAR_GPIO_6_HIGHADDR 0x4017FFFF
#define XPAR_GPIO_6_DEVICE_ID XPAR_LED_DEVICE_ID
#define XPAR_GPIO_6_INTERRUPT_PRESENT 0
#define XPAR_GPIO_6_IS_DUAL 0

/* Canonical definitions for peripheral LOG_AXI_GPIO_0 */
#define XPAR_GPIO_7_BASEADDR 0x40180000
#define XPAR_GPIO_7_HIGHADDR 0x4018FFFF
#define XPAR_GPIO_7_DEVICE_ID XPAR_LOG_AXI_GPIO_0_DEVICE_ID
#define XPAR_GPIO_7_INTERRUPT_PRESENT 0
#define XPAR_GPIO_7_IS_DUAL 0

/* Canonical definitions for peripheral MISCELLANEOUS_GPIO_0 */
#define XPAR_GPIO_8_BASEADDR 0x40190000
#define XPAR_GPIO_8_HIGHADDR 0x4019FFFF
#define XPAR_GPIO_8_DEVICE_ID XPAR_MISCELLANEOUS_GPIO_0_DEVICE_ID
#define XPAR_GPIO_8_INTERRUPT_PRESENT 1
#define XPAR_GPIO_8_IS_DUAL 1

/* Canonical definitions for peripheral ZERO_INJECTER_COUNT */
#define XPAR_GPIO_9_BASEADDR 0x401E0000
#define XPAR_GPIO_9_HIGHADDR 0x401EFFFF
#define XPAR_GPIO_9_DEVICE_ID XPAR_ZERO_INJECTER_COUNT_DEVICE_ID
#define XPAR_GPIO_9_INTERRUPT_PRESENT 0
#define XPAR_GPIO_9_IS_DUAL 1

/* Canonical definitions for peripheral ZERO_INJECTER_START */
#define XPAR_GPIO_10_BASEADDR 0x401F0000
#define XPAR_GPIO_10_HIGHADDR 0x401FFFFF
#define XPAR_GPIO_10_DEVICE_ID XPAR_ZERO_INJECTER_START_DEVICE_ID
#define XPAR_GPIO_10_INTERRUPT_PRESENT 0
#define XPAR_GPIO_10_IS_DUAL 0


/******************************************************************/

/* Definitions for driver GPIOPS */
#define XPAR_XGPIOPS_NUM_INSTANCES 1

/* Definitions for peripheral PS7_GPIO_0 */
#define XPAR_PS7_GPIO_0_DEVICE_ID 0
#define XPAR_PS7_GPIO_0_BASEADDR 0xE000A000
#define XPAR_PS7_GPIO_0_HIGHADDR 0xE000AFFF


/******************************************************************/

/* Canonical definitions for peripheral PS7_GPIO_0 */
#define XPAR_XGPIOPS_0_DEVICE_ID XPAR_PS7_GPIO_0_DEVICE_ID
#define XPAR_XGPIOPS_0_BASEADDR 0xE000A000
#define XPAR_XGPIOPS_0_HIGHADDR 0xE000AFFF


/******************************************************************/

/* Definitions for driver IICPS */
#define XPAR_XIICPS_NUM_INSTANCES 1

/* Definitions for peripheral PS7_I2C_0 */
#define XPAR_PS7_I2C_0_DEVICE_ID 0
#define XPAR_PS7_I2C_0_BASEADDR 0xE0004000
#define XPAR_PS7_I2C_0_HIGHADDR 0xE0004FFF
#define XPAR_PS7_I2C_0_I2C_CLK_FREQ_HZ 133333328


/******************************************************************/

/* Canonical definitions for peripheral PS7_I2C_0 */
#define XPAR_XIICPS_0_DEVICE_ID XPAR_PS7_I2C_0_DEVICE_ID
#define XPAR_XIICPS_0_BASEADDR 0xE0004000
#define XPAR_XIICPS_0_HIGHADDR 0xE0004FFF
#define XPAR_XIICPS_0_I2C_CLK_FREQ_HZ 133333328


/******************************************************************/

/* Definition for input Clock */
/* Definitions for driver LLFIFO */
#define XPAR_XLLFIFO_NUM_INSTANCES 2U

/* Definitions for peripheral SERIALIZA_FIFO */
#define XPAR_SERIALIZA_FIFO_DEVICE_ID 0U
#define XPAR_SERIALIZA_FIFO_BASEADDR 0x40050000U
#define XPAR_SERIALIZA_FIFO_HIGHADDR 0x4005FFFFU
#define XPAR_SERIALIZA_FIFO_AXI4_BASEADDR 0U
#define XPAR_SERIALIZA_FIFO_AXI4_HIGHADDR 0U
#define XPAR_SERIALIZA_FIFO_DATA_INTERFACE_TYPE 0U

/* Canonical definitions for peripheral SERIALIZA_FIFO */
#define XPAR_AXI_FIFO_0_DEVICE_ID 0U
#define XPAR_AXI_FIFO_0_BASEADDR 0x40050000U
#define XPAR_AXI_FIFO_0_HIGHADDR 0x4005FFFFU
#define XPAR_AXI_FIFO_0_AXI4_BASEADDR 0U
#define XPAR_AXI_FIFO_0_AXI4_HIGHADDR 0U
#define XPAR_AXI_FIFO_0_DATA_INTERFACE_TYPE 0U



/* Definitions for peripheral AXI_FIFO_MM_S_0 */
#define XPAR_AXI_FIFO_MM_S_0_DEVICE_ID 1U
#define XPAR_AXI_FIFO_MM_S_0_BASEADDR 0x40090000U
#define XPAR_AXI_FIFO_MM_S_0_HIGHADDR 0x4009FFFFU
#define XPAR_AXI_FIFO_MM_S_0_AXI4_BASEADDR 0x90000000U
#define XPAR_AXI_FIFO_MM_S_0_AXI4_HIGHADDR 0x9000FFFFU
#define XPAR_AXI_FIFO_MM_S_0_DATA_INTERFACE_TYPE 1U

/* Canonical definitions for peripheral AXI_FIFO_MM_S_0 */
#define XPAR_AXI_FIFO_1_DEVICE_ID 1U
#define XPAR_AXI_FIFO_1_BASEADDR 0x40090000U
#define XPAR_AXI_FIFO_1_HIGHADDR 0x4009FFFFU
#define XPAR_AXI_FIFO_1_AXI4_BASEADDR 0x90000000U
#define XPAR_AXI_FIFO_1_AXI4_HIGHADDR 0x9000FFFFU
#define XPAR_AXI_FIFO_1_DATA_INTERFACE_TYPE 1U



/******************************************************************/

/* Definitions for driver MIG_7SERIES */
#define XPAR_XMIG7SERIES_NUM_INSTANCES 1U

/* Definitions for peripheral SDRAM */
#define XPAR_SDRAM_DEVICE_ID 0U
#define XPAR_SDRAM_DDR3_ROW_WIDTH 14U
#define XPAR_SDRAM_DDR3_COL_WIDTH 0U
#define XPAR_SDRAM_DDR3_BANK_WIDTH 3U
#define XPAR_SDRAM_DDR3_DQ_WIDTH 16U


/******************************************************************/


/* Definitions for peripheral SDRAM */
#define XPAR_SDRAM_BASEADDR 0x80000000
#define XPAR_SDRAM_HIGHADDR 0x8FFFFFFF


/******************************************************************/

/* Canonical definitions for peripheral SDRAM */
#define XPAR_MIG7SERIES_0_DEVICE_ID XPAR_SDRAM_DEVICE_ID
#define XPAR_MIG7SERIES_0_DDR_ROW_WIDTH 14U
#define XPAR_MIG7SERIES_0_DDR_COL_WIDTH 0U
#define XPAR_MIG7SERIES_0_DDR_BANK_WIDTH 3U
#define XPAR_MIG7SERIES_0_DDR_DQ_WIDTH 16U
#define XPAR_MIG7SERIES_0_BASEADDR 0x80000000U
#define XPAR_MIG7SERIES_0_HIGHADDR 0x8FFFFFFFU


/******************************************************************/


/* Definitions for peripheral MUTEX_0 IF 0 */
#define XPAR_MUTEX_0_IF_0_DEVICE_ID 0U
#define XPAR_MUTEX_0_TESTAPP_ID 0U
#define XPAR_MUTEX_0_IF_0_BASEADDR 0x401A0000U
#define XPAR_MUTEX_0_IF_0_NUM_MUTEX 16U
#define XPAR_MUTEX_0_IF_0_ENABLE_USER 1U

/* Definitions for driver MUTEX */
#define XPAR_XMUTEX_NUM_INSTANCES 1U

/******************************************************************/


/* Canonical definitions for peripheral MUTEX_0 IF 0 */
#define XPAR_MUTEX_0_DEVICE_ID XPAR_MUTEX_0_IF_0_DEVICE_ID
#define XPAR_MUTEX_0_BASEADDR 0x401A0000U
#define XPAR_MUTEX_0_HIGHADDR 0x401AFFFFU
#define XPAR_MUTEX_0_NUM_MUTEX 16U
#define XPAR_MUTEX_0_ENABLE_USER 1U


/******************************************************************/

/* Definitions for driver QSPIPS */
#define XPAR_XQSPIPS_NUM_INSTANCES 1

/* Definitions for peripheral PS7_QSPI_0 */
#define XPAR_PS7_QSPI_0_DEVICE_ID 0
#define XPAR_PS7_QSPI_0_BASEADDR 0xE000D000
#define XPAR_PS7_QSPI_0_HIGHADDR 0xE000DFFF
#define XPAR_PS7_QSPI_0_QSPI_CLK_FREQ_HZ 200000000
#define XPAR_PS7_QSPI_0_QSPI_MODE 0
#define XPAR_PS7_QSPI_0_QSPI_BUS_WIDTH 2


/******************************************************************/

/* Canonical definitions for peripheral PS7_QSPI_0 */
#define XPAR_XQSPIPS_0_DEVICE_ID XPAR_PS7_QSPI_0_DEVICE_ID
#define XPAR_XQSPIPS_0_BASEADDR 0xE000D000
#define XPAR_XQSPIPS_0_HIGHADDR 0xE000DFFF
#define XPAR_XQSPIPS_0_QSPI_CLK_FREQ_HZ 200000000
#define XPAR_XQSPIPS_0_QSPI_MODE 0
#define XPAR_XQSPIPS_0_QSPI_BUS_WIDTH 2


/******************************************************************/

/* Definitions for Fabric interrupts connected to ps7_scugic_0 */
#define XPAR_FABRIC_UCI_AXI_MASTER_INTERRUPT_INTR 61U
#define XPAR_FABRIC_DATAMOVER_CTRL_AXI_LITE_0_INTERRUPT_INTR 62U
#define XPAR_FABRIC_TRIGGER_TIMER_INTERRUPT_INTR 63U
#define XPAR_FABRIC_TRIG_HW_AXILITE_0_TRIG_INT_INTR 64U
#define XPAR_FABRIC_TIMESTAMP_TIMER_INTERRUPT_INTR 65U
#define XPAR_FABRIC_MISCELLANEOUS_GPIO_0_IP2INTC_IRPT_INTR 66U
#define XPAR_FABRIC_AXI_TIMER_0_INTERRUPT_INTR 67U
#define XPAR_FABRIC_AXI_TIMER_1_INTERRUPT_INTR 68U
#define XPAR_FABRIC_AXI_TIMER_2_INTERRUPT_INTR 84U
#define XPAR_FABRIC_SERIALIZA_FIFO_INTERRUPT_INTR 85U
#define XPAR_FABRIC_AXI_FIFO_MM_S_0_INTERRUPT_INTR 86U
#define XPAR_FABRIC_AXI_GPIO_LINK_PRO_IP2INTC_IRPT_INTR 87U
#define XPAR_FABRIC_CTRL_DATAMOVER_DDR_PL_INTERRUPT_INTR 89U

/******************************************************************/

/* Canonical definitions for Fabric interrupts connected to ps7_scugic_0 */
#define XPAR_FABRIC_TMRCTR_4_VEC_ID XPAR_FABRIC_TRIGGER_TIMER_INTERRUPT_INTR
#define XPAR_FABRIC_TMRCTR_3_VEC_ID XPAR_FABRIC_TIMESTAMP_TIMER_INTERRUPT_INTR
#define XPAR_FABRIC_GPIO_8_VEC_ID XPAR_FABRIC_MISCELLANEOUS_GPIO_0_IP2INTC_IRPT_INTR
#define XPAR_FABRIC_TMRCTR_0_VEC_ID XPAR_FABRIC_AXI_TIMER_0_INTERRUPT_INTR
#define XPAR_FABRIC_TMRCTR_1_VEC_ID XPAR_FABRIC_AXI_TIMER_1_INTERRUPT_INTR
#define XPAR_FABRIC_TMRCTR_2_VEC_ID XPAR_FABRIC_AXI_TIMER_2_INTERRUPT_INTR
#define XPAR_FABRIC_LLFIFO_0_VEC_ID XPAR_FABRIC_SERIALIZA_FIFO_INTERRUPT_INTR
#define XPAR_FABRIC_LLFIFO_1_VEC_ID XPAR_FABRIC_AXI_FIFO_MM_S_0_INTERRUPT_INTR
#define XPAR_FABRIC_GPIO_5_VEC_ID XPAR_FABRIC_AXI_GPIO_LINK_PRO_IP2INTC_IRPT_INTR

/******************************************************************/

/* Definitions for driver SCUGIC */
#define XPAR_XSCUGIC_NUM_INSTANCES 1U

/* Definitions for peripheral PS7_SCUGIC_0 */
#define XPAR_PS7_SCUGIC_0_DEVICE_ID 0U
#define XPAR_PS7_SCUGIC_0_BASEADDR 0xF8F00100U
#define XPAR_PS7_SCUGIC_0_HIGHADDR 0xF8F001FFU
#define XPAR_PS7_SCUGIC_0_DIST_BASEADDR 0xF8F01000U


/******************************************************************/

/* Canonical definitions for peripheral PS7_SCUGIC_0 */
#define XPAR_SCUGIC_0_DEVICE_ID 0U
#define XPAR_SCUGIC_0_CPU_BASEADDR 0xF8F00100U
#define XPAR_SCUGIC_0_CPU_HIGHADDR 0xF8F001FFU
#define XPAR_SCUGIC_0_DIST_BASEADDR 0xF8F01000U


/******************************************************************/

/* Definitions for driver SCUTIMER */
#define XPAR_XSCUTIMER_NUM_INSTANCES 1

/* Definitions for peripheral PS7_SCUTIMER_0 */
#define XPAR_PS7_SCUTIMER_0_DEVICE_ID 0
#define XPAR_PS7_SCUTIMER_0_BASEADDR 0xF8F00600
#define XPAR_PS7_SCUTIMER_0_HIGHADDR 0xF8F0061F


/******************************************************************/

/* Canonical definitions for peripheral PS7_SCUTIMER_0 */
#define XPAR_XSCUTIMER_0_DEVICE_ID XPAR_PS7_SCUTIMER_0_DEVICE_ID
#define XPAR_XSCUTIMER_0_BASEADDR 0xF8F00600
#define XPAR_XSCUTIMER_0_HIGHADDR 0xF8F0061F


/******************************************************************/

/* Definitions for driver SCUWDT */
#define XPAR_XSCUWDT_NUM_INSTANCES 1

/* Definitions for peripheral PS7_SCUWDT_0 */
#define XPAR_PS7_SCUWDT_0_DEVICE_ID 0
#define XPAR_PS7_SCUWDT_0_BASEADDR 0xF8F00620
#define XPAR_PS7_SCUWDT_0_HIGHADDR 0xF8F006FF


/******************************************************************/

/* Canonical definitions for peripheral PS7_SCUWDT_0 */
#define XPAR_SCUWDT_0_DEVICE_ID XPAR_PS7_SCUWDT_0_DEVICE_ID
#define XPAR_SCUWDT_0_BASEADDR 0xF8F00620
#define XPAR_SCUWDT_0_HIGHADDR 0xF8F006FF


/******************************************************************/

/* Definitions for driver SDPS */
#define XPAR_XSDPS_NUM_INSTANCES 1

/* Definitions for peripheral PS7_SD_0 */
#define XPAR_PS7_SD_0_DEVICE_ID 0
#define XPAR_PS7_SD_0_BASEADDR 0xE0100000
#define XPAR_PS7_SD_0_HIGHADDR 0xE0100FFF
#define XPAR_PS7_SD_0_SDIO_CLK_FREQ_HZ 50000000
#define XPAR_PS7_SD_0_HAS_CD 1
#define XPAR_PS7_SD_0_HAS_WP 0
#define XPAR_PS7_SD_0_BUS_WIDTH 0
#define XPAR_PS7_SD_0_MIO_BANK 0
#define XPAR_PS7_SD_0_HAS_EMIO 0
#define XPAR_PS7_SD_0_SLOT_TYPE 0
#define XPAR_PS7_SD_0_CLK_50_SDR_ITAP_DLY 0
#define XPAR_PS7_SD_0_CLK_50_SDR_OTAP_DLY 0
#define XPAR_PS7_SD_0_CLK_50_DDR_ITAP_DLY 0
#define XPAR_PS7_SD_0_CLK_50_DDR_OTAP_DLY 0
#define XPAR_PS7_SD_0_CLK_100_SDR_OTAP_DLY 0
#define XPAR_PS7_SD_0_CLK_200_SDR_OTAP_DLY 0


/******************************************************************/

#define XPAR_PS7_SD_0_IS_CACHE_COHERENT 0
/* Canonical definitions for peripheral PS7_SD_0 */
#define XPAR_XSDPS_0_DEVICE_ID XPAR_PS7_SD_0_DEVICE_ID
#define XPAR_XSDPS_0_BASEADDR 0xE0100000
#define XPAR_XSDPS_0_HIGHADDR 0xE0100FFF
#define XPAR_XSDPS_0_SDIO_CLK_FREQ_HZ 50000000
#define XPAR_XSDPS_0_HAS_CD 1
#define XPAR_XSDPS_0_HAS_WP 0
#define XPAR_XSDPS_0_BUS_WIDTH 0
#define XPAR_XSDPS_0_MIO_BANK 0
#define XPAR_XSDPS_0_HAS_EMIO 0
#define XPAR_XSDPS_0_SLOT_TYPE 0
#define XPAR_XSDPS_0_IS_CACHE_COHERENT 0
#define XPAR_XSDPS_0_CLK_50_SDR_ITAP_DLY 0
#define XPAR_XSDPS_0_CLK_50_SDR_OTAP_DLY 0
#define XPAR_XSDPS_0_CLK_50_DDR_ITAP_DLY 0
#define XPAR_XSDPS_0_CLK_50_DDR_OTAP_DLY 0
#define XPAR_XSDPS_0_CLK_100_SDR_OTAP_DLY 0
#define XPAR_XSDPS_0_CLK_200_SDR_OTAP_DLY 0


/******************************************************************/

/* Definitions for driver SYSMON */
#define XPAR_XSYSMON_NUM_INSTANCES 1U

/* Definitions for peripheral XADC_WIZ */
#define XPAR_XADC_WIZ_IP_TYPE 0U
#define XPAR_XADC_WIZ_DEVICE_ID 0U
#define XPAR_XADC_WIZ_BASEADDR 0x90010000U
#define XPAR_XADC_WIZ_HIGHADDR 0x9001FFFFU
#define XPAR_XADC_WIZ_INCLUDE_INTR 1U


/******************************************************************/

/* Canonical definitions for peripheral XADC_WIZ */
#define XPAR_SYSMON_0_IP_TYPE 0U
#define XPAR_SYSMON_0_DEVICE_ID XPAR_XADC_WIZ_DEVICE_ID
#define XPAR_SYSMON_0_BASEADDR 0x90010000U
#define XPAR_SYSMON_0_HIGHADDR 0x9001FFFFU
#define XPAR_SYSMON_0_INCLUDE_INTR 1U


/******************************************************************/

/* Definitions for driver TMRCTR */
#define XPAR_XTMRCTR_NUM_INSTANCES 5U

/* Definitions for peripheral AXI_TIMER_0 */
#define XPAR_AXI_TIMER_0_DEVICE_ID 0U
#define XPAR_AXI_TIMER_0_BASEADDR 0x400C0000U
#define XPAR_AXI_TIMER_0_HIGHADDR 0x400CFFFFU
#define XPAR_AXI_TIMER_0_CLOCK_FREQ_HZ 125000000U


/* Definitions for peripheral AXI_TIMER_1 */
#define XPAR_AXI_TIMER_1_DEVICE_ID 1U
#define XPAR_AXI_TIMER_1_BASEADDR 0x400D0000U
#define XPAR_AXI_TIMER_1_HIGHADDR 0x400DFFFFU
#define XPAR_AXI_TIMER_1_CLOCK_FREQ_HZ 125000000U


/* Definitions for peripheral AXI_TIMER_2 */
#define XPAR_AXI_TIMER_2_DEVICE_ID 2U
#define XPAR_AXI_TIMER_2_BASEADDR 0x400E0000U
#define XPAR_AXI_TIMER_2_HIGHADDR 0x400EFFFFU
#define XPAR_AXI_TIMER_2_CLOCK_FREQ_HZ 125000000U


/* Definitions for peripheral TIMESTAMP_TIMER */
#define XPAR_TIMESTAMP_TIMER_DEVICE_ID 3U
#define XPAR_TIMESTAMP_TIMER_BASEADDR 0x401B0000U
#define XPAR_TIMESTAMP_TIMER_HIGHADDR 0x401BFFFFU
#define XPAR_TIMESTAMP_TIMER_CLOCK_FREQ_HZ 125000000U


/* Definitions for peripheral TRIGGER_TIMER */
#define XPAR_TRIGGER_TIMER_DEVICE_ID 4U
#define XPAR_TRIGGER_TIMER_BASEADDR 0x401D0000U
#define XPAR_TRIGGER_TIMER_HIGHADDR 0x401DFFFFU
#define XPAR_TRIGGER_TIMER_CLOCK_FREQ_HZ 125000000U


/******************************************************************/

/* Canonical definitions for peripheral AXI_TIMER_0 */
#define XPAR_TMRCTR_0_DEVICE_ID 0U
#define XPAR_TMRCTR_0_BASEADDR 0x400C0000U
#define XPAR_TMRCTR_0_HIGHADDR 0x400CFFFFU
#define XPAR_TMRCTR_0_CLOCK_FREQ_HZ XPAR_AXI_TIMER_0_CLOCK_FREQ_HZ
/* Canonical definitions for peripheral AXI_TIMER_1 */
#define XPAR_TMRCTR_1_DEVICE_ID 1U
#define XPAR_TMRCTR_1_BASEADDR 0x400D0000U
#define XPAR_TMRCTR_1_HIGHADDR 0x400DFFFFU
#define XPAR_TMRCTR_1_CLOCK_FREQ_HZ XPAR_AXI_TIMER_1_CLOCK_FREQ_HZ
/* Canonical definitions for peripheral AXI_TIMER_2 */
#define XPAR_TMRCTR_2_DEVICE_ID 2U
#define XPAR_TMRCTR_2_BASEADDR 0x400E0000U
#define XPAR_TMRCTR_2_HIGHADDR 0x400EFFFFU
#define XPAR_TMRCTR_2_CLOCK_FREQ_HZ XPAR_AXI_TIMER_2_CLOCK_FREQ_HZ
/* Canonical definitions for peripheral TIMESTAMP_TIMER */
#define XPAR_TMRCTR_3_DEVICE_ID 3U
#define XPAR_TMRCTR_3_BASEADDR 0x401B0000U
#define XPAR_TMRCTR_3_HIGHADDR 0x401BFFFFU
#define XPAR_TMRCTR_3_CLOCK_FREQ_HZ XPAR_TIMESTAMP_TIMER_CLOCK_FREQ_HZ
/* Canonical definitions for peripheral TRIGGER_TIMER */
#define XPAR_TMRCTR_4_DEVICE_ID 4U
#define XPAR_TMRCTR_4_BASEADDR 0x401D0000U
#define XPAR_TMRCTR_4_HIGHADDR 0x401DFFFFU
#define XPAR_TMRCTR_4_CLOCK_FREQ_HZ XPAR_TRIGGER_TIMER_CLOCK_FREQ_HZ

/******************************************************************/

/* Definitions for driver TTCPS */
#define XPAR_XTTCPS_NUM_INSTANCES 3U

/* Definitions for peripheral PS7_TTC_0 */
#define XPAR_PS7_TTC_0_DEVICE_ID 0U
#define XPAR_PS7_TTC_0_BASEADDR 0XF8001000U
#define XPAR_PS7_TTC_0_TTC_CLK_FREQ_HZ 133333344U
#define XPAR_PS7_TTC_0_TTC_CLK_CLKSRC 0U
#define XPAR_PS7_TTC_1_DEVICE_ID 1U
#define XPAR_PS7_TTC_1_BASEADDR 0XF8001004U
#define XPAR_PS7_TTC_1_TTC_CLK_FREQ_HZ 133333344U
#define XPAR_PS7_TTC_1_TTC_CLK_CLKSRC 0U
#define XPAR_PS7_TTC_2_DEVICE_ID 2U
#define XPAR_PS7_TTC_2_BASEADDR 0XF8001008U
#define XPAR_PS7_TTC_2_TTC_CLK_FREQ_HZ 133333344U
#define XPAR_PS7_TTC_2_TTC_CLK_CLKSRC 0U


/******************************************************************/

/* Canonical definitions for peripheral PS7_TTC_0 */
#define XPAR_XTTCPS_0_DEVICE_ID XPAR_PS7_TTC_0_DEVICE_ID
#define XPAR_XTTCPS_0_BASEADDR 0xF8001000U
#define XPAR_XTTCPS_0_TTC_CLK_FREQ_HZ 133333344U
#define XPAR_XTTCPS_0_TTC_CLK_CLKSRC 0U

#define XPAR_XTTCPS_1_DEVICE_ID XPAR_PS7_TTC_1_DEVICE_ID
#define XPAR_XTTCPS_1_BASEADDR 0xF8001004U
#define XPAR_XTTCPS_1_TTC_CLK_FREQ_HZ 133333344U
#define XPAR_XTTCPS_1_TTC_CLK_CLKSRC 0U

#define XPAR_XTTCPS_2_DEVICE_ID XPAR_PS7_TTC_2_DEVICE_ID
#define XPAR_XTTCPS_2_BASEADDR 0xF8001008U
#define XPAR_XTTCPS_2_TTC_CLK_FREQ_HZ 133333344U
#define XPAR_XTTCPS_2_TTC_CLK_CLKSRC 0U


/******************************************************************/

/* Definitions for driver UARTPS */
#define XPAR_XUARTPS_NUM_INSTANCES 1

/* Definitions for peripheral PS7_UART_0 */
#define XPAR_PS7_UART_0_DEVICE_ID 0
#define XPAR_PS7_UART_0_BASEADDR 0xE0000000
#define XPAR_PS7_UART_0_HIGHADDR 0xE0000FFF
#define XPAR_PS7_UART_0_UART_CLK_FREQ_HZ 100000000
#define XPAR_PS7_UART_0_HAS_MODEM 0


/******************************************************************/

/* Canonical definitions for peripheral PS7_UART_0 */
#define XPAR_XUARTPS_0_DEVICE_ID XPAR_PS7_UART_0_DEVICE_ID
#define XPAR_XUARTPS_0_BASEADDR 0xE0000000
#define XPAR_XUARTPS_0_HIGHADDR 0xE0000FFF
#define XPAR_XUARTPS_0_UART_CLK_FREQ_HZ 100000000
#define XPAR_XUARTPS_0_HAS_MODEM 0


/******************************************************************/

/* Definition for input Clock */
/* Definitions for driver USBPS */
#define XPAR_XUSBPS_NUM_INSTANCES 1

/* Definitions for peripheral PS7_USB_0 */
#define XPAR_PS7_USB_0_DEVICE_ID 0
#define XPAR_PS7_USB_0_BASEADDR 0xE0002000
#define XPAR_PS7_USB_0_HIGHADDR 0xE0002FFF


/******************************************************************/

/* Canonical definitions for peripheral PS7_USB_0 */
#define XPAR_XUSBPS_0_DEVICE_ID XPAR_PS7_USB_0_DEVICE_ID
#define XPAR_XUSBPS_0_BASEADDR 0xE0002000
#define XPAR_XUSBPS_0_HIGHADDR 0xE0002FFF


/******************************************************************/

/* Definitions for driver WDTPS */
#define XPAR_XWDTPS_NUM_INSTANCES 1

/* Definitions for peripheral PS7_WDT_0 */
#define XPAR_PS7_WDT_0_DEVICE_ID 0
#define XPAR_PS7_WDT_0_BASEADDR 0xF8005000
#define XPAR_PS7_WDT_0_HIGHADDR 0xF8005FFF
#define XPAR_PS7_WDT_0_WDT_CLK_FREQ_HZ 133333344


/******************************************************************/

/* Canonical definitions for peripheral PS7_WDT_0 */
#define XPAR_XWDTPS_0_DEVICE_ID XPAR_PS7_WDT_0_DEVICE_ID
#define XPAR_XWDTPS_0_BASEADDR 0xF8005000
#define XPAR_XWDTPS_0_HIGHADDR 0xF8005FFF
#define XPAR_XWDTPS_0_WDT_CLK_FREQ_HZ 133333344


/******************************************************************/

/* Definitions for driver XADCPS */
#define XPAR_XADCPS_NUM_INSTANCES 1

/* Definitions for peripheral PS7_XADC_0 */
#define XPAR_PS7_XADC_0_DEVICE_ID 0
#define XPAR_PS7_XADC_0_BASEADDR 0xF8007100
#define XPAR_PS7_XADC_0_HIGHADDR 0xF8007120


/******************************************************************/

/* Canonical definitions for peripheral PS7_XADC_0 */
#define XPAR_XADCPS_0_DEVICE_ID XPAR_PS7_XADC_0_DEVICE_ID
#define XPAR_XADCPS_0_BASEADDR 0xF8007100
#define XPAR_XADCPS_0_HIGHADDR 0xF8007120


/******************************************************************/

#endif  /* end of protection macro */
