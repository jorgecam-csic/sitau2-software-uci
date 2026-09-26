#include "network.h"

void transfer_data(void);
void platform_enable_interrupts();
void start_network_aplications();

void tcp_fasttmr(void);
void tcp_slowtmr(void);

#if LWIP_DHCP==1
extern volatile int dhcp_timoutcntr;
err_t dhcp_start(struct netif *netif);
#endif
extern volatile int TxPerfConnMonCntr;
extern volatile int TcpFastTmrFlag;
extern volatile int TcpSlowTmrFlag;

void lwip_init(void);	// Parece que hay que declararla...

u8 fpga_ip[4] = {0,0,0,0};
u8 netmask[4] = {0,0,0,0};
u8 gateway[4] = {0,0,0,0};

struct netif Net_interface;


void print_ip(char *msg, ip4_addr_t *ip)
{
    print(msg);
    xil_printf("%d.%d.%d.%d\r\n", ip4_addr1(ip), ip4_addr2(ip),ip4_addr3(ip), ip4_addr4(ip));
}

void print_ip_settings(ip4_addr_t *ip, ip4_addr_t *mask, ip4_addr_t *gw)
{
    print_ip("Board IP:       ", ip);
    print_ip("Netmask :       ", mask);
    print_ip("Gateway :       ", gw);
}

void transfer_data(void)
{

    if (INCLUDE_IMAGE_SERVER)
    	transfer_tcpimage_data();
    if (INCLUDE_TCPCOM_SERVER)
        transfer_tcpcom_data();


}

void start_network_applications()
{
	if (INCLUDE_TCPCOM_SERVER)
		start_tcpcom_application();
	if (INCLUDE_IMAGE_SERVER)
		start_tcpimage_application();
}
void print_network_applications_headers(void)
{
	if (INCLUDE_TCPCOM_SERVER)
		print_tcpcom_app_header();
	if (INCLUDE_IMAGE_SERVER)
		print_tcpimage_app_header();
}

void network_loop()
{
	if (TcpFastTmrFlag)
	{
		tcp_fasttmr();
		TcpFastTmrFlag = 0;
	}
	if (TcpSlowTmrFlag)
	{
		tcp_slowtmr();
		TcpSlowTmrFlag = 0;
	}

	xemacif_input(&Net_interface);
	transfer_data();
}

int init_network()
{
	struct netif *netif;
	ip4_addr_t ipaddr, netmsk, gw;

	int i;


	/* the mac address of the board. this should be unique per board */
	unsigned char mac_ethernet_address[6];

	init_network_parameters();

	for(i=0;i<6;i++)
		mac_ethernet_address[i]=Net_param.mac_addr[i];

	netif = &Net_interface;

	xil_printf("\r\n\r\n");
	xil_printf("----- Initializing network ------\r\n");
	/* initialize IP addresses to be used */
#if (LWIP_DHCP==0)
	IP4_ADDR(&ipaddr, Net_param.ip_addr[0],Net_param.ip_addr[1],Net_param.ip_addr[2],Net_param.ip_addr[3]);
	IP4_ADDR(&netmsk, Net_param.mask[0],Net_param.mask[1],Net_param.mask[2],Net_param.mask[3]);
	IP4_ADDR(&gw,     Net_param.gateway[0],Net_param.gateway[1],Net_param.gateway[2],Net_param.gateway[3]);
    print_ip_settings(&ipaddr, &netmsk, &gw);
#endif
	lwip_init();

#if (LWIP_DHCP==1)
	ipaddr.addr = 0;
	gw.addr = 0;
	netmsk.addr = 0;
#endif

	/* Add network interface to the netif_list, and set it as default */
	if (!xemac_add(netif, &ipaddr, &netmsk, &gw, mac_ethernet_address, PLATFORM_EMAC_BASEADDR))
	{
		xil_printf("Error adding N/W interface\r\n");
		return -1;
	}
	netif_set_default(netif);

	/* specify that the network if is up */
	netif_set_up(netif);

	/* now enable interrupts */
	platform_enable_interrupts();

#if (LWIP_DHCP==1)
	/* Create a new DHCP client for this interface.
	 * Note: you must call dhcp_fine_tmr() and dhcp_coarse_tmr() at
	 * the predefined regular intervals after starting the client.
	 */
	if(Net_param.options&DHCP_OPTION_MASK)
	{
		xil_printf("Performing DHCP...\n\r");
		dhcp_start(netif);
		dhcp_timoutcntr = 24;
		TxPerfConnMonCntr = 0;
		while(((netif->ip_addr.addr) == 0) && (dhcp_timoutcntr > 0)) {
			xemacif_input(netif);
			if (TcpFastTmrFlag) {
				tcp_fasttmr();
				TcpFastTmrFlag = 0;
			}
			if (TcpSlowTmrFlag) {
				tcp_slowtmr();
				TcpSlowTmrFlag = 0;
			}
		}

		if (dhcp_timoutcntr <= 0)
		{
			if ((netif->ip_addr.addr) == 0) {
				xil_printf("DHCP Timeout, loading default\r\n");
				IP4_ADDR(&ipaddr, Net_param.ip_addr[0],Net_param.ip_addr[1],Net_param.ip_addr[2],Net_param.ip_addr[3]);
				IP4_ADDR(&netmsk, Net_param.mask[0],Net_param.mask[1],Net_param.mask[2],Net_param.mask[3]);
				IP4_ADDR(&gw,     Net_param.gateway[0],Net_param.gateway[1],Net_param.gateway[2],Net_param.gateway[3]);
			}
			memcpy((void*)&(netif->ip_addr),(void*)&ipaddr,4);
			memcpy((void*)&(netif->gw),(void*)&gw,4);
			memcpy((void*)&(netif->netmask),(void*)&netmsk,4);
		}
	}
	else
	{
		xil_printf("DHCP disabled, loading default\r\n");
		IP4_ADDR(&ipaddr, Net_param.ip_addr[0],Net_param.ip_addr[1],Net_param.ip_addr[2],Net_param.ip_addr[3]);
		IP4_ADDR(&netmsk, Net_param.mask[0],Net_param.mask[1],Net_param.mask[2],Net_param.mask[3]);
		IP4_ADDR(&gw,     Net_param.gateway[0],Net_param.gateway[1],Net_param.gateway[2],Net_param.gateway[3]);
		memcpy((void*)&(netif->ip_addr),(void*)&ipaddr,4);
		memcpy((void*)&(netif->gw),(void*)&gw,4);
		memcpy((void*)&(netif->netmask),(void*)&netmsk,4);
	}
	print_ip_settings(&(netif->ip_addr), &(netif->netmask), &(netif->gw));
#endif

	memcpy((void*)&Net_param.ip_addr,(void*)&(netif->ip_addr),4);
	memcpy((void*)&Net_param.gateway,(void*)&(netif->gw),4);
	memcpy((void*)&Net_param.mask,(void*)&(netif->netmask),4);

	/* start the application (web server, rxtest, txtest, etc..) */
	start_network_applications();
	print_network_applications_headers();

	return 0;
}

#define XEmacPs_WriteReg_m(BaseAddress, RegOffset, Data) \
    XEmacPs_Out32((BaseAddress) + (u32)(RegOffset), (u32)(Data))

#define XEmacPs_ReadReg_m(BaseAddress, RegOffset) \
    XEmacPs_In32((BaseAddress) + (u32)(RegOffset))

LONG XEmacPs_PhyRead_bestia(u32* baseaddress, u32 PhyAddress,
		     u32 RegisterNum, u16 *PhyDataPtr)
{
	u32 Mgtcr;
	volatile u32 Ipisr;
	u32 IpReadTemp;
	LONG Status;

	/* Make sure no other PHY operation is currently in progress */
	if ((!(XEmacPs_ReadReg_m(baseaddress,
				XEMACPS_NWSR_OFFSET) &
	      XEMACPS_NWSR_MDIOIDLE_MASK))==TRUE) {
		Status = (LONG)(XST_EMAC_MII_BUSY);
	}
	//else
	{

	/* Construct Mgtcr mask for the operation */
	Mgtcr = XEMACPS_PHYMNTNC_OP_MASK | XEMACPS_PHYMNTNC_OP_R_MASK |
			(PhyAddress << XEMACPS_PHYMNTNC_PHAD_SHFT_MSK) |
			(RegisterNum << XEMACPS_PHYMNTNC_PREG_SHFT_MSK);

	/* Write Mgtcr and wait for completion */
	XEmacPs_WriteReg_m(baseaddress,
			   XEMACPS_PHYMNTNC_OFFSET, Mgtcr);

	do {
		Ipisr = XEmacPs_ReadReg_m(baseaddress,
					  XEMACPS_NWSR_OFFSET);
			IpReadTemp = Ipisr;
		} while ((IpReadTemp & XEMACPS_NWSR_MDIOIDLE_MASK) == 0x00000000U);

	/* Read data */
		*PhyDataPtr = (u16)XEmacPs_ReadReg_m(baseaddress,
					XEMACPS_PHYMNTNC_OFFSET);
		Status = (LONG)(XST_SUCCESS);
	}
	return Status;
}

#define IEEE_CONTROL_REG_OFFSET_MODCHAP                    0
#define IEEE_STATUS_REG_OFFSET_MODCHAP                     1
#define IEEE_PARTNER_ABILITIES_1_REG_OFFSET_MODCHAP        5
#define IEEE_PARTNER_ABILITIES_2_REG_OFFSET_MODCHAP        8
#define IEEE_PARTNER_ABILITIES_3_REG_OFFSET_MODCHAP        10
#define IEEE_1000_ADVERTISE_REG_OFFSET_MODCHAP             9
#define IEEE_STAT_1GBPS_EXTENSIONS_MODCHAP                 0x0100
#define IEEE_AN3_ABILITY_MASK_1GBPS_MODCHAP                0x0C00
#define IEEE_AN1_ABILITY_MASK_100MBPS_MODCHAP              0x0380
#define IEEE_AN1_ABILITY_MASK_10MBPS_MODCHAP               0x0060


get_phy_speed_mod(u32* baseaddress)
{
	u32_t phy_addr=3;
	u16 status,partner_capabilities,partner_capabilities_1000;
	XEmacPs_PhyRead_bestia(baseaddress, phy_addr, IEEE_STATUS_REG_OFFSET_MODCHAP, &status);

	XEmacPs_PhyRead_bestia(baseaddress, phy_addr, IEEE_PARTNER_ABILITIES_1_REG_OFFSET_MODCHAP, &partner_capabilities);

	XEmacPs_PhyRead_bestia(baseaddress, phy_addr, IEEE_PARTNER_ABILITIES_3_REG_OFFSET_MODCHAP, &partner_capabilities_1000);

	//if (status & IEEE_STAT_1GBPS_EXTENSIONS_MODCHAP)
	{
		XEmacPs_PhyRead_bestia(baseaddress, phy_addr, IEEE_PARTNER_ABILITIES_3_REG_OFFSET_MODCHAP, &partner_capabilities_1000);
		if (partner_capabilities_1000 & IEEE_AN3_ABILITY_MASK_1GBPS_MODCHAP){
			return 1000;
		}
	}

	if (partner_capabilities & IEEE_AN1_ABILITY_MASK_100MBPS_MODCHAP){
		return 100;
	}
	if (partner_capabilities & IEEE_AN1_ABILITY_MASK_10MBPS_MODCHAP)
	{
		return 10;
	}
}

u16 monitor_ethernet_link()
{
    u32* PUNTERO_BUENO_XEmacPs=0xe000b000;
	return get_phy_speed_mod(PUNTERO_BUENO_XEmacPs);
 }


