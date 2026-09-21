/*
 * bcc_bussar.h
 *
 *  Created on: 27 mar. 2019
 *      Author: Jorge
 */

#ifndef SRC_SITAU2_BCC_BUSSAR_BCC_BUSSAR_H_
#define SRC_SITAU2_BCC_BUSSAR_BCC_BUSSAR_H_

#include "xil_types.h"
#include "xparameters.h"
#include <math.h>
#define TEST_FLAG_PRO_RISE 1
//AHORA MISMO NO FUNCIONA!
#define SEND_BUFFER_NO_DMA	1
#define MAX_LVDS_CONF_BUFFER_DATA		0x1FFFFF

#define MAX_COMMAND_BUFFER_DATA (512*1024)
typedef struct command_buffer_s
{
	//__align(8) u32 data[MAX_COMMAND_BUFFER_DATA];
	u32 *data;
	int total_len;
	int max_len;
}commnd_buffer_t;

typedef union bcc_ctrl_bits_u
{
	u8 CTRL_BYTE;
	struct
	{
		u8 SPC	:	1;	//SPeCial register access
		u8 NCK	:	1;	//Not aCKnowledge
		u8 ACK	:	1;	//ACKnowledge
		u8 WR	:	1;	//WRite
		u8 RD	:	1;	//ReaD
		u8 RS1	:	1;	//ReServed 1
		u8 RS2	:	1;	//ReServed 2
		u8 ROS	:	1;	//Register=0 Or Stream=1
	}BITS;

}bcc_ctrl_bits_t;
typedef union spc_u
{
	u16 SPC_WORD;
	struct
	{
		u16 SPC_VAL	:	13;	//SPeCial register access
		u16 SPC_DIR	:	3;	//Not aCKnowledge
	}BITS;
}spc_t;

typedef union bcc_header_u
{
	u32 HEADER;
	struct
	{
		u8 REGIST;
		u8 SUBMOD;
		u8 MODULE;
		bcc_ctrl_bits_t  CTRL;
	}BUSSAR_CMD;
	struct
	{
		u16 LENGHT;
		u8 MODULE;
		bcc_ctrl_bits_t  CTRL;
	}STREAM_CMD;
	struct
	{
		spc_t SPC_WORD;
		u8 MODULE;
		bcc_ctrl_bits_t  CTRL;
	}SPECIAL_CMD;
}bcc_header_t;

#define SPCREG_MOD_NUMBER_REG		0
#define SPCREG_WORD_COUNT_EXT_REG	1
#define SPCREG_FIFO_DELAY_REG		2
#define SPCREG_PRO_REG				3
#define SPCREG_DELAY_PER_MODULE_REG	4
#define SPCREG_RESERVED				5
#define SPCREG_AUTO_DELAY_REG		6
#define SPCREG_AUTO_NUMBER_REG		7

#define MISC_PS_CONTROL_UDC_MASK 	0x10

#define MISC_PRO_LINK_GTX_MASK				0x1
#define MISC_PRO_LINK_LVDS_MASK				0x2
#define MISC_PRO_LINK_VALUE_MASK			0x4
#define MISC_PRO_LINK_DISABLE_TRIG_SEC_MASK	0x8

#define MISC_ADDR_CHAIN_16_FALSE_32_TRUE_MASK	0x1



#define GPIO_CONF_ADDR						(XPAR_PROG_GPIO_BASEADDR)
#define GPIO_CONF_LVDS_M1_MASK					0x01
#define GPIO_CONF_LVDS_PROGB_MASK				0x02
#define GPIO_CONF_LVDS_DSS_MASK					0x04
#define GPIO_CONF_LVDS_SCLK_MASK				0x08
#define GPIO_CONF_LVDS_DESKEW_START_MASK		0x10
#define GPIO_CONF_LVDS_BITSLIP_START_MASK		0x20
#define GPIO_CONF_LVDS_UNLOCK_DESKEW_CONF_MASK	0x40
#define GPIO_CONF_LVDS_WORDALIGN_MASK			0x80

#define GPIO_CONF_INPUT_ADDR			 	(XPAR_PROG_GPIO_BASEADDR+ 0x08)
#define GPIO_CONF_LVDS_DESKEW_PAT				0x01
#define GPIO_CONF_LVDS_BITSLIP_PAT				0x02
#define GPIO_CONF_LVDS_CONF_DONE_MASK			0x04 // Equivale a bitslip done tambien
#define GPIO_CONF_LVDS_WORDALIGN_PAT			0x08


void bcc_print_all_busy_flags(u8 last_minibase);
void init_bcc_and_bussar(void);
u32 bcc_get_busy_in_word(u8 minibase);
u32 bcc_get_busy_or_word(u8 minibase);
u32 bcc_get_pro_mask_word(u8 minibase);
u32 bcc_set_pro_mask_word(u8 minibase,u32 pro_mask_word);
u32 bcc_set_busy_or_word(u8 minibase,u32 not_busy_word);
u32 bcc_set_pro_and_busy_mask_buffer(u8 minibase,u32 pro_mask_word,u32 busy_mask,commnd_buffer_t *commnd_buf_p);
int bcc_copy_uci2mem(u32* datos,u32 n_datos,u32 remote_addr,u8 minibase,commnd_buffer_t *commnd_buf_p);
int BCC_read_reg(u8 module,u8 submod,u8 reg,u32 *data_read);
u16 BCC_read_PRO(u8 module);
void BCC_write_PRO(u16 pro_val, u8 module);
int BCC_wait_flag(u32 waiting_flags,float us_timeout);
int push_bussar_write_reg(u32 data,u8 module,u8 submodule,u8 reg,commnd_buffer_t *commnd_buf);
int push_bussar_u32(u32 data_to_push,commnd_buffer_t *commnd_buf);
int BCC_SendBuffer(commnd_buffer_t *commnd_buf);
void BCC_write_reg_inmediate(u8 module,u8 submod,u8 reg,u32 data);
int BCC_wait_PRO_rise(float us_timeout);
int BCC_wait_PRO_fall(float us_timeout);
int BCC_deskew_LVDS(float us_timeout);
int BCC_test_LVDS_configuration(void);
int BCC_compare_buffers(u32 *Sent_data, u32 *Rec_data, u32 len);
int BCC_set_dynamic_phase(int minibase, int delay_ps, float us_timeout);
int BCC_test_TGC_module();
int BCC_test_DDR_remote(void);

extern commnd_buffer_t Global_commnd_buf;


#endif /* SRC_SITAU2_BCC_BUSSAR_BCC_BUSSAR_H_ */
