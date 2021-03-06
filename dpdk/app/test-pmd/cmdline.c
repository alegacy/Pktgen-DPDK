/*-
 *   BSD LICENSE
 * 
 *   Copyright(c) 2010-2012 Intel Corporation. All rights reserved.
 *   All rights reserved.
 * 
 *   Redistribution and use in source and binary forms, with or without 
 *   modification, are permitted provided that the following conditions 
 *   are met:
 * 
 *     * Redistributions of source code must retain the above copyright 
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright 
 *       notice, this list of conditions and the following disclaimer in 
 *       the documentation and/or other materials provided with the 
 *       distribution.
 *     * Neither the name of Intel Corporation nor the names of its 
 *       contributors may be used to endorse or promote products derived 
 *       from this software without specific prior written permission.
 * 
 *   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
 *   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
 *   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR 
 *   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
 *   OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
 *   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
 *   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
 *   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY 
 *   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
 *   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
 *   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 */

#include <stdarg.h>
#include <errno.h>
#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <inttypes.h>
#ifndef __linux__
#include <net/socket.h>
#endif
#include <netinet/in.h>

#include <sys/queue.h>

#include <rte_common.h>
#include <rte_byteorder.h>
#include <rte_log.h>
#include <rte_debug.h>
#include <rte_cycles.h>
#include <rte_memory.h>
#include <rte_memzone.h>
#include <rte_launch.h>
#include <rte_tailq.h>
#include <rte_eal.h>
#include <rte_per_lcore.h>
#include <rte_lcore.h>
#include <rte_atomic.h>
#include <rte_branch_prediction.h>
#include <rte_ring.h>
#include <rte_mempool.h>
#include <rte_interrupts.h>
#include <rte_pci.h>
#include <rte_ether.h>
#include <rte_ethdev.h>
#include <rte_string_fns.h>

#include <cmdline_rdline.h>
#include <cmdline_parse.h>
#include <cmdline_parse_num.h>
#include <cmdline_parse_string.h>
#include <cmdline_parse_ipaddr.h>
#include <cmdline_parse_etheraddr.h>
#include <cmdline_socket.h>
#include <cmdline.h>

#include "testpmd.h"

static void cmd_reconfig_device_queue(portid_t id, uint8_t dev, uint8_t queue);

/* *** HELP *** */
struct cmd_help_result {
	cmdline_fixed_string_t help;
};

static void cmd_help_parsed(__attribute__((unused)) void *parsed_result,
			    struct cmdline *cl,
			    __attribute__((unused)) void *data)
{
	cmdline_printf(cl,
		       "\n"
		       "TEST PMD\n"
		       "--------\n"
		       "\n"
		       "This commandline can be used to configure forwarding\n"
		       "\n");
	cmdline_printf(cl,
		       "Display informations:\n"
		       "---------------------\n"
		       "- show port info|stats|fdir|stat_qmap X|all\n"
		       "    Diplays information or stats or stats queue mapping on port X, or all\n"
		       "- clear port stats X|all\n"
		       "    Clear stats for port X, or all\n"
		       "- show config rxtx|cores|fwd\n"
		       "    Displays the given configuration\n"
		       "- read reg port_id reg_off\n"
		       "    Displays value of a port register\n"
		       "- read regfield port_id reg_off bit_x bit_y\n"
		       "    Displays value of a port register bit field\n"
		       "- read regbit port_id reg_off bit_x\n"
		       "    Displays value of a port register bit\n"
		       "- read rxd port_id queue_id rxd_id\n"
		       "    Displays a RX descriptor of a port RX queue\n"
		       "- read txd port_id queue_id txd_id\n"
		       "    Displays a TX descriptor of a port TX queue\n"
		       "\n");
	cmdline_printf(cl,
		       "Configure:\n"
		       "----------\n"
		       "Modifications are taken into account once "
		       "forwarding is restarted.\n"
		       "- set default\n"
		       "    Set forwarding to default configuration\n"
		       "- set nbport|nbcore|burst|verbose X\n"
		       "    Set number of ports, number of cores, number "
		       "of packets per burst,\n    or verbose level to X\n"
		       "- set txpkts x[,y]*\n"
		       "    Set the length of each segment of TXONLY packets\n"
		       "- set coremask|portmask X\n"
		       "    Set the hexadecimal mask of forwarding cores / "
		       "forwarding ports\n"
		       "- set corelist|portlist x[,y]*\n"
		       "    Set the list of forwarding cores / forwarding "
		       "ports\n"
		       "- vlan set strip|filter|qinq on/off port_id\n"
		       "    Set the VLAN strip, filter, QinQ(extended) on a port"
		       "- rx_vlan add/rm vlan_id|all port_id\n"
		       "    Set the VLAN filter table, add/remove vlan_id, or all "
		       "identifiers, to/from the set of VLAN Identifiers\n"
		       "filtered by port_id\n"
		       "- rx_vlan set tpid value port_id\n"
		       "    Set Outer VLAN TPID for Packet Filtering on a port \n"
		       "- tx_vlan set vlan_id port_id\n"
		       "    Set hardware insertion of VLAN ID in packets sent on a port\n"
		       "- tx_vlan reset port_id\n"
		       "    Disable hardware insertion of a VLAN header in "
		       "packets sent on port_id\n"
		       "- tx_checksum set mask port_id\n"
		       "    Enable hardware insertion of checksum offload with "
		       "the 4-bit mask (0~0xf)\n    in packets sent on port_id\n"
		       "    Please check the NIC datasheet for HW limits\n"
		       "      bit 0 - insert ip checksum offload if set \n"
		       "      bit 1 - insert udp checksum offload if set \n"
		       "      bit 2 - insert tcp checksum offload if set\n"
		       "      bit 3 - insert sctp checksum offload if set\n"
#ifdef RTE_LIBRTE_IEEE1588
		       "- set fwd io|mac|rxonly|txonly|csum|ieee1588\n"
		       "    Set IO, MAC, RXONLY, TXONLY, CSUM or IEEE1588 "
		       "packet forwarding mode\n"
#else
		       "- set fwd io|mac|rxonly|txonly|csum\n"
		       "    Set IO, MAC, RXONLY, CSUM or TXONLY packet "
		       "forwarding mode\n"
#endif
		       "- mac_addr add|remove X <xx:xx:xx:xx:xx:xx>\n"
		       "    Add/Remove the MAC address <xx:xx:xx:xx:xx:xx> on port X\n"
		       "- set promisc|allmulti [all|X] on|off\n"
		       "    Set/unset promisc|allmulti mode on port X, or all\n"
		       "- set flow_ctrl rx on|off tx on|off high_water low_water "
						"pause_time send_xon port_id \n"
		       "    Set the link flow control parameter on the port \n"
		       "- set pfc_ctrl rx on|off tx on|off high_water low_water "
						"pause_time priority port_id \n"
		       "    Set the priority flow control parameter on the port \n"
		       "- write reg port_id reg_off value\n"
		       "    Set value of a port register\n"
		       "- write regfield port_id reg_off bit_x bit_y value\n"
		       "    Set bit field value of a port register\n"
		       "- write regbit port_id reg_off bit_x value\n"
		       "    Set bit value of a port register\n"
		       "- set stat_qmap tx|rx port_id queue_id qmapping\n"
		       "    Set statistics mapping (qmapping 0..15) for tx|rx queue_id on port_id\n"
		       "    e.g., 'set stat_qmap rx 0 2 5' sets rx queue 2 on port 0 to mapping 5\n"
		       "\n");
	cmdline_printf(cl,
		       "Control forwarding:\n"
		       "-------------------\n"
		       "- start\n"
		       "    Start packet forwarding with current config\n"
		       "- start tx_first\n"
		       "    Start packet forwarding with current config"
		       " after sending one burst\n    of packets\n"
		       "- stop\n"
		       "    Stop packet forwarding, and displays accumulated"
		       " stats\n"
		       "\n");
	cmdline_printf(cl,
		       "Flow director mode:\n"
		       "-------------------\n"
		       "- add_signature_filter port_id ip|udp|tcp|sctp src\n"
		       "    ip_src_address port_src dst ip_dst_address port_dst\n"
		       "    flexbytes flexbytes_values vlan vlan_id queue queue_id\n"
		       "- upd_signature_filter port_id ip|udp|tcp|sctp src \n"
		       "    ip_src_address port_src dst ip_dst_address port_dst\n"
		       "    flexbytes flexbytes_values vlan vlan_id queue queue_id\n"
		       "- rm_signature_filter port_id ip|udp|tcp|sctp src\n"
		       "    ip_src_address port_src dst ip_dst_address port_dst\n"
		       "    flexbytes flexbytes_values vlan vlan_id\n"
		       "- add_perfect_filter port_id ip|udp|tcp|sctp src\n"
		       "    ip_src_address port_src dst ip_dst_address port_dst\n"
		       "    flexbytes flexbytes_values vlan vlan_id queue \n"
		       "    queue_id soft soft_id\n"
		       "- upd_perfect_filter port_id ip|udp|tcp|sctp src\n"
		       "    ip_src_address port_src dst ip_dst_address port_dst\n"
		       "    flexbytes flexbytes_values vlan vlan_id queue queue_id\n"
		       "- rm_perfect_filter port_id ip|udp|tcp|sctp src\n"
		       "    ip_src_address port_src dst ip_dst_address port_dst\n"
		       "    flexbytes flexbytes_values vlan vlan_id soft soft_id\n"
		       "- set_masks_filter port_id only_ip_flow 0|1 src_mask\n"
		       "    ip_src_mask port_src_mask dst_mask ip_dst_mask\n"
		       "    port_dst_mask flexbytes 0|1 vlan_id 0|1 vlan_prio 0|1\n"
		       "- set_ipv6_masks_filter port_id only_ip_flow 0|1 src_mask\n"
		       "    ip_src_mask port_src_mask dst_mask ip_dst_mask\n"
		       "    port_dst_mask flexbytes 0|1 vlan_id 0|1\n"
		       "    vlan_prio 0|1 compare_dst 0|1\n"
		       "\n");
	cmdline_printf(cl,
		       "Port Operations:\n"
		       "--------------\n"
		       "- port start|stop|close all|X\n"
		       "    start/stop/close all ports or port X\n"
		       "- port config all|X speed 10|100|1000|10000|auto "
		       "duplex half|full|auto\n"
		       "    set speed for all ports or port X\n"
		       "- port config all rxq|txq|rxd|txd value\n"
		       "    set number for rxq/txq/rxd/txd\n"
		       "- port config all max-pkt-len value\n"
		       "    set the max packet lenght\n"
		       "- port config all crc-strip|rx-cksum|hw-vlan|drop-en on|off\n"
		       "    set crc-strip/rx-checksum/hardware-vlan/drop_en on or off"
		       "\n"
		       "- port config all rss ip|udp|none\n"
		       "    set rss mode\n"
		       "- port config port-id dcb vt on|off nb-tcs pfc on|off\n"
		       "    set dcb mode\n"
		       "- port config all burst value\n"
		       "    set the number of packet per burst\n"
		       "- port config all txpt|txht|txwt|rxpt|rxht|rxwt value\n"
		       "    set ring prefetch/host/writeback threshold for "
		       "tx/rx queue\n"
		       "- port config all txfreet|txrst|rxfreet value\n"
		       "    set free threshold for rx/tx, or set tx rs bit "
		       "threshold\n"
		       "\n");
	cmdline_printf(cl,
		       "Misc:\n"
		       "-----\n"
		       "- quit\n"
		       "    Quit to prompt in linux, and reboot on baremetal\n"
		       "\n");
}

cmdline_parse_token_string_t cmd_help_help =
	TOKEN_STRING_INITIALIZER(struct cmd_help_result, help, "help");

cmdline_parse_inst_t cmd_help = {
	.f = cmd_help_parsed,
	.data = NULL,
	.help_str = "show help",
	.tokens = {
		(void *)&cmd_help_help,
		NULL,
	},
};

/* *** start/stop/close all ports *** */
struct cmd_operate_port_result {
	cmdline_fixed_string_t keyword;
	cmdline_fixed_string_t name;
	cmdline_fixed_string_t value;
};

static void cmd_operate_port_parsed(void *parsed_result,
				__attribute__((unused)) struct cmdline *cl,
				__attribute__((unused)) void *data)
{
	struct cmd_operate_port_result *res = parsed_result;

	if (!strcmp(res->name, "start"))
		start_port(RTE_PORT_ALL);
	else if (!strcmp(res->name, "stop"))
		stop_port(RTE_PORT_ALL);
	else if (!strcmp(res->name, "close"))
		close_port(RTE_PORT_ALL);
	else
		printf("Unknown parameter\n");
}

cmdline_parse_token_string_t cmd_operate_port_all_cmd =
	TOKEN_STRING_INITIALIZER(struct cmd_operate_port_result, keyword,
								"port");
cmdline_parse_token_string_t cmd_operate_port_all_port =
	TOKEN_STRING_INITIALIZER(struct cmd_operate_port_result, name,
						"start#stop#close");
cmdline_parse_token_string_t cmd_operate_port_all_all =
	TOKEN_STRING_INITIALIZER(struct cmd_operate_port_result, value, "all");

cmdline_parse_inst_t cmd_operate_port = {
	.f = cmd_operate_port_parsed,
	.data = NULL,
	.help_str = "port start|stop|close all: start/stop/close all ports",
	.tokens = {
		(void *)&cmd_operate_port_all_cmd,
		(void *)&cmd_operate_port_all_port,
		(void *)&cmd_operate_port_all_all,
		NULL,
	},
};

/* *** start/stop/close specific port *** */
struct cmd_operate_specific_port_result {
	cmdline_fixed_string_t keyword;
	cmdline_fixed_string_t name;
	uint8_t value;
};

static void cmd_operate_specific_port_parsed(void *parsed_result,
			__attribute__((unused)) struct cmdline *cl,
				__attribute__((unused)) void *data)
{
	struct cmd_operate_specific_port_result *res = parsed_result;

	if (!strcmp(res->name, "start"))
		start_port(res->value);
	else if (!strcmp(res->name, "stop"))
		stop_port(res->value);
	else if (!strcmp(res->name, "close"))
		close_port(res->value);
	else
		printf("Unknown parameter\n");
}

cmdline_parse_token_string_t cmd_operate_specific_port_cmd =
	TOKEN_STRING_INITIALIZER(struct cmd_operate_specific_port_result,
							keyword, "port");
cmdline_parse_token_string_t cmd_operate_specific_port_port =
	TOKEN_STRING_INITIALIZER(struct cmd_operate_specific_port_result,
						name, "start#stop#close");
cmdline_parse_token_num_t cmd_operate_specific_port_id =
	TOKEN_NUM_INITIALIZER(struct cmd_operate_specific_port_result,
							value, UINT8);

cmdline_parse_inst_t cmd_operate_specific_port = {
	.f = cmd_operate_specific_port_parsed,
	.data = NULL,
	.help_str = "port start|stop|close X: start/stop/close port X",
	.tokens = {
		(void *)&cmd_operate_specific_port_cmd,
		(void *)&cmd_operate_specific_port_port,
		(void *)&cmd_operate_specific_port_id,
		NULL,
	},
};

/* *** configure speed for all ports *** */
struct cmd_config_speed_all {
	cmdline_fixed_string_t port;
	cmdline_fixed_string_t keyword;
	cmdline_fixed_string_t all;
	cmdline_fixed_string_t item1;
	cmdline_fixed_string_t item2;
	cmdline_fixed_string_t value1;
	cmdline_fixed_string_t value2;
};

static void
cmd_config_speed_all_parsed(void *parsed_result,
			__attribute__((unused)) struct cmdline *cl,
			__attribute__((unused)) void *data)
{
	struct cmd_config_speed_all *res = parsed_result;
	uint16_t link_speed = ETH_LINK_SPEED_AUTONEG;
	uint16_t link_duplex = 0;
	portid_t pid;

	if (!all_ports_stopped()) {
		printf("Please stop all ports first\n");
		return;
	}

	if (!strcmp(res->value1, "10"))
		link_speed = ETH_LINK_SPEED_10;
	else if (!strcmp(res->value1, "100"))
		link_speed = ETH_LINK_SPEED_100;
	else if (!strcmp(res->value1, "1000"))
		link_speed = ETH_LINK_SPEED_1000;
	else if (!strcmp(res->value1, "10000"))
		link_speed = ETH_LINK_SPEED_10000;
	else if (!strcmp(res->value1, "auto"))
		link_speed = ETH_LINK_SPEED_AUTONEG;
	else {
		printf("Unknown parameter\n");
		return;
	}

	if (!strcmp(res->value2, "half"))
		link_duplex = ETH_LINK_HALF_DUPLEX;
	else if (!strcmp(res->value2, "full"))
		link_duplex = ETH_LINK_FULL_DUPLEX;
	else if (!strcmp(res->value2, "auto"))
		link_duplex = ETH_LINK_AUTONEG_DUPLEX;
	else {
		printf("Unknown parameter\n");
		return;
	}

	for (pid = 0; pid < nb_ports; pid++) {
		ports[pid].dev_conf.link_speed = link_speed;
		ports[pid].dev_conf.link_duplex = link_duplex;
	}

	cmd_reconfig_device_queue(RTE_PORT_ALL, 1, 1);
}

cmdline_parse_token_string_t cmd_config_speed_all_port =
	TOKEN_STRING_INITIALIZER(struct cmd_config_speed_all, port, "port");
cmdline_parse_token_string_t cmd_config_speed_all_keyword =
	TOKEN_STRING_INITIALIZER(struct cmd_config_speed_all, keyword,
							"config");
cmdline_parse_token_string_t cmd_config_speed_all_all =
	TOKEN_STRING_INITIALIZER(struct cmd_config_speed_all, all, "all");
cmdline_parse_token_string_t cmd_config_speed_all_item1 =
	TOKEN_STRING_INITIALIZER(struct cmd_config_speed_all, item1, "speed");
cmdline_parse_token_string_t cmd_config_speed_all_value1 =
	TOKEN_STRING_INITIALIZER(struct cmd_config_speed_all, value1,
						"10#100#1000#10000#auto");
cmdline_parse_token_string_t cmd_config_speed_all_item2 =
	TOKEN_STRING_INITIALIZER(struct cmd_config_speed_all, item2, "duplex");
cmdline_parse_token_string_t cmd_config_speed_all_value2 =
	TOKEN_STRING_INITIALIZER(struct cmd_config_speed_all, value2,
						"half#full#auto");

cmdline_parse_inst_t cmd_config_speed_all = {
	.f = cmd_config_speed_all_parsed,
	.data = NULL,
	.help_str = "port config all speed 10|100|1000|10000|auto duplex "
							"half|full|auto",
	.tokens = {
		(void *)&cmd_config_speed_all_port,
		(void *)&cmd_config_speed_all_keyword,
		(void *)&cmd_config_speed_all_all,
		(void *)&cmd_config_speed_all_item1,
		(void *)&cmd_config_speed_all_value1,
		(void *)&cmd_config_speed_all_item2,
		(void *)&cmd_config_speed_all_value2,
		NULL,
	},
};

/* *** configure speed for specific port *** */
struct cmd_config_speed_specific {
	cmdline_fixed_string_t port;
	cmdline_fixed_string_t keyword;
	uint8_t id;
	cmdline_fixed_string_t item1;
	cmdline_fixed_string_t item2;
	cmdline_fixed_string_t value1;
	cmdline_fixed_string_t value2;
};

static void
cmd_config_speed_specific_parsed(void *parsed_result,
				__attribute__((unused)) struct cmdline *cl,
				__attribute__((unused)) void *data)
{
	struct cmd_config_speed_specific *res = parsed_result;
	uint16_t link_speed = ETH_LINK_SPEED_AUTONEG;
	uint16_t link_duplex = 0;

	if (!all_ports_stopped()) {
		printf("Please stop all ports first\n");
		return;
	}

	if (res->id >= nb_ports) {
		printf("Port id %d must be less than %d\n", res->id, nb_ports);
		return;
	}

	if (!strcmp(res->value1, "10"))
		link_speed = ETH_LINK_SPEED_10;
	else if (!strcmp(res->value1, "100"))
		link_speed = ETH_LINK_SPEED_100;
	else if (!strcmp(res->value1, "1000"))
		link_speed = ETH_LINK_SPEED_1000;
	else if (!strcmp(res->value1, "10000"))
		link_speed = ETH_LINK_SPEED_10000;
	else if (!strcmp(res->value1, "auto"))
		link_speed = ETH_LINK_SPEED_AUTONEG;
	else {
		printf("Unknown parameter\n");
		return;
	}

	if (!strcmp(res->value2, "half"))
		link_duplex = ETH_LINK_HALF_DUPLEX;
	else if (!strcmp(res->value2, "full"))
		link_duplex = ETH_LINK_FULL_DUPLEX;
	else if (!strcmp(res->value2, "auto"))
		link_duplex = ETH_LINK_AUTONEG_DUPLEX;
	else {
		printf("Unknown parameter\n");
		return;
	}

	ports[res->id].dev_conf.link_speed = link_speed;
	ports[res->id].dev_conf.link_duplex = link_duplex;

	cmd_reconfig_device_queue(RTE_PORT_ALL, 1, 1);
}


cmdline_parse_token_string_t cmd_config_speed_specific_port =
	TOKEN_STRING_INITIALIZER(struct cmd_config_speed_specific, port,
								"port");
cmdline_parse_token_string_t cmd_config_speed_specific_keyword =
	TOKEN_STRING_INITIALIZER(struct cmd_config_speed_specific, keyword,
								"config");
cmdline_parse_token_num_t cmd_config_speed_specific_id =
	TOKEN_NUM_INITIALIZER(struct cmd_config_speed_specific, id, UINT8);
cmdline_parse_token_string_t cmd_config_speed_specific_item1 =
	TOKEN_STRING_INITIALIZER(struct cmd_config_speed_specific, item1,
								"speed");
cmdline_parse_token_string_t cmd_config_speed_specific_value1 =
	TOKEN_STRING_INITIALIZER(struct cmd_config_speed_specific, value1,
						"10#100#1000#10000#auto");
cmdline_parse_token_string_t cmd_config_speed_specific_item2 =
	TOKEN_STRING_INITIALIZER(struct cmd_config_speed_specific, item2,
								"duplex");
cmdline_parse_token_string_t cmd_config_speed_specific_value2 =
	TOKEN_STRING_INITIALIZER(struct cmd_config_speed_specific, value2,
							"half#full#auto");

cmdline_parse_inst_t cmd_config_speed_specific = {
	.f = cmd_config_speed_specific_parsed,
	.data = NULL,
	.help_str = "port config X speed 10|100|1000|10000|auto duplex "
							"half|full|auto",
	.tokens = {
		(void *)&cmd_config_speed_specific_port,
		(void *)&cmd_config_speed_specific_keyword,
		(void *)&cmd_config_speed_specific_id,
		(void *)&cmd_config_speed_specific_item1,
		(void *)&cmd_config_speed_specific_value1,
		(void *)&cmd_config_speed_specific_item2,
		(void *)&cmd_config_speed_specific_value2,
		NULL,
	},
};

/* *** configure txq/rxq, txd/rxd *** */
struct cmd_config_rx_tx {
	cmdline_fixed_string_t port;
	cmdline_fixed_string_t keyword;
	cmdline_fixed_string_t all;
	cmdline_fixed_string_t name;
	uint16_t value;
};

static void
cmd_config_rx_tx_parsed(void *parsed_result,
			__attribute__((unused)) struct cmdline *cl,
			__attribute__((unused)) void *data)
{
	struct cmd_config_rx_tx *res = parsed_result;

	if (!all_ports_stopped()) {
		printf("Please stop all ports first\n");
		return;
	}

	if (!strcmp(res->name, "rxq")) {
		if (res->value <= 0) {
			printf("rxq %d invalid - must be > 0\n", res->value);
			return;
		}
		nb_rxq = res->value;
	}
	else if (!strcmp(res->name, "txq")) {
		if (res->value <= 0) {
			printf("txq %d invalid - must be > 0\n", res->value);
			return;
		}
		nb_txq = res->value;
	}
	else if (!strcmp(res->name, "rxd")) {
		if (res->value <= 0 || res->value > RTE_TEST_RX_DESC_MAX) {
			printf("rxd %d invalid - must be > 0 && <= %d\n",
					res->value, RTE_TEST_RX_DESC_MAX);
			return;
		}
		nb_rxd = res->value;
	} else if (!strcmp(res->name, "txd")) {
		if (res->value <= 0 || res->value > RTE_TEST_TX_DESC_MAX) {
			printf("txd %d invalid - must be > 0 && <= %d\n",
					res->value, RTE_TEST_TX_DESC_MAX);
			return;
		}
		nb_txd = res->value;
	} else {
		printf("Unknown parameter\n");
		return;
	}

	init_port_config();

	cmd_reconfig_device_queue(RTE_PORT_ALL, 1, 1);
}

cmdline_parse_token_string_t cmd_config_rx_tx_port =
	TOKEN_STRING_INITIALIZER(struct cmd_config_rx_tx, port, "port");
cmdline_parse_token_string_t cmd_config_rx_tx_keyword =
	TOKEN_STRING_INITIALIZER(struct cmd_config_rx_tx, keyword, "config");
cmdline_parse_token_string_t cmd_config_rx_tx_all =
	TOKEN_STRING_INITIALIZER(struct cmd_config_rx_tx, all, "all");
cmdline_parse_token_string_t cmd_config_rx_tx_name =
	TOKEN_STRING_INITIALIZER(struct cmd_config_rx_tx, name,
						"rxq#txq#rxd#txd");
cmdline_parse_token_num_t cmd_config_rx_tx_value =
	TOKEN_NUM_INITIALIZER(struct cmd_config_rx_tx, value, UINT16);

cmdline_parse_inst_t cmd_config_rx_tx = {
	.f = cmd_config_rx_tx_parsed,
	.data = NULL,
	.help_str = "port config all rxq|txq|rxd|txd value",
	.tokens = {
		(void *)&cmd_config_rx_tx_port,
		(void *)&cmd_config_rx_tx_keyword,
		(void *)&cmd_config_rx_tx_all,
		(void *)&cmd_config_rx_tx_name,
		(void *)&cmd_config_rx_tx_value,
		NULL,
	},
};

/* *** config max packet length *** */
struct cmd_config_max_pkt_len_result {
	cmdline_fixed_string_t port;
	cmdline_fixed_string_t keyword;
	cmdline_fixed_string_t all;
	cmdline_fixed_string_t name;
	uint32_t value;
};

static void
cmd_config_max_pkt_len_parsed(void *parsed_result,
				__attribute__((unused)) struct cmdline *cl,
				__attribute__((unused)) void *data)
{
	struct cmd_config_max_pkt_len_result *res = parsed_result;

	if (!all_ports_stopped()) {
		printf("Please stop all ports first\n");
		return;
	}

	if (!strcmp(res->name, "max-pkt-len")) {
		if (res->value < ETHER_MIN_LEN) {
			printf("max-pkt-len can not be less than %d\n",
							ETHER_MIN_LEN);
			return;
		}
		if (res->value == rx_mode.max_rx_pkt_len)
			return;

		rx_mode.max_rx_pkt_len = res->value;
		if (res->value > ETHER_MAX_LEN)
			rx_mode.jumbo_frame = 1;
		else
			rx_mode.jumbo_frame = 0;
	} else {
		printf("Unknown parameter\n");
		return;
	}

	init_port_config();

	cmd_reconfig_device_queue(RTE_PORT_ALL, 1, 1);
}

cmdline_parse_token_string_t cmd_config_max_pkt_len_port =
	TOKEN_STRING_INITIALIZER(struct cmd_config_max_pkt_len_result, port,
								"port");
cmdline_parse_token_string_t cmd_config_max_pkt_len_keyword =
	TOKEN_STRING_INITIALIZER(struct cmd_config_max_pkt_len_result, keyword,
								"config");
cmdline_parse_token_string_t cmd_config_max_pkt_len_all =
	TOKEN_STRING_INITIALIZER(struct cmd_config_max_pkt_len_result, all,
								"all");
cmdline_parse_token_string_t cmd_config_max_pkt_len_name =
	TOKEN_STRING_INITIALIZER(struct cmd_config_max_pkt_len_result, name,
								"max-pkt-len");
cmdline_parse_token_num_t cmd_config_max_pkt_len_value =
	TOKEN_NUM_INITIALIZER(struct cmd_config_max_pkt_len_result, value,
								UINT32);

cmdline_parse_inst_t cmd_config_max_pkt_len = {
	.f = cmd_config_max_pkt_len_parsed,
	.data = NULL,
	.help_str = "port config all max-pkt-len value",
	.tokens = {
		(void *)&cmd_config_max_pkt_len_port,
		(void *)&cmd_config_max_pkt_len_keyword,
		(void *)&cmd_config_max_pkt_len_all,
		(void *)&cmd_config_max_pkt_len_name,
		(void *)&cmd_config_max_pkt_len_value,
		NULL,
	},
};

/* *** configure rx mode *** */
struct cmd_config_rx_mode_flag {
	cmdline_fixed_string_t port;
	cmdline_fixed_string_t keyword;
	cmdline_fixed_string_t all;
	cmdline_fixed_string_t name;
	cmdline_fixed_string_t value;
};

static void
cmd_config_rx_mode_flag_parsed(void *parsed_result,
				__attribute__((unused)) struct cmdline *cl,
				__attribute__((unused)) void *data)
{
	struct cmd_config_rx_mode_flag *res = parsed_result;

	if (!all_ports_stopped()) {
		printf("Please stop all ports first\n");
		return;
	}

	if (!strcmp(res->name, "crc-strip")) {
		if (!strcmp(res->value, "on"))
			rx_mode.hw_strip_crc = 1;
		else if (!strcmp(res->value, "off"))
			rx_mode.hw_strip_crc = 0;
		else {
			printf("Unknown parameter\n");
			return;
		}
	} else if (!strcmp(res->name, "rx-cksum")) {
		if (!strcmp(res->value, "on"))
			rx_mode.hw_ip_checksum = 1;
		else if (!strcmp(res->value, "off"))
			rx_mode.hw_ip_checksum = 0;
		else {
			printf("Unknown parameter\n");
			return;
		}
	} else if (!strcmp(res->name, "hw-vlan")) {
		if (!strcmp(res->value, "on")) {
			rx_mode.hw_vlan_filter = 1;
			rx_mode.hw_vlan_strip  = 1;
		}
		else if (!strcmp(res->value, "off")) {
			rx_mode.hw_vlan_filter = 0;
			rx_mode.hw_vlan_strip  = 0;
		}
		else {
			printf("Unknown parameter\n");
			return;
		}
	} else if (!strcmp(res->name, "drop-en")) {
		if (!strcmp(res->value, "on"))
			rx_drop_en = 1;
		else if (!strcmp(res->value, "off"))
			rx_drop_en = 0;
		else {
			printf("Unknown parameter\n");
			return;
		}
	} else {
		printf("Unknown parameter\n");
		return;
	}

	init_port_config();

	cmd_reconfig_device_queue(RTE_PORT_ALL, 1, 1);
}

cmdline_parse_token_string_t cmd_config_rx_mode_flag_port =
	TOKEN_STRING_INITIALIZER(struct cmd_config_rx_mode_flag, port, "port");
cmdline_parse_token_string_t cmd_config_rx_mode_flag_keyword =
	TOKEN_STRING_INITIALIZER(struct cmd_config_rx_mode_flag, keyword,
								"config");
cmdline_parse_token_string_t cmd_config_rx_mode_flag_all =
	TOKEN_STRING_INITIALIZER(struct cmd_config_rx_mode_flag, all, "all");
cmdline_parse_token_string_t cmd_config_rx_mode_flag_name =
	TOKEN_STRING_INITIALIZER(struct cmd_config_rx_mode_flag, name,
					"crc-strip#rx-cksum#hw-vlan");
cmdline_parse_token_string_t cmd_config_rx_mode_flag_value =
	TOKEN_STRING_INITIALIZER(struct cmd_config_rx_mode_flag, value,
							"on#off");

cmdline_parse_inst_t cmd_config_rx_mode_flag = {
	.f = cmd_config_rx_mode_flag_parsed,
	.data = NULL,
	.help_str = "port config all crc-strip|rx-cksum|hw-vlan on|off",
	.tokens = {
		(void *)&cmd_config_rx_mode_flag_port,
		(void *)&cmd_config_rx_mode_flag_keyword,
		(void *)&cmd_config_rx_mode_flag_all,
		(void *)&cmd_config_rx_mode_flag_name,
		(void *)&cmd_config_rx_mode_flag_value,
		NULL,
	},
};

/* *** configure rss *** */
struct cmd_config_rss {
	cmdline_fixed_string_t port;
	cmdline_fixed_string_t keyword;
	cmdline_fixed_string_t all;
	cmdline_fixed_string_t name;
	cmdline_fixed_string_t value;
};

static void
cmd_config_rss_parsed(void *parsed_result,
			__attribute__((unused)) struct cmdline *cl,
			__attribute__((unused)) void *data)
{
	struct cmd_config_rss *res = parsed_result;

	if (!all_ports_stopped()) {
		printf("Please stop all ports first\n");
		return;
	}

	if (!strcmp(res->value, "ip"))
		rss_hf = ETH_RSS_IPV4 | ETH_RSS_IPV6;
	else if (!strcmp(res->value, "udp"))
		rss_hf = ETH_RSS_IPV4 | ETH_RSS_IPV6 | ETH_RSS_IPV4_UDP;
	else if (!strcmp(res->value, "none"))
		rss_hf = 0;
	else {
		printf("Unknown parameter\n");
		return;
	}

	init_port_config();

	cmd_reconfig_device_queue(RTE_PORT_ALL, 1, 1);
}

cmdline_parse_token_string_t cmd_config_rss_port =
	TOKEN_STRING_INITIALIZER(struct cmd_config_rss, port, "port");
cmdline_parse_token_string_t cmd_config_rss_keyword =
	TOKEN_STRING_INITIALIZER(struct cmd_config_rss, keyword, "config");
cmdline_parse_token_string_t cmd_config_rss_all =
	TOKEN_STRING_INITIALIZER(struct cmd_config_rss, all, "all");
cmdline_parse_token_string_t cmd_config_rss_name =
	TOKEN_STRING_INITIALIZER(struct cmd_config_rss, name, "rss");
cmdline_parse_token_string_t cmd_config_rss_value =
	TOKEN_STRING_INITIALIZER(struct cmd_config_rss, value, "ip#udp#none");

cmdline_parse_inst_t cmd_config_rss = {
	.f = cmd_config_rss_parsed,
	.data = NULL,
	.help_str = "port config all rss ip|udp|none",
	.tokens = {
		(void *)&cmd_config_rss_port,
		(void *)&cmd_config_rss_keyword,
		(void *)&cmd_config_rss_all,
		(void *)&cmd_config_rss_name,
		(void *)&cmd_config_rss_value,
		NULL,
	},
};

/* *** Configure DCB *** */
struct cmd_config_dcb {
	cmdline_fixed_string_t port;
	cmdline_fixed_string_t config;
	uint8_t port_id; 
	cmdline_fixed_string_t dcb;
	cmdline_fixed_string_t vt;
	cmdline_fixed_string_t vt_en;
	uint8_t num_tcs; 
	cmdline_fixed_string_t pfc;
	cmdline_fixed_string_t pfc_en;
};

static void
cmd_config_dcb_parsed(void *parsed_result,
                        __attribute__((unused)) struct cmdline *cl,
                        __attribute__((unused)) void *data)
{
	struct cmd_config_dcb *res = parsed_result;
	struct dcb_config dcb_conf;
	portid_t port_id = res->port_id;
	struct rte_port *port;
	
	port = &ports[port_id];
	/** Check if the port is not started **/
	if (port->port_status != RTE_PORT_STOPPED) {
		printf("Please stop port %d first\n",port_id);
		return;
	}
		
	dcb_conf.num_tcs = (enum rte_eth_nb_tcs) res->num_tcs;
	if ((dcb_conf.num_tcs != ETH_4_TCS) && (dcb_conf.num_tcs != ETH_8_TCS)){
		printf("The invalid number of traffic class,only 4 or 8 allowed\n");
		return;
	}

	/* DCB in VT mode */
	if (!strncmp(res->vt_en, "on",2)) 
		dcb_conf.dcb_mode = DCB_VT_ENABLED;	
	else
		dcb_conf.dcb_mode = DCB_ENABLED;

	if (!strncmp(res->pfc_en, "on",2)) {
		dcb_conf.pfc_en = 1;
	}
	else
		dcb_conf.pfc_en = 0;

	if (init_port_dcb_config(port_id,&dcb_conf) != 0) {
		printf("Cannot initialize network ports\n");
		return;
	}

	cmd_reconfig_device_queue(port_id, 1, 1);
}
 
cmdline_parse_token_string_t cmd_config_dcb_port =
        TOKEN_STRING_INITIALIZER(struct cmd_config_dcb, port, "port");
cmdline_parse_token_string_t cmd_config_dcb_config =
        TOKEN_STRING_INITIALIZER(struct cmd_config_dcb, config, "config");
cmdline_parse_token_num_t cmd_config_dcb_port_id =
        TOKEN_NUM_INITIALIZER(struct cmd_config_dcb, port_id, UINT8);
cmdline_parse_token_string_t cmd_config_dcb_dcb =
        TOKEN_STRING_INITIALIZER(struct cmd_config_dcb, dcb, "dcb");
cmdline_parse_token_string_t cmd_config_dcb_vt =
        TOKEN_STRING_INITIALIZER(struct cmd_config_dcb, vt, "vt");
cmdline_parse_token_string_t cmd_config_dcb_vt_en =
        TOKEN_STRING_INITIALIZER(struct cmd_config_dcb, vt_en, "on#off");
cmdline_parse_token_num_t cmd_config_dcb_num_tcs =
        TOKEN_NUM_INITIALIZER(struct cmd_config_dcb, num_tcs, UINT8);
cmdline_parse_token_string_t cmd_config_dcb_pfc=
        TOKEN_STRING_INITIALIZER(struct cmd_config_dcb, pfc, "pfc");
cmdline_parse_token_string_t cmd_config_dcb_pfc_en =
        TOKEN_STRING_INITIALIZER(struct cmd_config_dcb, pfc_en, "on#off");

cmdline_parse_inst_t cmd_config_dcb = {
        .f = cmd_config_dcb_parsed,
        .data = NULL,
        .help_str = "port config port-id dcb vt on|off nb-tcs pfc on|off",
        .tokens = {
		(void *)&cmd_config_dcb_port,
		(void *)&cmd_config_dcb_config,
		(void *)&cmd_config_dcb_port_id,
		(void *)&cmd_config_dcb_dcb,
		(void *)&cmd_config_dcb_vt,
		(void *)&cmd_config_dcb_vt_en,
		(void *)&cmd_config_dcb_num_tcs,
		(void *)&cmd_config_dcb_pfc,
		(void *)&cmd_config_dcb_pfc_en,
                NULL,
        },
};

/* *** configure number of packets per burst *** */
struct cmd_config_burst {
	cmdline_fixed_string_t port;
	cmdline_fixed_string_t keyword;
	cmdline_fixed_string_t all;
	cmdline_fixed_string_t name;
	uint16_t value;
};

static void
cmd_config_burst_parsed(void *parsed_result,
			__attribute__((unused)) struct cmdline *cl,
			__attribute__((unused)) void *data)
{
	struct cmd_config_burst *res = parsed_result;

	if (!all_ports_stopped()) {
		printf("Please stop all ports first\n");
		return;
	}

	if (!strcmp(res->name, "burst")) {
		if (res->value < 1 || res->value > MAX_PKT_BURST) {
			printf("burst must be >= 1 && <= %d\n", MAX_PKT_BURST);
			return;
		}
		nb_pkt_per_burst = res->value;
	} else {
		printf("Unknown parameter\n");
		return;
	}

	init_port_config();

	cmd_reconfig_device_queue(RTE_PORT_ALL, 1, 1);
}

cmdline_parse_token_string_t cmd_config_burst_port =
	TOKEN_STRING_INITIALIZER(struct cmd_config_burst, port, "port");
cmdline_parse_token_string_t cmd_config_burst_keyword =
	TOKEN_STRING_INITIALIZER(struct cmd_config_burst, keyword, "config");
cmdline_parse_token_string_t cmd_config_burst_all =
	TOKEN_STRING_INITIALIZER(struct cmd_config_burst, all, "all");
cmdline_parse_token_string_t cmd_config_burst_name =
	TOKEN_STRING_INITIALIZER(struct cmd_config_burst, name, "burst");
cmdline_parse_token_num_t cmd_config_burst_value =
	TOKEN_NUM_INITIALIZER(struct cmd_config_burst, value, UINT16);

cmdline_parse_inst_t cmd_config_burst = {
	.f = cmd_config_burst_parsed,
	.data = NULL,
	.help_str = "port config all burst value",
	.tokens = {
		(void *)&cmd_config_burst_port,
		(void *)&cmd_config_burst_keyword,
		(void *)&cmd_config_burst_all,
		(void *)&cmd_config_burst_name,
		(void *)&cmd_config_burst_value,
		NULL,
	},
};

/* *** configure rx/tx queues *** */
struct cmd_config_thresh {
	cmdline_fixed_string_t port;
	cmdline_fixed_string_t keyword;
	cmdline_fixed_string_t all;
	cmdline_fixed_string_t name;
	uint8_t value;
};

static void
cmd_config_thresh_parsed(void *parsed_result,
			__attribute__((unused)) struct cmdline *cl,
			__attribute__((unused)) void *data)
{
	struct cmd_config_thresh *res = parsed_result;

	if (!all_ports_stopped()) {
		printf("Please stop all ports first\n");
		return;
	}

	if (!strcmp(res->name, "txpt"))
		tx_thresh.pthresh = res->value;
	else if(!strcmp(res->name, "txht"))
		tx_thresh.hthresh = res->value;
	else if(!strcmp(res->name, "txwt"))
		tx_thresh.wthresh = res->value;
	else if(!strcmp(res->name, "rxpt"))
		rx_thresh.pthresh = res->value;
	else if(!strcmp(res->name, "rxht"))
		rx_thresh.hthresh = res->value;
	else if(!strcmp(res->name, "rxwt"))
		rx_thresh.wthresh = res->value;
	else {
		printf("Unknown parameter\n");
		return;
	}

	init_port_config();

	cmd_reconfig_device_queue(RTE_PORT_ALL, 1, 1);
}

cmdline_parse_token_string_t cmd_config_thresh_port =
	TOKEN_STRING_INITIALIZER(struct cmd_config_thresh, port, "port");
cmdline_parse_token_string_t cmd_config_thresh_keyword =
	TOKEN_STRING_INITIALIZER(struct cmd_config_thresh, keyword, "config");
cmdline_parse_token_string_t cmd_config_thresh_all =
	TOKEN_STRING_INITIALIZER(struct cmd_config_thresh, all, "all");
cmdline_parse_token_string_t cmd_config_thresh_name =
	TOKEN_STRING_INITIALIZER(struct cmd_config_thresh, name,
				"txpt#txht#txwt#rxpt#rxht#rxwt");
cmdline_parse_token_num_t cmd_config_thresh_value =
	TOKEN_NUM_INITIALIZER(struct cmd_config_thresh, value, UINT8);

cmdline_parse_inst_t cmd_config_thresh = {
	.f = cmd_config_thresh_parsed,
	.data = NULL,
	.help_str = "port config all txpt|txht|txwt|rxpt|rxht|rxwt value",
	.tokens = {
		(void *)&cmd_config_thresh_port,
		(void *)&cmd_config_thresh_keyword,
		(void *)&cmd_config_thresh_all,
		(void *)&cmd_config_thresh_name,
		(void *)&cmd_config_thresh_value,
		NULL,
	},
};

/* *** configure free/rs threshold *** */
struct cmd_config_threshold {
	cmdline_fixed_string_t port;
	cmdline_fixed_string_t keyword;
	cmdline_fixed_string_t all;
	cmdline_fixed_string_t name;
	uint16_t value;
};

static void
cmd_config_threshold_parsed(void *parsed_result,
			__attribute__((unused)) struct cmdline *cl,
			__attribute__((unused)) void *data)
{
	struct cmd_config_threshold *res = parsed_result;

	if (!all_ports_stopped()) {
		printf("Please stop all ports first\n");
		return;
	}

	if (!strcmp(res->name, "txfreet"))
		tx_free_thresh = res->value;
	else if (!strcmp(res->name, "txrst"))
		tx_rs_thresh = res->value;
	else if (!strcmp(res->name, "rxfreet"))
		rx_free_thresh = res->value;
	else {
		printf("Unknown parameter\n");
		return;
	}

	init_port_config();

	cmd_reconfig_device_queue(RTE_PORT_ALL, 1, 1);
}

cmdline_parse_token_string_t cmd_config_threshold_port =
	TOKEN_STRING_INITIALIZER(struct cmd_config_threshold, port, "port");
cmdline_parse_token_string_t cmd_config_threshold_keyword =
	TOKEN_STRING_INITIALIZER(struct cmd_config_threshold, keyword,
								"config");
cmdline_parse_token_string_t cmd_config_threshold_all =
	TOKEN_STRING_INITIALIZER(struct cmd_config_threshold, all, "all");
cmdline_parse_token_string_t cmd_config_threshold_name =
	TOKEN_STRING_INITIALIZER(struct cmd_config_threshold, name,
						"txfreet#txrst#rxfreet");
cmdline_parse_token_num_t cmd_config_threshold_value =
	TOKEN_NUM_INITIALIZER(struct cmd_config_threshold, value, UINT16);

cmdline_parse_inst_t cmd_config_threshold = {
	.f = cmd_config_threshold_parsed,
	.data = NULL,
	.help_str = "port config all txfreet|txrst|rxfreet value",
	.tokens = {
		(void *)&cmd_config_threshold_port,
		(void *)&cmd_config_threshold_keyword,
		(void *)&cmd_config_threshold_all,
		(void *)&cmd_config_threshold_name,
		(void *)&cmd_config_threshold_value,
		NULL,
	},
};

/* *** stop *** */
struct cmd_stop_result {
	cmdline_fixed_string_t stop;
};

static void cmd_stop_parsed(__attribute__((unused)) void *parsed_result,
			    __attribute__((unused)) struct cmdline *cl,
			    __attribute__((unused)) void *data)
{
	stop_packet_forwarding();
}

cmdline_parse_token_string_t cmd_stop_stop =
	TOKEN_STRING_INITIALIZER(struct cmd_stop_result, stop, "stop");

cmdline_parse_inst_t cmd_stop = {
	.f = cmd_stop_parsed,
	.data = NULL,
	.help_str = "stop - stop packet forwarding",
	.tokens = {
		(void *)&cmd_stop_stop,
		NULL,
	},
};

/* *** SET CORELIST and PORTLIST CONFIGURATION *** */

static unsigned int
parse_item_list(char* str, const char* item_name, unsigned int max_items,
		unsigned int *parsed_items, int check_unique_values)
{
	unsigned int nb_item;
	unsigned int value;
	unsigned int i;
	unsigned int j;
	int value_ok;
	char c;

	/*
	 * First parse all items in the list and store their value.
	 */
	value = 0;
	nb_item = 0;
	value_ok = 0;
	for (i = 0; i < strnlen(str, STR_TOKEN_SIZE); i++) {
		c = str[i];
		if ((c >= '0') && (c <= '9')) {
			value = (unsigned int) (value * 10 + (c - '0'));
			value_ok = 1;
			continue;
		}
		if (c != ',') {
			printf("character %c is not a decimal digit\n", c);
			return (0);
		}
		if (! value_ok) {
			printf("No valid value before comma\n");
			return (0);
		}
		if (nb_item < max_items) {
			parsed_items[nb_item] = value;
			value_ok = 0;
			value = 0;
		}
		nb_item++;
	}
	if (nb_item >= max_items) {
		printf("Number of %s = %u > %u (maximum items)\n",
		       item_name, nb_item + 1, max_items);
		return (0);
	}
	parsed_items[nb_item++] = value;
	if (! check_unique_values)
		return (nb_item);

	/*
	 * Then, check that all values in the list are differents.
	 * No optimization here...
	 */
	for (i = 0; i < nb_item; i++) {
		for (j = i + 1; j < nb_item; j++) {
			if (parsed_items[j] == parsed_items[i]) {
				printf("duplicated %s %u at index %u and %u\n",
				       item_name, parsed_items[i], i, j);
				return (0);
			}
		}
	}
	return (nb_item);
}

struct cmd_set_list_result {
	cmdline_fixed_string_t cmd_keyword;
	cmdline_fixed_string_t list_name;
	cmdline_fixed_string_t list_of_items;
};

static void cmd_set_list_parsed(void *parsed_result,
				__attribute__((unused)) struct cmdline *cl,
				__attribute__((unused)) void *data)
{
	struct cmd_set_list_result *res;
	union {
		unsigned int lcorelist[RTE_MAX_LCORE];
		unsigned int portlist[RTE_MAX_ETHPORTS];
	} parsed_items;
	unsigned int nb_item;

	res = parsed_result;
	if (!strcmp(res->list_name, "corelist")) {
		nb_item = parse_item_list(res->list_of_items, "core",
					  RTE_MAX_LCORE,
					  parsed_items.lcorelist, 1);
		if (nb_item > 0)
			set_fwd_lcores_list(parsed_items.lcorelist, nb_item);
		return;
	}
	if (!strcmp(res->list_name, "portlist")) {
		nb_item = parse_item_list(res->list_of_items, "port",
					  RTE_MAX_ETHPORTS,
					  parsed_items.portlist, 1);
		if (nb_item > 0)
			set_fwd_ports_list(parsed_items.portlist, nb_item);
	}
}

cmdline_parse_token_string_t cmd_set_list_keyword =
	TOKEN_STRING_INITIALIZER(struct cmd_set_list_result, cmd_keyword,
				 "set");
cmdline_parse_token_string_t cmd_set_list_name =
	TOKEN_STRING_INITIALIZER(struct cmd_set_list_result, list_name,
				 "corelist#portlist");
cmdline_parse_token_string_t cmd_set_list_of_items =
	TOKEN_STRING_INITIALIZER(struct cmd_set_list_result, list_of_items,
				 NULL);

cmdline_parse_inst_t cmd_set_fwd_list = {
	.f = cmd_set_list_parsed,
	.data = NULL,
	.help_str = "set corelist|portlist x[,y]*",
	.tokens = {
		(void *)&cmd_set_list_keyword,
		(void *)&cmd_set_list_name,
		(void *)&cmd_set_list_of_items,
		NULL,
	},
};

/* *** SET COREMASK and PORTMASK CONFIGURATION *** */

struct cmd_setmask_result {
	cmdline_fixed_string_t set;
	cmdline_fixed_string_t mask;
	uint64_t hexavalue;
};

static void cmd_set_mask_parsed(void *parsed_result,
				__attribute__((unused)) struct cmdline *cl,
				__attribute__((unused)) void *data)
{
	struct cmd_setmask_result *res = parsed_result;

	if (!strcmp(res->mask, "coremask"))
		set_fwd_lcores_mask(res->hexavalue);
	else if (!strcmp(res->mask, "portmask"))
		set_fwd_ports_mask(res->hexavalue);
}

cmdline_parse_token_string_t cmd_setmask_set =
	TOKEN_STRING_INITIALIZER(struct cmd_setmask_result, set, "set");
cmdline_parse_token_string_t cmd_setmask_mask =
	TOKEN_STRING_INITIALIZER(struct cmd_setmask_result, mask,
				 "coremask#portmask");
cmdline_parse_token_num_t cmd_setmask_value =
	TOKEN_NUM_INITIALIZER(struct cmd_setmask_result, hexavalue, UINT64);

cmdline_parse_inst_t cmd_set_fwd_mask = {
	.f = cmd_set_mask_parsed,
	.data = NULL,
	.help_str = "set coremask|portmask hexadecimal value",
	.tokens = {
		(void *)&cmd_setmask_set,
		(void *)&cmd_setmask_mask,
		(void *)&cmd_setmask_value,
		NULL,
	},
};

/*
 * SET NBPORT, NBCORE, PACKET BURST, and VERBOSE LEVEL CONFIGURATION
 */
struct cmd_set_result {
	cmdline_fixed_string_t set;
	cmdline_fixed_string_t what;
	uint16_t value;
};

static void cmd_set_parsed(void *parsed_result,
			   __attribute__((unused)) struct cmdline *cl,
			   __attribute__((unused)) void *data)
{
	struct cmd_set_result *res = parsed_result;
	if (!strcmp(res->what, "nbport"))
		set_fwd_ports_number(res->value);
	else if (!strcmp(res->what, "nbcore"))
		set_fwd_lcores_number(res->value);
	else if (!strcmp(res->what, "burst"))
		set_nb_pkt_per_burst(res->value);
	else if (!strcmp(res->what, "verbose"))
		set_verbose_level(res->value);
}

cmdline_parse_token_string_t cmd_set_set =
	TOKEN_STRING_INITIALIZER(struct cmd_set_result, set, "set");
cmdline_parse_token_string_t cmd_set_what =
	TOKEN_STRING_INITIALIZER(struct cmd_set_result, what,
				 "nbport#nbcore#burst#verbose");
cmdline_parse_token_num_t cmd_set_value =
	TOKEN_NUM_INITIALIZER(struct cmd_set_result, value, UINT16);

cmdline_parse_inst_t cmd_set_numbers = {
	.f = cmd_set_parsed,
	.data = NULL,
	.help_str = "set nbport|nbcore|burst|verbose value",
	.tokens = {
		(void *)&cmd_set_set,
		(void *)&cmd_set_what,
		(void *)&cmd_set_value,
		NULL,
	},
};

/* *** SET SEGMENT LENGTHS OF TXONLY PACKETS *** */

struct cmd_set_txpkts_result {
	cmdline_fixed_string_t cmd_keyword;
	cmdline_fixed_string_t txpkts;
	cmdline_fixed_string_t seg_lengths;
};

static void
cmd_set_txpkts_parsed(void *parsed_result,
		      __attribute__((unused)) struct cmdline *cl,
		      __attribute__((unused)) void *data)
{
	struct cmd_set_txpkts_result *res;
	unsigned seg_lengths[RTE_MAX_SEGS_PER_PKT];
	unsigned int nb_segs;

	res = parsed_result;
	nb_segs = parse_item_list(res->seg_lengths, "segment lengths",
				  RTE_MAX_SEGS_PER_PKT, seg_lengths, 0);
	if (nb_segs > 0)
		set_tx_pkt_segments(seg_lengths, nb_segs);
}

cmdline_parse_token_string_t cmd_set_txpkts_keyword =
	TOKEN_STRING_INITIALIZER(struct cmd_set_txpkts_result,
				 cmd_keyword, "set");
cmdline_parse_token_string_t cmd_set_txpkts_name =
	TOKEN_STRING_INITIALIZER(struct cmd_set_txpkts_result,
				 txpkts, "txpkts");
cmdline_parse_token_string_t cmd_set_txpkts_lengths =
	TOKEN_STRING_INITIALIZER(struct cmd_set_txpkts_result,
				 seg_lengths, NULL);

cmdline_parse_inst_t cmd_set_txpkts = {
	.f = cmd_set_txpkts_parsed,
	.data = NULL,
	.help_str = "set txpkts x[,y]*",
	.tokens = {
		(void *)&cmd_set_txpkts_keyword,
		(void *)&cmd_set_txpkts_name,
		(void *)&cmd_set_txpkts_lengths,
		NULL,
	},
};

/* *** ADD/REMOVE ALL VLAN IDENTIFIERS TO/FROM A PORT VLAN RX FILTER *** */
struct cmd_rx_vlan_filter_all_result {
	cmdline_fixed_string_t rx_vlan;
	cmdline_fixed_string_t what;
	cmdline_fixed_string_t all;
	uint8_t port_id;
};

static void
cmd_rx_vlan_filter_all_parsed(void *parsed_result,
			      __attribute__((unused)) struct cmdline *cl,
			      __attribute__((unused)) void *data)
{
	struct cmd_rx_vlan_filter_all_result *res = parsed_result;

	if (!strcmp(res->what, "add"))
		rx_vlan_all_filter_set(res->port_id, 1);
	else
		rx_vlan_all_filter_set(res->port_id, 0);
}

cmdline_parse_token_string_t cmd_rx_vlan_filter_all_rx_vlan =
	TOKEN_STRING_INITIALIZER(struct cmd_rx_vlan_filter_all_result,
				 rx_vlan, "rx_vlan");
cmdline_parse_token_string_t cmd_rx_vlan_filter_all_what =
	TOKEN_STRING_INITIALIZER(struct cmd_rx_vlan_filter_all_result,
				 what, "add#rm");
cmdline_parse_token_string_t cmd_rx_vlan_filter_all_all =
	TOKEN_STRING_INITIALIZER(struct cmd_rx_vlan_filter_all_result,
				 all, "all");
cmdline_parse_token_num_t cmd_rx_vlan_filter_all_portid =
	TOKEN_NUM_INITIALIZER(struct cmd_rx_vlan_filter_all_result,
			      port_id, UINT8);

cmdline_parse_inst_t cmd_rx_vlan_filter_all = {
	.f = cmd_rx_vlan_filter_all_parsed,
	.data = NULL,
	.help_str = "add/remove all identifiers to/from the set of VLAN "
	"Identifiers filtered by a port",
	.tokens = {
		(void *)&cmd_rx_vlan_filter_all_rx_vlan,
		(void *)&cmd_rx_vlan_filter_all_what,
		(void *)&cmd_rx_vlan_filter_all_all,
		(void *)&cmd_rx_vlan_filter_all_portid,
		NULL,
	},
};

/* *** VLAN OFFLOAD SET ON A PORT *** */
struct cmd_vlan_offload_result {
	cmdline_fixed_string_t vlan;
	cmdline_fixed_string_t set;
	cmdline_fixed_string_t what;
	cmdline_fixed_string_t on;
	cmdline_fixed_string_t port_id;
};

static void
cmd_vlan_offload_parsed(void *parsed_result,
			  __attribute__((unused)) struct cmdline *cl,
			  __attribute__((unused)) void *data)
{
	int on;
	struct cmd_vlan_offload_result *res = parsed_result;	
	char *str;
	int i, len = 0;
	portid_t port_id = 0;
	unsigned int tmp;
	
	str = res->port_id;
	len = strnlen(str, STR_TOKEN_SIZE);
	i = 0;
	/* Get port_id first */
	while(i < len){
		if(str[i] == ',')
			break;
		
		i++;
	}
	str[i]='\0';
	tmp = strtoul(str, NULL, 0);
	/* If port_id greater that what portid_t can represent, return */
	if(tmp > 255)
		return;
	port_id = (portid_t)tmp;

	if (!strcmp(res->on, "on"))
		on = 1;
	else
		on = 0;

	if (!strcmp(res->what, "strip"))
		rx_vlan_strip_set(port_id,  on);
	else if(!strcmp(res->what, "stripq")){
		uint16_t queue_id = 0;

		/* No queue_id, return */
		if(i + 1 >= len)
			return;
		tmp = strtoul(str + i + 1, NULL, 0);
		/* If queue_id greater that what 16-bits can represent, return */
		if(tmp > 0xffff)
			return;
		
		queue_id = (uint16_t)tmp;
		rx_vlan_strip_set_on_queue(port_id, queue_id, on);
	}
	else if (!strcmp(res->what, "filter"))
		rx_vlan_filter_set(port_id, on);
	else
		vlan_extend_set(port_id, on);

	return;
}

cmdline_parse_token_string_t cmd_vlan_offload_vlan =
	TOKEN_STRING_INITIALIZER(struct cmd_vlan_offload_result,
				 vlan, "vlan");
cmdline_parse_token_string_t cmd_vlan_offload_set =
	TOKEN_STRING_INITIALIZER(struct cmd_vlan_offload_result,
				 set, "set");
cmdline_parse_token_string_t cmd_vlan_offload_what =
	TOKEN_STRING_INITIALIZER(struct cmd_vlan_offload_result,
				 what, "strip#filter#qinq#stripq");
cmdline_parse_token_string_t cmd_vlan_offload_on =
	TOKEN_STRING_INITIALIZER(struct cmd_vlan_offload_result,
			      on, "on#off");
cmdline_parse_token_string_t cmd_vlan_offload_portid =
	TOKEN_STRING_INITIALIZER(struct cmd_vlan_offload_result,
			      port_id, NULL);

cmdline_parse_inst_t cmd_vlan_offload = {
	.f = cmd_vlan_offload_parsed,
	.data = NULL,
	.help_str = "set strip|filter|qinq|stripq on|off port_id[,queue_id], filter/strip for rx side"
	" qinq(extended) for both rx/tx sides ",
	.tokens = {
		(void *)&cmd_vlan_offload_vlan,
		(void *)&cmd_vlan_offload_set,
		(void *)&cmd_vlan_offload_what,
		(void *)&cmd_vlan_offload_on,
		(void *)&cmd_vlan_offload_portid,
		NULL,
	},
};

/* *** VLAN TPID SET ON A PORT *** */
struct cmd_vlan_tpid_result {
	cmdline_fixed_string_t vlan;
	cmdline_fixed_string_t set;
	cmdline_fixed_string_t what;
	uint16_t tp_id;
	uint8_t port_id;
};

static void
cmd_vlan_tpid_parsed(void *parsed_result,
			  __attribute__((unused)) struct cmdline *cl,
			  __attribute__((unused)) void *data)
{
	struct cmd_vlan_tpid_result *res = parsed_result;
	vlan_tpid_set(res->port_id, res->tp_id);
	return;
}

cmdline_parse_token_string_t cmd_vlan_tpid_vlan =
	TOKEN_STRING_INITIALIZER(struct cmd_vlan_tpid_result,
				 vlan, "vlan");
cmdline_parse_token_string_t cmd_vlan_tpid_set =
	TOKEN_STRING_INITIALIZER(struct cmd_vlan_tpid_result,
				 set, "set");
cmdline_parse_token_string_t cmd_vlan_tpid_what =
	TOKEN_STRING_INITIALIZER(struct cmd_vlan_tpid_result,
				 what, "tpid");
cmdline_parse_token_num_t cmd_vlan_tpid_tpid =
	TOKEN_NUM_INITIALIZER(struct cmd_vlan_tpid_result,
			      tp_id, UINT16);
cmdline_parse_token_num_t cmd_vlan_tpid_portid =
	TOKEN_NUM_INITIALIZER(struct cmd_vlan_tpid_result,
			      port_id, UINT8);

cmdline_parse_inst_t cmd_vlan_tpid = {
	.f = cmd_vlan_tpid_parsed,
	.data = NULL,
	.help_str = "set tpid tp_id port_id, set the Outer VLAN Ether type",
	.tokens = {
		(void *)&cmd_vlan_tpid_vlan,
		(void *)&cmd_vlan_tpid_set,
		(void *)&cmd_vlan_tpid_what,
		(void *)&cmd_vlan_tpid_tpid,
		(void *)&cmd_vlan_tpid_portid,
		NULL,
	},
};

/* *** ADD/REMOVE A VLAN IDENTIFIER TO/FROM A PORT VLAN RX FILTER *** */
struct cmd_rx_vlan_filter_result {
	cmdline_fixed_string_t rx_vlan;
	cmdline_fixed_string_t what;
	uint16_t vlan_id;
	uint8_t port_id;
};

static void
cmd_rx_vlan_filter_parsed(void *parsed_result,
			  __attribute__((unused)) struct cmdline *cl,
			  __attribute__((unused)) void *data)
{
	struct cmd_rx_vlan_filter_result *res = parsed_result;

	if (!strcmp(res->what, "add"))
		rx_vft_set(res->port_id, res->vlan_id, 1);
	else
		rx_vft_set(res->port_id, res->vlan_id, 0);
}

cmdline_parse_token_string_t cmd_rx_vlan_filter_rx_vlan =
	TOKEN_STRING_INITIALIZER(struct cmd_rx_vlan_filter_result,
				 rx_vlan, "rx_vlan");
cmdline_parse_token_string_t cmd_rx_vlan_filter_what =
	TOKEN_STRING_INITIALIZER(struct cmd_rx_vlan_filter_result,
				 what, "add#rm");
cmdline_parse_token_num_t cmd_rx_vlan_filter_vlanid =
	TOKEN_NUM_INITIALIZER(struct cmd_rx_vlan_filter_result,
			      vlan_id, UINT16);
cmdline_parse_token_num_t cmd_rx_vlan_filter_portid =
	TOKEN_NUM_INITIALIZER(struct cmd_rx_vlan_filter_result,
			      port_id, UINT8);

cmdline_parse_inst_t cmd_rx_vlan_filter = {
	.f = cmd_rx_vlan_filter_parsed,
	.data = NULL,
	.help_str = "add/remove a VLAN identifier to/from the set of VLAN "
	"Identifiers filtered by a port",
	.tokens = {
		(void *)&cmd_rx_vlan_filter_rx_vlan,
		(void *)&cmd_rx_vlan_filter_what,
		(void *)&cmd_rx_vlan_filter_vlanid,
		(void *)&cmd_rx_vlan_filter_portid,
		NULL,
	},
};

/* *** ENABLE HARDWARE INSERTION OF VLAN HEADER IN TX PACKETS *** */
struct cmd_tx_vlan_set_result {
	cmdline_fixed_string_t tx_vlan;
	cmdline_fixed_string_t set;
	uint16_t vlan_id;
	uint8_t port_id;
};

static void
cmd_tx_vlan_set_parsed(void *parsed_result,
		       __attribute__((unused)) struct cmdline *cl,
		       __attribute__((unused)) void *data)
{
	struct cmd_tx_vlan_set_result *res = parsed_result;
	tx_vlan_set(res->port_id, res->vlan_id);
}

cmdline_parse_token_string_t cmd_tx_vlan_set_tx_vlan =
	TOKEN_STRING_INITIALIZER(struct cmd_tx_vlan_set_result,
				 tx_vlan, "tx_vlan");
cmdline_parse_token_string_t cmd_tx_vlan_set_set =
	TOKEN_STRING_INITIALIZER(struct cmd_tx_vlan_set_result,
				 set, "set");
cmdline_parse_token_num_t cmd_tx_vlan_set_vlanid =
	TOKEN_NUM_INITIALIZER(struct cmd_tx_vlan_set_result,
			      vlan_id, UINT16);
cmdline_parse_token_num_t cmd_tx_vlan_set_portid =
	TOKEN_NUM_INITIALIZER(struct cmd_tx_vlan_set_result,
			      port_id, UINT8);

cmdline_parse_inst_t cmd_tx_vlan_set = {
	.f = cmd_tx_vlan_set_parsed,
	.data = NULL,
	.help_str = "enable hardware insertion of a VLAN header with a given "
	"TAG Identifier in packets sent on a port",
	.tokens = {
		(void *)&cmd_tx_vlan_set_tx_vlan,
		(void *)&cmd_tx_vlan_set_set,
		(void *)&cmd_tx_vlan_set_vlanid,
		(void *)&cmd_tx_vlan_set_portid,
		NULL,
	},
};

/* *** DISABLE HARDWARE INSERTION OF VLAN HEADER IN TX PACKETS *** */
struct cmd_tx_vlan_reset_result {
	cmdline_fixed_string_t tx_vlan;
	cmdline_fixed_string_t reset;
	uint8_t port_id;
};

static void
cmd_tx_vlan_reset_parsed(void *parsed_result,
			 __attribute__((unused)) struct cmdline *cl,
			 __attribute__((unused)) void *data)
{
	struct cmd_tx_vlan_reset_result *res = parsed_result;

	tx_vlan_reset(res->port_id);
}

cmdline_parse_token_string_t cmd_tx_vlan_reset_tx_vlan =
	TOKEN_STRING_INITIALIZER(struct cmd_tx_vlan_reset_result,
				 tx_vlan, "tx_vlan");
cmdline_parse_token_string_t cmd_tx_vlan_reset_reset =
	TOKEN_STRING_INITIALIZER(struct cmd_tx_vlan_reset_result,
				 reset, "reset");
cmdline_parse_token_num_t cmd_tx_vlan_reset_portid =
	TOKEN_NUM_INITIALIZER(struct cmd_tx_vlan_reset_result,
			      port_id, UINT8);

cmdline_parse_inst_t cmd_tx_vlan_reset = {
	.f = cmd_tx_vlan_reset_parsed,
	.data = NULL,
	.help_str = "disable hardware insertion of a VLAN header in packets "
	"sent on a port",
	.tokens = {
		(void *)&cmd_tx_vlan_reset_tx_vlan,
		(void *)&cmd_tx_vlan_reset_reset,
		(void *)&cmd_tx_vlan_reset_portid,
		NULL,
	},
};


/* *** ENABLE HARDWARE INSERTION OF CHECKSUM IN TX PACKETS *** */
struct cmd_tx_cksum_set_result {
	cmdline_fixed_string_t tx_cksum;
	cmdline_fixed_string_t set;
	uint8_t cksum_mask;
	uint8_t port_id;
};

static void
cmd_tx_cksum_set_parsed(void *parsed_result,
		       __attribute__((unused)) struct cmdline *cl,
		       __attribute__((unused)) void *data)
{
	struct cmd_tx_cksum_set_result *res = parsed_result;

	tx_cksum_set(res->port_id, res->cksum_mask);
}

cmdline_parse_token_string_t cmd_tx_cksum_set_tx_cksum =
	TOKEN_STRING_INITIALIZER(struct cmd_tx_cksum_set_result,
				tx_cksum, "tx_checksum");
cmdline_parse_token_string_t cmd_tx_cksum_set_set =
	TOKEN_STRING_INITIALIZER(struct cmd_tx_cksum_set_result,
				set, "set");
cmdline_parse_token_num_t cmd_tx_cksum_set_cksum_mask =
	TOKEN_NUM_INITIALIZER(struct cmd_tx_cksum_set_result,
				cksum_mask, UINT8);
cmdline_parse_token_num_t cmd_tx_cksum_set_portid =
	TOKEN_NUM_INITIALIZER(struct cmd_tx_cksum_set_result,
				port_id, UINT8);

cmdline_parse_inst_t cmd_tx_cksum_set = {
	.f = cmd_tx_cksum_set_parsed,
	.data = NULL,
	.help_str = "enable hardware insertion of L3/L4checksum with a given "
	"mask in packets sent on a port, the bit mapping is given as, Bit 0 for ip"
	"Bit 1 for UDP, Bit 2 for TCP, Bit 3 for SCTP",
	.tokens = {
		(void *)&cmd_tx_cksum_set_tx_cksum,
		(void *)&cmd_tx_cksum_set_set,
		(void *)&cmd_tx_cksum_set_cksum_mask,
		(void *)&cmd_tx_cksum_set_portid,
		NULL,
	},
};

/* *** SET FORWARDING MODE *** */
struct cmd_set_fwd_mode_result {
	cmdline_fixed_string_t set;
	cmdline_fixed_string_t fwd;
	cmdline_fixed_string_t mode;
};

static void cmd_set_fwd_mode_parsed(void *parsed_result,
				    __attribute__((unused)) struct cmdline *cl,
				    __attribute__((unused)) void *data)
{
	struct cmd_set_fwd_mode_result *res = parsed_result;

	set_pkt_forwarding_mode(res->mode);
}

cmdline_parse_token_string_t cmd_setfwd_set =
	TOKEN_STRING_INITIALIZER(struct cmd_set_fwd_mode_result, set, "set");
cmdline_parse_token_string_t cmd_setfwd_fwd =
	TOKEN_STRING_INITIALIZER(struct cmd_set_fwd_mode_result, fwd, "fwd");
cmdline_parse_token_string_t cmd_setfwd_mode =
	TOKEN_STRING_INITIALIZER(struct cmd_set_fwd_mode_result, mode,
#ifdef RTE_LIBRTE_IEEE1588
				 "io#mac#rxonly#txonly#csum#ieee1588");
#else
				 "io#mac#rxonly#txonly#csum");
#endif

cmdline_parse_inst_t cmd_set_fwd_mode = {
	.f = cmd_set_fwd_mode_parsed,
	.data = NULL,
#ifdef RTE_LIBRTE_IEEE1588
	.help_str = "set fwd io|mac|rxonly|txonly|csum|ieee1588 - set IO, MAC,"
	" RXONLY, TXONLY, CSUM or IEEE1588 packet forwarding mode",
#else
	.help_str = "set fwd io|mac|rxonly|txonly|csum - set IO, MAC,"
	" RXONLY, CSUM or TXONLY packet forwarding mode",
#endif
	.tokens = {
		(void *)&cmd_setfwd_set,
		(void *)&cmd_setfwd_fwd,
		(void *)&cmd_setfwd_mode,
		NULL,
	},
};

/* *** SET PROMISC MODE *** */
struct cmd_set_promisc_mode_result {
	cmdline_fixed_string_t set;
	cmdline_fixed_string_t promisc;
	cmdline_fixed_string_t port_all; /* valid if "allports" argument == 1 */
	uint8_t port_num;                /* valid if "allports" argument == 0 */
	cmdline_fixed_string_t mode;
};

static void cmd_set_promisc_mode_parsed(void *parsed_result,
					__attribute__((unused)) struct cmdline *cl,
					void *allports)
{
	struct cmd_set_promisc_mode_result *res = parsed_result;
	int enable;
	portid_t i;

	if (!strcmp(res->mode, "on"))
		enable = 1;
	else
		enable = 0;

	/* all ports */
	if (allports) {
		for (i = 0; i < nb_ports; i++) {
			if (enable)
				rte_eth_promiscuous_enable(i);
			else
				rte_eth_promiscuous_disable(i);
		}
	}
	else {
		if (enable)
			rte_eth_promiscuous_enable(res->port_num);
		else
			rte_eth_promiscuous_disable(res->port_num);
	}
}

cmdline_parse_token_string_t cmd_setpromisc_set =
	TOKEN_STRING_INITIALIZER(struct cmd_set_promisc_mode_result, set, "set");
cmdline_parse_token_string_t cmd_setpromisc_promisc =
	TOKEN_STRING_INITIALIZER(struct cmd_set_promisc_mode_result, promisc,
				 "promisc");
cmdline_parse_token_string_t cmd_setpromisc_portall =
	TOKEN_STRING_INITIALIZER(struct cmd_set_promisc_mode_result, port_all,
				 "all");
cmdline_parse_token_num_t cmd_setpromisc_portnum =
	TOKEN_NUM_INITIALIZER(struct cmd_set_promisc_mode_result, port_num,
			      UINT8);
cmdline_parse_token_string_t cmd_setpromisc_mode =
	TOKEN_STRING_INITIALIZER(struct cmd_set_promisc_mode_result, mode,
				 "on#off");

cmdline_parse_inst_t cmd_set_promisc_mode_all = {
	.f = cmd_set_promisc_mode_parsed,
	.data = (void *)1,
	.help_str = "set promisc all on|off: set promisc mode for all ports",
	.tokens = {
		(void *)&cmd_setpromisc_set,
		(void *)&cmd_setpromisc_promisc,
		(void *)&cmd_setpromisc_portall,
		(void *)&cmd_setpromisc_mode,
		NULL,
	},
};

cmdline_parse_inst_t cmd_set_promisc_mode_one = {
	.f = cmd_set_promisc_mode_parsed,
	.data = (void *)0,
	.help_str = "set promisc X on|off: set promisc mode on port X",
	.tokens = {
		(void *)&cmd_setpromisc_set,
		(void *)&cmd_setpromisc_promisc,
		(void *)&cmd_setpromisc_portnum,
		(void *)&cmd_setpromisc_mode,
		NULL,
	},
};

/* *** SET ALLMULTI MODE *** */
struct cmd_set_allmulti_mode_result {
	cmdline_fixed_string_t set;
	cmdline_fixed_string_t allmulti;
	cmdline_fixed_string_t port_all; /* valid if "allports" argument == 1 */
	uint8_t port_num;                /* valid if "allports" argument == 0 */
	cmdline_fixed_string_t mode;
};

static void cmd_set_allmulti_mode_parsed(void *parsed_result,
					__attribute__((unused)) struct cmdline *cl,
					void *allports)
{
	struct cmd_set_allmulti_mode_result *res = parsed_result;
	int enable;
	portid_t i;

	if (!strcmp(res->mode, "on"))
		enable = 1;
	else
		enable = 0;

	/* all ports */
	if (allports) {
		for (i = 0; i < nb_ports; i++) {
			if (enable)
				rte_eth_allmulticast_enable(i);
			else
				rte_eth_allmulticast_disable(i);
		}
	}
	else {
		if (enable)
			rte_eth_allmulticast_enable(res->port_num);
		else
			rte_eth_allmulticast_disable(res->port_num);
	}
}

cmdline_parse_token_string_t cmd_setallmulti_set =
	TOKEN_STRING_INITIALIZER(struct cmd_set_allmulti_mode_result, set, "set");
cmdline_parse_token_string_t cmd_setallmulti_allmulti =
	TOKEN_STRING_INITIALIZER(struct cmd_set_allmulti_mode_result, allmulti,
				 "allmulti");
cmdline_parse_token_string_t cmd_setallmulti_portall =
	TOKEN_STRING_INITIALIZER(struct cmd_set_allmulti_mode_result, port_all,
				 "all");
cmdline_parse_token_num_t cmd_setallmulti_portnum =
	TOKEN_NUM_INITIALIZER(struct cmd_set_allmulti_mode_result, port_num,
			      UINT8);
cmdline_parse_token_string_t cmd_setallmulti_mode =
	TOKEN_STRING_INITIALIZER(struct cmd_set_allmulti_mode_result, mode,
				 "on#off");

cmdline_parse_inst_t cmd_set_allmulti_mode_all = {
	.f = cmd_set_allmulti_mode_parsed,
	.data = (void *)1,
	.help_str = "set allmulti all on|off: set allmulti mode for all ports",
	.tokens = {
		(void *)&cmd_setallmulti_set,
		(void *)&cmd_setallmulti_allmulti,
		(void *)&cmd_setallmulti_portall,
		(void *)&cmd_setallmulti_mode,
		NULL,
	},
};

cmdline_parse_inst_t cmd_set_allmulti_mode_one = {
	.f = cmd_set_allmulti_mode_parsed,
	.data = (void *)0,
	.help_str = "set allmulti X on|off: set allmulti mode on port X",
	.tokens = {
		(void *)&cmd_setallmulti_set,
		(void *)&cmd_setallmulti_allmulti,
		(void *)&cmd_setallmulti_portnum,
		(void *)&cmd_setallmulti_mode,
		NULL,
	},
};

/* *** ADD/REMOVE A PKT FILTER *** */
struct cmd_pkt_filter_result {
	cmdline_fixed_string_t pkt_filter;
	uint8_t  port_id;
	cmdline_fixed_string_t protocol;
	cmdline_fixed_string_t src;
	cmdline_ipaddr_t ip_src;
	uint16_t port_src;
	cmdline_fixed_string_t dst;
	cmdline_ipaddr_t ip_dst;
	uint16_t port_dst;
	cmdline_fixed_string_t flexbytes;
	uint16_t flexbytes_value;
	cmdline_fixed_string_t vlan;
	uint16_t  vlan_id;
	cmdline_fixed_string_t queue;
	int8_t  queue_id;
	cmdline_fixed_string_t soft;
	uint8_t  soft_id;
};

static void
cmd_pkt_filter_parsed(void *parsed_result,
			  __attribute__((unused)) struct cmdline *cl,
			  __attribute__((unused)) void *data)
{
	struct rte_fdir_filter fdir_filter;
	struct cmd_pkt_filter_result *res = parsed_result;

	memset(&fdir_filter, 0, sizeof(struct rte_fdir_filter));

	if (res->ip_src.family == AF_INET)
		fdir_filter.ip_src.ipv4_addr = res->ip_src.addr.ipv4.s_addr;
	else
		memcpy(&(fdir_filter.ip_src.ipv6_addr),
		       &(res->ip_src.addr.ipv6),
		       sizeof(struct in6_addr));

	if (res->ip_dst.family == AF_INET)
		fdir_filter.ip_dst.ipv4_addr = res->ip_dst.addr.ipv4.s_addr;
	else
		memcpy(&(fdir_filter.ip_dst.ipv6_addr),
		       &(res->ip_dst.addr.ipv6),
		       sizeof(struct in6_addr));

	fdir_filter.port_dst = rte_cpu_to_be_16(res->port_dst);
	fdir_filter.port_src = rte_cpu_to_be_16(res->port_src);

	if (!strcmp(res->protocol, "udp"))
		fdir_filter.l4type = RTE_FDIR_L4TYPE_UDP;
	else if (!strcmp(res->protocol, "tcp"))
		fdir_filter.l4type = RTE_FDIR_L4TYPE_TCP;
	else if (!strcmp(res->protocol, "sctp"))
		fdir_filter.l4type = RTE_FDIR_L4TYPE_SCTP;
	else /* default only IP */
		fdir_filter.l4type = RTE_FDIR_L4TYPE_NONE;

	if (res->ip_dst.family == AF_INET6)
		fdir_filter.iptype = RTE_FDIR_IPTYPE_IPV6;
	else
		fdir_filter.iptype = RTE_FDIR_IPTYPE_IPV4;

	fdir_filter.vlan_id    = rte_cpu_to_be_16(res->vlan_id);
	fdir_filter.flex_bytes = rte_cpu_to_be_16(res->flexbytes_value);

	if (!strcmp(res->pkt_filter, "add_signature_filter"))
		fdir_add_signature_filter(res->port_id, res->queue_id,
					  &fdir_filter);
	else if (!strcmp(res->pkt_filter, "upd_signature_filter"))
		fdir_update_signature_filter(res->port_id, res->queue_id,
					     &fdir_filter);
	else if (!strcmp(res->pkt_filter, "rm_signature_filter"))
		fdir_remove_signature_filter(res->port_id, &fdir_filter);
	else if (!strcmp(res->pkt_filter, "add_perfect_filter"))
		fdir_add_perfect_filter(res->port_id, res->soft_id,
					res->queue_id,
					(uint8_t) (res->queue_id < 0),
					&fdir_filter);
	else if (!strcmp(res->pkt_filter, "upd_perfect_filter"))
		fdir_update_perfect_filter(res->port_id, res->soft_id,
					   res->queue_id,
					   (uint8_t) (res->queue_id < 0),
					   &fdir_filter);
	else if (!strcmp(res->pkt_filter, "rm_perfect_filter"))
		fdir_remove_perfect_filter(res->port_id, res->soft_id,
					   &fdir_filter);

}


cmdline_parse_token_num_t cmd_pkt_filter_port_id =
	TOKEN_NUM_INITIALIZER(struct cmd_pkt_filter_result,
			      port_id, UINT8);
cmdline_parse_token_string_t cmd_pkt_filter_protocol =
	TOKEN_STRING_INITIALIZER(struct cmd_pkt_filter_result,
				 protocol, "ip#tcp#udp#sctp");
cmdline_parse_token_string_t cmd_pkt_filter_src =
	TOKEN_STRING_INITIALIZER(struct cmd_pkt_filter_result,
				 src, "src");
cmdline_parse_token_ipaddr_t cmd_pkt_filter_ip_src =
	TOKEN_IPADDR_INITIALIZER(struct cmd_pkt_filter_result,
				 ip_src);
cmdline_parse_token_num_t cmd_pkt_filter_port_src =
	TOKEN_NUM_INITIALIZER(struct cmd_pkt_filter_result,
			      port_src, UINT16);
cmdline_parse_token_string_t cmd_pkt_filter_dst =
	TOKEN_STRING_INITIALIZER(struct cmd_pkt_filter_result,
				 dst, "dst");
cmdline_parse_token_ipaddr_t cmd_pkt_filter_ip_dst =
	TOKEN_IPADDR_INITIALIZER(struct cmd_pkt_filter_result,
				 ip_dst);
cmdline_parse_token_num_t cmd_pkt_filter_port_dst =
	TOKEN_NUM_INITIALIZER(struct cmd_pkt_filter_result,
			      port_dst, UINT16);
cmdline_parse_token_string_t cmd_pkt_filter_flexbytes =
	TOKEN_STRING_INITIALIZER(struct cmd_pkt_filter_result,
				 flexbytes, "flexbytes");
cmdline_parse_token_num_t cmd_pkt_filter_flexbytes_value =
	TOKEN_NUM_INITIALIZER(struct cmd_pkt_filter_result,
			      flexbytes_value, UINT16);
cmdline_parse_token_string_t cmd_pkt_filter_vlan =
	TOKEN_STRING_INITIALIZER(struct cmd_pkt_filter_result,
				 vlan, "vlan");
cmdline_parse_token_num_t cmd_pkt_filter_vlan_id =
	TOKEN_NUM_INITIALIZER(struct cmd_pkt_filter_result,
			      vlan_id, UINT16);
cmdline_parse_token_string_t cmd_pkt_filter_queue =
	TOKEN_STRING_INITIALIZER(struct cmd_pkt_filter_result,
				 queue, "queue");
cmdline_parse_token_num_t cmd_pkt_filter_queue_id =
	TOKEN_NUM_INITIALIZER(struct cmd_pkt_filter_result,
			      queue_id, INT8);
cmdline_parse_token_string_t cmd_pkt_filter_soft =
	TOKEN_STRING_INITIALIZER(struct cmd_pkt_filter_result,
				 soft, "soft");
cmdline_parse_token_num_t cmd_pkt_filter_soft_id =
	TOKEN_NUM_INITIALIZER(struct cmd_pkt_filter_result,
			      soft_id, UINT16);


cmdline_parse_token_string_t cmd_pkt_filter_add_signature_filter =
	TOKEN_STRING_INITIALIZER(struct cmd_pkt_filter_result,
				 pkt_filter, "add_signature_filter");
cmdline_parse_inst_t cmd_add_signature_filter = {
	.f = cmd_pkt_filter_parsed,
	.data = NULL,
	.help_str = "add a signature filter",
	.tokens = {
		(void *)&cmd_pkt_filter_add_signature_filter,
		(void *)&cmd_pkt_filter_port_id,
		(void *)&cmd_pkt_filter_protocol,
		(void *)&cmd_pkt_filter_src,
		(void *)&cmd_pkt_filter_ip_src,
		(void *)&cmd_pkt_filter_port_src,
		(void *)&cmd_pkt_filter_dst,
		(void *)&cmd_pkt_filter_ip_dst,
		(void *)&cmd_pkt_filter_port_dst,
		(void *)&cmd_pkt_filter_flexbytes,
		(void *)&cmd_pkt_filter_flexbytes_value,
		(void *)&cmd_pkt_filter_vlan,
		(void *)&cmd_pkt_filter_vlan_id,
		(void *)&cmd_pkt_filter_queue,
		(void *)&cmd_pkt_filter_queue_id,
		NULL,
	},
};


cmdline_parse_token_string_t cmd_pkt_filter_upd_signature_filter =
	TOKEN_STRING_INITIALIZER(struct cmd_pkt_filter_result,
				 pkt_filter, "upd_signature_filter");
cmdline_parse_inst_t cmd_upd_signature_filter = {
	.f = cmd_pkt_filter_parsed,
	.data = NULL,
	.help_str = "update a signature filter",
	.tokens = {
		(void *)&cmd_pkt_filter_upd_signature_filter,
		(void *)&cmd_pkt_filter_port_id,
		(void *)&cmd_pkt_filter_protocol,
		(void *)&cmd_pkt_filter_src,
		(void *)&cmd_pkt_filter_ip_src,
		(void *)&cmd_pkt_filter_port_src,
		(void *)&cmd_pkt_filter_dst,
		(void *)&cmd_pkt_filter_ip_dst,
		(void *)&cmd_pkt_filter_port_dst,
		(void *)&cmd_pkt_filter_flexbytes,
		(void *)&cmd_pkt_filter_flexbytes_value,
		(void *)&cmd_pkt_filter_vlan,
		(void *)&cmd_pkt_filter_vlan_id,
		(void *)&cmd_pkt_filter_queue,
		(void *)&cmd_pkt_filter_queue_id,
		NULL,
	},
};


cmdline_parse_token_string_t cmd_pkt_filter_rm_signature_filter =
	TOKEN_STRING_INITIALIZER(struct cmd_pkt_filter_result,
				 pkt_filter, "rm_signature_filter");
cmdline_parse_inst_t cmd_rm_signature_filter = {
	.f = cmd_pkt_filter_parsed,
	.data = NULL,
	.help_str = "remove a signature filter",
	.tokens = {
		(void *)&cmd_pkt_filter_rm_signature_filter,
		(void *)&cmd_pkt_filter_port_id,
		(void *)&cmd_pkt_filter_protocol,
		(void *)&cmd_pkt_filter_src,
		(void *)&cmd_pkt_filter_ip_src,
		(void *)&cmd_pkt_filter_port_src,
		(void *)&cmd_pkt_filter_dst,
		(void *)&cmd_pkt_filter_ip_dst,
		(void *)&cmd_pkt_filter_port_dst,
		(void *)&cmd_pkt_filter_flexbytes,
		(void *)&cmd_pkt_filter_flexbytes_value,
		(void *)&cmd_pkt_filter_vlan,
		(void *)&cmd_pkt_filter_vlan_id,
		NULL
		},
};


cmdline_parse_token_string_t cmd_pkt_filter_add_perfect_filter =
	TOKEN_STRING_INITIALIZER(struct cmd_pkt_filter_result,
				 pkt_filter, "add_perfect_filter");
cmdline_parse_inst_t cmd_add_perfect_filter = {
	.f = cmd_pkt_filter_parsed,
	.data = NULL,
	.help_str = "add a perfect filter",
	.tokens = {
		(void *)&cmd_pkt_filter_add_perfect_filter,
		(void *)&cmd_pkt_filter_port_id,
		(void *)&cmd_pkt_filter_protocol,
		(void *)&cmd_pkt_filter_src,
		(void *)&cmd_pkt_filter_ip_src,
		(void *)&cmd_pkt_filter_port_src,
		(void *)&cmd_pkt_filter_dst,
		(void *)&cmd_pkt_filter_ip_dst,
		(void *)&cmd_pkt_filter_port_dst,
		(void *)&cmd_pkt_filter_flexbytes,
		(void *)&cmd_pkt_filter_flexbytes_value,
		(void *)&cmd_pkt_filter_vlan,
		(void *)&cmd_pkt_filter_vlan_id,
		(void *)&cmd_pkt_filter_queue,
		(void *)&cmd_pkt_filter_queue_id,
		(void *)&cmd_pkt_filter_soft,
		(void *)&cmd_pkt_filter_soft_id,
		NULL,
	},
};


cmdline_parse_token_string_t cmd_pkt_filter_upd_perfect_filter =
	TOKEN_STRING_INITIALIZER(struct cmd_pkt_filter_result,
				 pkt_filter, "upd_perfect_filter");
cmdline_parse_inst_t cmd_upd_perfect_filter = {
	.f = cmd_pkt_filter_parsed,
	.data = NULL,
	.help_str = "update a perfect filter",
	.tokens = {
		(void *)&cmd_pkt_filter_upd_perfect_filter,
		(void *)&cmd_pkt_filter_port_id,
		(void *)&cmd_pkt_filter_protocol,
		(void *)&cmd_pkt_filter_src,
		(void *)&cmd_pkt_filter_ip_src,
		(void *)&cmd_pkt_filter_port_src,
		(void *)&cmd_pkt_filter_dst,
		(void *)&cmd_pkt_filter_ip_dst,
		(void *)&cmd_pkt_filter_port_dst,
		(void *)&cmd_pkt_filter_flexbytes,
		(void *)&cmd_pkt_filter_flexbytes_value,
		(void *)&cmd_pkt_filter_vlan,
		(void *)&cmd_pkt_filter_vlan_id,
		(void *)&cmd_pkt_filter_queue,
		(void *)&cmd_pkt_filter_queue_id,
		(void *)&cmd_pkt_filter_soft,
		(void *)&cmd_pkt_filter_soft_id,
		NULL,
	},
};


cmdline_parse_token_string_t cmd_pkt_filter_rm_perfect_filter =
	TOKEN_STRING_INITIALIZER(struct cmd_pkt_filter_result,
				 pkt_filter, "rm_perfect_filter");
cmdline_parse_inst_t cmd_rm_perfect_filter = {
	.f = cmd_pkt_filter_parsed,
	.data = NULL,
	.help_str = "remove a perfect filter",
	.tokens = {
		(void *)&cmd_pkt_filter_rm_perfect_filter,
		(void *)&cmd_pkt_filter_port_id,
		(void *)&cmd_pkt_filter_protocol,
		(void *)&cmd_pkt_filter_src,
		(void *)&cmd_pkt_filter_ip_src,
		(void *)&cmd_pkt_filter_port_src,
		(void *)&cmd_pkt_filter_dst,
		(void *)&cmd_pkt_filter_ip_dst,
		(void *)&cmd_pkt_filter_port_dst,
		(void *)&cmd_pkt_filter_flexbytes,
		(void *)&cmd_pkt_filter_flexbytes_value,
		(void *)&cmd_pkt_filter_vlan,
		(void *)&cmd_pkt_filter_vlan_id,
		(void *)&cmd_pkt_filter_soft,
		(void *)&cmd_pkt_filter_soft_id,
		NULL,
	},
};

/* *** SETUP MASKS FILTER *** */
struct cmd_pkt_filter_masks_result {
	cmdline_fixed_string_t filter_mask;
	uint8_t  port_id;
	cmdline_fixed_string_t src_mask;
	uint32_t ip_src_mask;
	uint16_t ipv6_src_mask;
	uint16_t port_src_mask;
	cmdline_fixed_string_t dst_mask;
	uint32_t ip_dst_mask;
	uint16_t ipv6_dst_mask;
	uint16_t port_dst_mask;
	cmdline_fixed_string_t flexbytes;
	uint8_t flexbytes_value;
	cmdline_fixed_string_t vlan_id;
	uint8_t  vlan_id_value;
	cmdline_fixed_string_t vlan_prio;
	uint8_t  vlan_prio_value;
	cmdline_fixed_string_t only_ip_flow;
	uint8_t  only_ip_flow_value;
	cmdline_fixed_string_t comp_ipv6_dst;
	uint8_t  comp_ipv6_dst_value;
};

static void
cmd_pkt_filter_masks_parsed(void *parsed_result,
			  __attribute__((unused)) struct cmdline *cl,
			  __attribute__((unused)) void *data)
{
	struct rte_fdir_masks fdir_masks;
	struct cmd_pkt_filter_masks_result *res = parsed_result;

	memset(&fdir_masks, 0, sizeof(struct rte_fdir_masks));

	fdir_masks.only_ip_flow  = res->only_ip_flow_value;
	fdir_masks.vlan_id       = res->vlan_id_value;
	fdir_masks.vlan_prio     = res->vlan_prio_value;
	fdir_masks.dst_ipv4_mask = res->ip_dst_mask;
	fdir_masks.src_ipv4_mask = res->ip_src_mask;
	fdir_masks.src_port_mask = res->port_src_mask;
	fdir_masks.dst_port_mask = res->port_dst_mask;
	fdir_masks.flexbytes     = res->flexbytes_value;

	fdir_set_masks(res->port_id, &fdir_masks);
}

cmdline_parse_token_string_t cmd_pkt_filter_masks_filter_mask =
	TOKEN_STRING_INITIALIZER(struct cmd_pkt_filter_masks_result,
				 filter_mask, "set_masks_filter");
cmdline_parse_token_num_t cmd_pkt_filter_masks_port_id =
	TOKEN_NUM_INITIALIZER(struct cmd_pkt_filter_masks_result,
			      port_id, UINT8);
cmdline_parse_token_string_t cmd_pkt_filter_masks_only_ip_flow =
	TOKEN_STRING_INITIALIZER(struct cmd_pkt_filter_masks_result,
				 only_ip_flow, "only_ip_flow");
cmdline_parse_token_num_t cmd_pkt_filter_masks_only_ip_flow_value =
	TOKEN_NUM_INITIALIZER(struct cmd_pkt_filter_masks_result,
			      only_ip_flow_value, UINT8);
cmdline_parse_token_string_t cmd_pkt_filter_masks_src_mask =
	TOKEN_STRING_INITIALIZER(struct cmd_pkt_filter_masks_result,
				 src_mask, "src_mask");
cmdline_parse_token_num_t cmd_pkt_filter_masks_ip_src_mask =
	TOKEN_NUM_INITIALIZER(struct cmd_pkt_filter_masks_result,
			      ip_src_mask, UINT32);
cmdline_parse_token_num_t cmd_pkt_filter_masks_port_src_mask =
	TOKEN_NUM_INITIALIZER(struct cmd_pkt_filter_masks_result,
			      port_src_mask, UINT16);
cmdline_parse_token_string_t cmd_pkt_filter_masks_dst_mask =
	TOKEN_STRING_INITIALIZER(struct cmd_pkt_filter_masks_result,
				 dst_mask, "dst_mask");
cmdline_parse_token_num_t cmd_pkt_filter_masks_ip_dst_mask =
	TOKEN_NUM_INITIALIZER(struct cmd_pkt_filter_masks_result,
			      ip_dst_mask, UINT32);
cmdline_parse_token_num_t cmd_pkt_filter_masks_port_dst_mask =
	TOKEN_NUM_INITIALIZER(struct cmd_pkt_filter_masks_result,
			      port_dst_mask, UINT16);
cmdline_parse_token_string_t cmd_pkt_filter_masks_flexbytes =
	TOKEN_STRING_INITIALIZER(struct cmd_pkt_filter_masks_result,
				 flexbytes, "flexbytes");
cmdline_parse_token_num_t cmd_pkt_filter_masks_flexbytes_value =
	TOKEN_NUM_INITIALIZER(struct cmd_pkt_filter_masks_result,
			      flexbytes_value, UINT8);
cmdline_parse_token_string_t cmd_pkt_filter_masks_vlan_id =
	TOKEN_STRING_INITIALIZER(struct cmd_pkt_filter_masks_result,
				 vlan_id, "vlan_id");
cmdline_parse_token_num_t cmd_pkt_filter_masks_vlan_id_value =
	TOKEN_NUM_INITIALIZER(struct cmd_pkt_filter_masks_result,
			      vlan_id_value, UINT8);
cmdline_parse_token_string_t cmd_pkt_filter_masks_vlan_prio =
	TOKEN_STRING_INITIALIZER(struct cmd_pkt_filter_masks_result,
				 vlan_prio, "vlan_prio");
cmdline_parse_token_num_t cmd_pkt_filter_masks_vlan_prio_value =
	TOKEN_NUM_INITIALIZER(struct cmd_pkt_filter_masks_result,
			      vlan_prio_value, UINT8);

cmdline_parse_inst_t cmd_set_masks_filter = {
	.f = cmd_pkt_filter_masks_parsed,
	.data = NULL,
	.help_str = "setup masks filter",
	.tokens = {
		(void *)&cmd_pkt_filter_masks_filter_mask,
		(void *)&cmd_pkt_filter_masks_port_id,
		(void *)&cmd_pkt_filter_masks_only_ip_flow,
		(void *)&cmd_pkt_filter_masks_only_ip_flow_value,
		(void *)&cmd_pkt_filter_masks_src_mask,
		(void *)&cmd_pkt_filter_masks_ip_src_mask,
		(void *)&cmd_pkt_filter_masks_port_src_mask,
		(void *)&cmd_pkt_filter_masks_dst_mask,
		(void *)&cmd_pkt_filter_masks_ip_dst_mask,
		(void *)&cmd_pkt_filter_masks_port_dst_mask,
		(void *)&cmd_pkt_filter_masks_flexbytes,
		(void *)&cmd_pkt_filter_masks_flexbytes_value,
		(void *)&cmd_pkt_filter_masks_vlan_id,
		(void *)&cmd_pkt_filter_masks_vlan_id_value,
		(void *)&cmd_pkt_filter_masks_vlan_prio,
		(void *)&cmd_pkt_filter_masks_vlan_prio_value,
		NULL,
	},
};

static void
cmd_pkt_filter_masks_ipv6_parsed(void *parsed_result,
			  __attribute__((unused)) struct cmdline *cl,
			  __attribute__((unused)) void *data)
{
	struct rte_fdir_masks fdir_masks;
	struct cmd_pkt_filter_masks_result *res = parsed_result;

	memset(&fdir_masks, 0, sizeof(struct rte_fdir_masks));

	fdir_masks.set_ipv6_mask = 1;
	fdir_masks.only_ip_flow  = res->only_ip_flow_value;
	fdir_masks.vlan_id       = res->vlan_id_value;
	fdir_masks.vlan_prio     = res->vlan_prio_value;
	fdir_masks.dst_ipv6_mask = res->ipv6_dst_mask;
	fdir_masks.src_ipv6_mask = res->ipv6_src_mask;
	fdir_masks.src_port_mask = res->port_src_mask;
	fdir_masks.dst_port_mask = res->port_dst_mask;
	fdir_masks.flexbytes     = res->flexbytes_value;
	fdir_masks.comp_ipv6_dst = res->comp_ipv6_dst_value;

	fdir_set_masks(res->port_id, &fdir_masks);
}

cmdline_parse_token_string_t cmd_pkt_filter_masks_filter_mask_ipv6 =
	TOKEN_STRING_INITIALIZER(struct cmd_pkt_filter_masks_result,
				 filter_mask, "set_ipv6_masks_filter");
cmdline_parse_token_num_t cmd_pkt_filter_masks_src_mask_ipv6_value =
	TOKEN_NUM_INITIALIZER(struct cmd_pkt_filter_masks_result,
			      ipv6_src_mask, UINT16);
cmdline_parse_token_num_t cmd_pkt_filter_masks_dst_mask_ipv6_value =
	TOKEN_NUM_INITIALIZER(struct cmd_pkt_filter_masks_result,
			      ipv6_dst_mask, UINT16);

cmdline_parse_token_string_t cmd_pkt_filter_masks_comp_ipv6_dst =
	TOKEN_STRING_INITIALIZER(struct cmd_pkt_filter_masks_result,
				 comp_ipv6_dst, "compare_dst");
cmdline_parse_token_num_t cmd_pkt_filter_masks_comp_ipv6_dst_value =
	TOKEN_NUM_INITIALIZER(struct cmd_pkt_filter_masks_result,
			      comp_ipv6_dst_value, UINT8);

cmdline_parse_inst_t cmd_set_ipv6_masks_filter = {
	.f = cmd_pkt_filter_masks_ipv6_parsed,
	.data = NULL,
	.help_str = "setup ipv6 masks filter",
	.tokens = {
		(void *)&cmd_pkt_filter_masks_filter_mask_ipv6,
		(void *)&cmd_pkt_filter_masks_port_id,
		(void *)&cmd_pkt_filter_masks_only_ip_flow,
		(void *)&cmd_pkt_filter_masks_only_ip_flow_value,
		(void *)&cmd_pkt_filter_masks_src_mask,
		(void *)&cmd_pkt_filter_masks_src_mask_ipv6_value,
		(void *)&cmd_pkt_filter_masks_port_src_mask,
		(void *)&cmd_pkt_filter_masks_dst_mask,
		(void *)&cmd_pkt_filter_masks_dst_mask_ipv6_value,
		(void *)&cmd_pkt_filter_masks_port_dst_mask,
		(void *)&cmd_pkt_filter_masks_flexbytes,
		(void *)&cmd_pkt_filter_masks_flexbytes_value,
		(void *)&cmd_pkt_filter_masks_vlan_id,
		(void *)&cmd_pkt_filter_masks_vlan_id_value,
		(void *)&cmd_pkt_filter_masks_vlan_prio,
		(void *)&cmd_pkt_filter_masks_vlan_prio_value,
		(void *)&cmd_pkt_filter_masks_comp_ipv6_dst,
		(void *)&cmd_pkt_filter_masks_comp_ipv6_dst_value,
		NULL,
	},
};

/* *** SETUP ETHERNET LINK FLOW CONTROL *** */
struct cmd_link_flow_ctrl_set_result {
	cmdline_fixed_string_t set;
	cmdline_fixed_string_t flow_ctrl;
	cmdline_fixed_string_t rx;
	cmdline_fixed_string_t rx_lfc_mode;
	cmdline_fixed_string_t tx;
	cmdline_fixed_string_t tx_lfc_mode;
	uint32_t high_water;
	uint32_t low_water;
	uint16_t pause_time;
	uint16_t send_xon;
	uint8_t  port_id;
};

static void
cmd_link_flow_ctrl_set_parsed(void *parsed_result,
		       __attribute__((unused)) struct cmdline *cl,
		       __attribute__((unused)) void *data)
{
	struct cmd_link_flow_ctrl_set_result *res = parsed_result;
	struct rte_eth_fc_conf fc_conf;
	int rx_fc_enable, tx_fc_enable;
	int ret;

	/*
	 * Rx on/off, flow control is enabled/disabled on RX side. This can indicate
	 * the RTE_FC_TX_PAUSE, Transmit pause frame at the Rx side.
	 * Tx on/off, flow control is enabled/disabled on TX side. This can indicate
	 * the RTE_FC_RX_PAUSE, Respond to the pause frame at the Tx side.
	 */
	static enum rte_eth_fc_mode rx_tx_onoff_2_lfc_mode[2][2] = {
			{RTE_FC_NONE, RTE_FC_RX_PAUSE}, {RTE_FC_TX_PAUSE, RTE_FC_FULL}
	};

	rx_fc_enable = (!strcmp(res->rx_lfc_mode, "on")) ? 1 : 0;
	tx_fc_enable = (!strcmp(res->tx_lfc_mode, "on")) ? 1 : 0;

	fc_conf.mode       = rx_tx_onoff_2_lfc_mode[rx_fc_enable][tx_fc_enable];
	fc_conf.high_water = res->high_water;
	fc_conf.low_water  = res->low_water;
	fc_conf.pause_time = res->pause_time;
	fc_conf.send_xon   = res->send_xon;

	ret = rte_eth_dev_flow_ctrl_set(res->port_id, &fc_conf);
	if (ret != 0)
		printf("bad flow contrl parameter, return code = %d \n", ret);
}

cmdline_parse_token_string_t cmd_lfc_set_set =
	TOKEN_STRING_INITIALIZER(struct cmd_link_flow_ctrl_set_result,
				set, "set");
cmdline_parse_token_string_t cmd_lfc_set_flow_ctrl =
	TOKEN_STRING_INITIALIZER(struct cmd_link_flow_ctrl_set_result,
				flow_ctrl, "flow_ctrl");
cmdline_parse_token_string_t cmd_lfc_set_rx =
	TOKEN_STRING_INITIALIZER(struct cmd_link_flow_ctrl_set_result,
				rx, "rx");
cmdline_parse_token_string_t cmd_lfc_set_rx_mode =
	TOKEN_STRING_INITIALIZER(struct cmd_link_flow_ctrl_set_result,
				rx_lfc_mode, "on#off");
cmdline_parse_token_string_t cmd_lfc_set_tx =
	TOKEN_STRING_INITIALIZER(struct cmd_link_flow_ctrl_set_result,
				tx, "tx");
cmdline_parse_token_string_t cmd_lfc_set_tx_mode =
	TOKEN_STRING_INITIALIZER(struct cmd_link_flow_ctrl_set_result,
				tx_lfc_mode, "on#off");
cmdline_parse_token_num_t cmd_lfc_set_high_water =
	TOKEN_NUM_INITIALIZER(struct cmd_link_flow_ctrl_set_result,
				high_water, UINT32);
cmdline_parse_token_num_t cmd_lfc_set_low_water =
	TOKEN_NUM_INITIALIZER(struct cmd_link_flow_ctrl_set_result,
				low_water, UINT32);
cmdline_parse_token_num_t cmd_lfc_set_pause_time =
	TOKEN_NUM_INITIALIZER(struct cmd_link_flow_ctrl_set_result,
				pause_time, UINT16);
cmdline_parse_token_num_t cmd_lfc_set_send_xon =
	TOKEN_NUM_INITIALIZER(struct cmd_link_flow_ctrl_set_result,
				send_xon, UINT16);
cmdline_parse_token_num_t cmd_lfc_set_portid =
	TOKEN_NUM_INITIALIZER(struct cmd_link_flow_ctrl_set_result,
				port_id, UINT8);

cmdline_parse_inst_t cmd_link_flow_control_set = {
	.f = cmd_link_flow_ctrl_set_parsed,
	.data = NULL,
	.help_str = "Configure the Ethernet link flow control...",
	.tokens = {
		(void *)&cmd_lfc_set_set,
		(void *)&cmd_lfc_set_flow_ctrl,
		(void *)&cmd_lfc_set_rx,
		(void *)&cmd_lfc_set_rx_mode,
		(void *)&cmd_lfc_set_tx,
		(void *)&cmd_lfc_set_tx_mode,
		(void *)&cmd_lfc_set_high_water,
		(void *)&cmd_lfc_set_low_water,
		(void *)&cmd_lfc_set_pause_time,
		(void *)&cmd_lfc_set_send_xon,
		(void *)&cmd_lfc_set_portid,
		NULL,
	},
};

/* *** SETUP ETHERNET PIRORITY FLOW CONTROL *** */
struct cmd_priority_flow_ctrl_set_result {
	cmdline_fixed_string_t set;
	cmdline_fixed_string_t pfc_ctrl;
	cmdline_fixed_string_t rx;
	cmdline_fixed_string_t rx_pfc_mode;
	cmdline_fixed_string_t tx;
	cmdline_fixed_string_t tx_pfc_mode;
	uint32_t high_water;
	uint32_t low_water;
	uint16_t pause_time;
	uint8_t  priority;
	uint8_t  port_id;
};

static void
cmd_priority_flow_ctrl_set_parsed(void *parsed_result,
		       __attribute__((unused)) struct cmdline *cl,
		       __attribute__((unused)) void *data)
{
	struct cmd_priority_flow_ctrl_set_result *res = parsed_result;
	struct rte_eth_pfc_conf pfc_conf;
	int rx_fc_enable, tx_fc_enable;
	int ret;

	/*
	 * Rx on/off, flow control is enabled/disabled on RX side. This can indicate
	 * the RTE_FC_TX_PAUSE, Transmit pause frame at the Rx side.
	 * Tx on/off, flow control is enabled/disabled on TX side. This can indicate
	 * the RTE_FC_RX_PAUSE, Respond to the pause frame at the Tx side.
	 */
	static enum rte_eth_fc_mode rx_tx_onoff_2_pfc_mode[2][2] = {
			{RTE_FC_NONE, RTE_FC_RX_PAUSE}, {RTE_FC_TX_PAUSE, RTE_FC_FULL}
	};

	rx_fc_enable = (!strncmp(res->rx_pfc_mode, "on",2)) ? 1 : 0;
	tx_fc_enable = (!strncmp(res->tx_pfc_mode, "on",2)) ? 1 : 0;
	pfc_conf.fc.mode       = rx_tx_onoff_2_pfc_mode[rx_fc_enable][tx_fc_enable];
	pfc_conf.fc.high_water = res->high_water;
	pfc_conf.fc.low_water  = res->low_water;
	pfc_conf.fc.pause_time = res->pause_time;
	pfc_conf.priority      = res->priority;

	ret = rte_eth_dev_priority_flow_ctrl_set(res->port_id, &pfc_conf);
	if (ret != 0)
		printf("bad priority flow contrl parameter, return code = %d \n", ret);
}

cmdline_parse_token_string_t cmd_pfc_set_set =
	TOKEN_STRING_INITIALIZER(struct cmd_priority_flow_ctrl_set_result,
				set, "set");
cmdline_parse_token_string_t cmd_pfc_set_flow_ctrl =
	TOKEN_STRING_INITIALIZER(struct cmd_priority_flow_ctrl_set_result,
				pfc_ctrl, "pfc_ctrl");
cmdline_parse_token_string_t cmd_pfc_set_rx =
	TOKEN_STRING_INITIALIZER(struct cmd_priority_flow_ctrl_set_result,
				rx, "rx");
cmdline_parse_token_string_t cmd_pfc_set_rx_mode =
	TOKEN_STRING_INITIALIZER(struct cmd_priority_flow_ctrl_set_result,
				rx_pfc_mode, "on#off");
cmdline_parse_token_string_t cmd_pfc_set_tx =
	TOKEN_STRING_INITIALIZER(struct cmd_priority_flow_ctrl_set_result,
				tx, "tx");
cmdline_parse_token_string_t cmd_pfc_set_tx_mode =
	TOKEN_STRING_INITIALIZER(struct cmd_priority_flow_ctrl_set_result,
				tx_pfc_mode, "on#off");
cmdline_parse_token_num_t cmd_pfc_set_high_water =
	TOKEN_NUM_INITIALIZER(struct cmd_priority_flow_ctrl_set_result,
				high_water, UINT32);
cmdline_parse_token_num_t cmd_pfc_set_low_water =
	TOKEN_NUM_INITIALIZER(struct cmd_priority_flow_ctrl_set_result,
				low_water, UINT32);
cmdline_parse_token_num_t cmd_pfc_set_pause_time =
	TOKEN_NUM_INITIALIZER(struct cmd_priority_flow_ctrl_set_result,
				pause_time, UINT16);
cmdline_parse_token_num_t cmd_pfc_set_priority =
	TOKEN_NUM_INITIALIZER(struct cmd_priority_flow_ctrl_set_result,
				priority, UINT8);
cmdline_parse_token_num_t cmd_pfc_set_portid =
	TOKEN_NUM_INITIALIZER(struct cmd_priority_flow_ctrl_set_result,
				port_id, UINT8);

cmdline_parse_inst_t cmd_priority_flow_control_set = {
	.f = cmd_priority_flow_ctrl_set_parsed,
	.data = NULL,
	.help_str = "Configure the Ethernet priority flow control: set pfc_ctrl rx on|off\n\
			tx on|off high_water low_water pause_time priority port_id",
	.tokens = {
		(void *)&cmd_pfc_set_set,
		(void *)&cmd_pfc_set_flow_ctrl,
		(void *)&cmd_pfc_set_rx,
		(void *)&cmd_pfc_set_rx_mode,
		(void *)&cmd_pfc_set_tx,
		(void *)&cmd_pfc_set_tx_mode,
		(void *)&cmd_pfc_set_high_water,
		(void *)&cmd_pfc_set_low_water,
		(void *)&cmd_pfc_set_pause_time,
		(void *)&cmd_pfc_set_priority,
		(void *)&cmd_pfc_set_portid,
		NULL,
	},
};

/* *** RESET CONFIGURATION *** */
struct cmd_reset_result {
	cmdline_fixed_string_t reset;
	cmdline_fixed_string_t def;
};

static void cmd_reset_parsed(__attribute__((unused)) void *parsed_result,
			     struct cmdline *cl,
			     __attribute__((unused)) void *data)
{
	cmdline_printf(cl, "Reset to default forwarding configuration...\n");
	set_def_fwd_config();
}

cmdline_parse_token_string_t cmd_reset_set =
	TOKEN_STRING_INITIALIZER(struct cmd_reset_result, reset, "set");
cmdline_parse_token_string_t cmd_reset_def =
	TOKEN_STRING_INITIALIZER(struct cmd_reset_result, def,
				 "default");

cmdline_parse_inst_t cmd_reset = {
	.f = cmd_reset_parsed,
	.data = NULL,
	.help_str = "set default: reset default forwarding configuration",
	.tokens = {
		(void *)&cmd_reset_set,
		(void *)&cmd_reset_def,
		NULL,
	},
};

/* *** START FORWARDING *** */
struct cmd_start_result {
	cmdline_fixed_string_t start;
};

cmdline_parse_token_string_t cmd_start_start =
	TOKEN_STRING_INITIALIZER(struct cmd_start_result, start, "start");

static void cmd_start_parsed(__attribute__((unused)) void *parsed_result,
			     __attribute__((unused)) struct cmdline *cl,
			     __attribute__((unused)) void *data)
{
	start_packet_forwarding(0);
}

cmdline_parse_inst_t cmd_start = {
	.f = cmd_start_parsed,
	.data = NULL,
	.help_str = "start packet forwarding",
	.tokens = {
		(void *)&cmd_start_start,
		NULL,
	},
};

/* *** START FORWARDING WITH ONE TX BURST FIRST *** */
struct cmd_start_tx_first_result {
	cmdline_fixed_string_t start;
	cmdline_fixed_string_t tx_first;
};

static void
cmd_start_tx_first_parsed(__attribute__((unused)) void *parsed_result,
			  __attribute__((unused)) struct cmdline *cl,
			  __attribute__((unused)) void *data)
{
	start_packet_forwarding(1);
}

cmdline_parse_token_string_t cmd_start_tx_first_start =
	TOKEN_STRING_INITIALIZER(struct cmd_start_tx_first_result, start,
				 "start");
cmdline_parse_token_string_t cmd_start_tx_first_tx_first =
	TOKEN_STRING_INITIALIZER(struct cmd_start_tx_first_result,
				 tx_first, "tx_first");

cmdline_parse_inst_t cmd_start_tx_first = {
	.f = cmd_start_tx_first_parsed,
	.data = NULL,
	.help_str = "start packet forwarding, after sending 1 burst of packets",
	.tokens = {
		(void *)&cmd_start_tx_first_start,
		(void *)&cmd_start_tx_first_tx_first,
		NULL,
	},
};

/* *** SHOW CFG *** */
struct cmd_showcfg_result {
	cmdline_fixed_string_t show;
	cmdline_fixed_string_t cfg;
	cmdline_fixed_string_t what;
};

static void cmd_showcfg_parsed(void *parsed_result,
			       __attribute__((unused)) struct cmdline *cl,
			       __attribute__((unused)) void *data)
{
	struct cmd_showcfg_result *res = parsed_result;
	if (!strcmp(res->what, "rxtx"))
		rxtx_config_display();
	else if (!strcmp(res->what, "cores"))
		fwd_lcores_config_display();
	else if (!strcmp(res->what, "fwd"))
		fwd_config_display();
}

cmdline_parse_token_string_t cmd_showcfg_show =
	TOKEN_STRING_INITIALIZER(struct cmd_showcfg_result, show, "show");
cmdline_parse_token_string_t cmd_showcfg_port =
	TOKEN_STRING_INITIALIZER(struct cmd_showcfg_result, cfg, "config");
cmdline_parse_token_string_t cmd_showcfg_what =
	TOKEN_STRING_INITIALIZER(struct cmd_showcfg_result, what,
				 "rxtx#cores#fwd");

cmdline_parse_inst_t cmd_showcfg = {
	.f = cmd_showcfg_parsed,
	.data = NULL,
	.help_str = "show config rxtx|cores|fwd",
	.tokens = {
		(void *)&cmd_showcfg_show,
		(void *)&cmd_showcfg_port,
		(void *)&cmd_showcfg_what,
		NULL,
	},
};

/* *** SHOW ALL PORT INFO *** */
struct cmd_showportall_result {
	cmdline_fixed_string_t show;
	cmdline_fixed_string_t port;
	cmdline_fixed_string_t what;
	cmdline_fixed_string_t all;
};

static void cmd_showportall_parsed(void *parsed_result,
				__attribute__((unused)) struct cmdline *cl,
				__attribute__((unused)) void *data)
{
	portid_t i;

	struct cmd_showportall_result *res = parsed_result;
	if (!strcmp(res->show, "clear")) {
		if (!strcmp(res->what, "stats"))
			for (i = 0; i < nb_ports; i++)
				nic_stats_clear(i);
	} else if (!strcmp(res->what, "info"))
		for (i = 0; i < nb_ports; i++)
			port_infos_display(i);
	else if (!strcmp(res->what, "stats"))
		for (i = 0; i < nb_ports; i++)
			nic_stats_display(i);
	else if (!strcmp(res->what, "fdir"))
		for (i = 0; i < nb_ports; i++)
			fdir_get_infos(i);
	else if (!strcmp(res->what, "stat_qmap"))
		for (i = 0; i < nb_ports; i++)
			nic_stats_mapping_display(i);
}

cmdline_parse_token_string_t cmd_showportall_show =
	TOKEN_STRING_INITIALIZER(struct cmd_showportall_result, show,
				 "show#clear");
cmdline_parse_token_string_t cmd_showportall_port =
	TOKEN_STRING_INITIALIZER(struct cmd_showportall_result, port, "port");
cmdline_parse_token_string_t cmd_showportall_what =
	TOKEN_STRING_INITIALIZER(struct cmd_showportall_result, what,
				 "info#stats#fdir#stat_qmap");
cmdline_parse_token_string_t cmd_showportall_all =
	TOKEN_STRING_INITIALIZER(struct cmd_showportall_result, all, "all");
cmdline_parse_inst_t cmd_showportall = {
	.f = cmd_showportall_parsed,
	.data = NULL,
	.help_str = "show|clear port info|stats|fdir|stat_qmap all",
	.tokens = {
		(void *)&cmd_showportall_show,
		(void *)&cmd_showportall_port,
		(void *)&cmd_showportall_what,
		(void *)&cmd_showportall_all,
		NULL,
	},
};

/* *** SHOW PORT INFO *** */
struct cmd_showport_result {
	cmdline_fixed_string_t show;
	cmdline_fixed_string_t port;
	cmdline_fixed_string_t what;
	uint8_t portnum;
};

static void cmd_showport_parsed(void *parsed_result,
				__attribute__((unused)) struct cmdline *cl,
				__attribute__((unused)) void *data)
{
	struct cmd_showport_result *res = parsed_result;
	if (!strcmp(res->show, "clear")) {
		if (!strcmp(res->what, "stats"))
			nic_stats_clear(res->portnum);
	} else if (!strcmp(res->what, "info"))
		port_infos_display(res->portnum);
	else if (!strcmp(res->what, "stats"))
		nic_stats_display(res->portnum);
	else if (!strcmp(res->what, "fdir"))
		 fdir_get_infos(res->portnum);
	else if (!strcmp(res->what, "stat_qmap"))
		nic_stats_mapping_display(res->portnum);
}

cmdline_parse_token_string_t cmd_showport_show =
	TOKEN_STRING_INITIALIZER(struct cmd_showport_result, show,
				 "show#clear");
cmdline_parse_token_string_t cmd_showport_port =
	TOKEN_STRING_INITIALIZER(struct cmd_showport_result, port, "port");
cmdline_parse_token_string_t cmd_showport_what =
	TOKEN_STRING_INITIALIZER(struct cmd_showport_result, what,
				 "info#stats#fdir#stat_qmap");
cmdline_parse_token_num_t cmd_showport_portnum =
	TOKEN_NUM_INITIALIZER(struct cmd_showport_result, portnum, INT32);

cmdline_parse_inst_t cmd_showport = {
	.f = cmd_showport_parsed,
	.data = NULL,
	.help_str = "show|clear port info|stats|fdir|stat_qmap X (X = port number)",
	.tokens = {
		(void *)&cmd_showport_show,
		(void *)&cmd_showport_port,
		(void *)&cmd_showport_what,
		(void *)&cmd_showport_portnum,
		NULL,
	},
};

/* *** READ PORT REGISTER *** */
struct cmd_read_reg_result {
	cmdline_fixed_string_t read;
	cmdline_fixed_string_t reg;
	uint8_t port_id;
	uint32_t reg_off;
};

static void
cmd_read_reg_parsed(void *parsed_result,
		    __attribute__((unused)) struct cmdline *cl,
		    __attribute__((unused)) void *data)
{
	struct cmd_read_reg_result *res = parsed_result;
	port_reg_display(res->port_id, res->reg_off);
}

cmdline_parse_token_string_t cmd_read_reg_read =
	TOKEN_STRING_INITIALIZER(struct cmd_read_reg_result, read, "read");
cmdline_parse_token_string_t cmd_read_reg_reg =
	TOKEN_STRING_INITIALIZER(struct cmd_read_reg_result, reg, "reg");
cmdline_parse_token_num_t cmd_read_reg_port_id =
	TOKEN_NUM_INITIALIZER(struct cmd_read_reg_result, port_id, UINT8);
cmdline_parse_token_num_t cmd_read_reg_reg_off =
	TOKEN_NUM_INITIALIZER(struct cmd_read_reg_result, reg_off, UINT32);

cmdline_parse_inst_t cmd_read_reg = {
	.f = cmd_read_reg_parsed,
	.data = NULL,
	.help_str = "read reg port_id reg_off",
	.tokens = {
		(void *)&cmd_read_reg_read,
		(void *)&cmd_read_reg_reg,
		(void *)&cmd_read_reg_port_id,
		(void *)&cmd_read_reg_reg_off,
		NULL,
	},
};

/* *** READ PORT REGISTER BIT FIELD *** */
struct cmd_read_reg_bit_field_result {
	cmdline_fixed_string_t read;
	cmdline_fixed_string_t regfield;
	uint8_t port_id;
	uint32_t reg_off;
	uint8_t bit1_pos;
	uint8_t bit2_pos;
};

static void
cmd_read_reg_bit_field_parsed(void *parsed_result,
			      __attribute__((unused)) struct cmdline *cl,
			      __attribute__((unused)) void *data)
{
	struct cmd_read_reg_bit_field_result *res = parsed_result;
	port_reg_bit_field_display(res->port_id, res->reg_off,
				   res->bit1_pos, res->bit2_pos);
}

cmdline_parse_token_string_t cmd_read_reg_bit_field_read =
	TOKEN_STRING_INITIALIZER(struct cmd_read_reg_bit_field_result, read,
				 "read");
cmdline_parse_token_string_t cmd_read_reg_bit_field_regfield =
	TOKEN_STRING_INITIALIZER(struct cmd_read_reg_bit_field_result,
				 regfield, "regfield");
cmdline_parse_token_num_t cmd_read_reg_bit_field_port_id =
	TOKEN_NUM_INITIALIZER(struct cmd_read_reg_bit_field_result, port_id,
			      UINT8);
cmdline_parse_token_num_t cmd_read_reg_bit_field_reg_off =
	TOKEN_NUM_INITIALIZER(struct cmd_read_reg_bit_field_result, reg_off,
			      UINT32);
cmdline_parse_token_num_t cmd_read_reg_bit_field_bit1_pos =
	TOKEN_NUM_INITIALIZER(struct cmd_read_reg_bit_field_result, bit1_pos,
			      UINT8);
cmdline_parse_token_num_t cmd_read_reg_bit_field_bit2_pos =
	TOKEN_NUM_INITIALIZER(struct cmd_read_reg_bit_field_result, bit2_pos,
			      UINT8);

cmdline_parse_inst_t cmd_read_reg_bit_field = {
	.f = cmd_read_reg_bit_field_parsed,
	.data = NULL,
	.help_str = "read regfield port_id reg_off bit_x bit_y "
	"(read register bit field between bit_x and bit_y included)",
	.tokens = {
		(void *)&cmd_read_reg_bit_field_read,
		(void *)&cmd_read_reg_bit_field_regfield,
		(void *)&cmd_read_reg_bit_field_port_id,
		(void *)&cmd_read_reg_bit_field_reg_off,
		(void *)&cmd_read_reg_bit_field_bit1_pos,
		(void *)&cmd_read_reg_bit_field_bit2_pos,
		NULL,
	},
};

/* *** READ PORT REGISTER BIT *** */
struct cmd_read_reg_bit_result {
	cmdline_fixed_string_t read;
	cmdline_fixed_string_t regbit;
	uint8_t port_id;
	uint32_t reg_off;
	uint8_t bit_pos;
};

static void
cmd_read_reg_bit_parsed(void *parsed_result,
			__attribute__((unused)) struct cmdline *cl,
			__attribute__((unused)) void *data)
{
	struct cmd_read_reg_bit_result *res = parsed_result;
	port_reg_bit_display(res->port_id, res->reg_off, res->bit_pos);
}

cmdline_parse_token_string_t cmd_read_reg_bit_read =
	TOKEN_STRING_INITIALIZER(struct cmd_read_reg_bit_result, read, "read");
cmdline_parse_token_string_t cmd_read_reg_bit_regbit =
	TOKEN_STRING_INITIALIZER(struct cmd_read_reg_bit_result,
				 regbit, "regbit");
cmdline_parse_token_num_t cmd_read_reg_bit_port_id =
	TOKEN_NUM_INITIALIZER(struct cmd_read_reg_bit_result, port_id, UINT8);
cmdline_parse_token_num_t cmd_read_reg_bit_reg_off =
	TOKEN_NUM_INITIALIZER(struct cmd_read_reg_bit_result, reg_off, UINT32);
cmdline_parse_token_num_t cmd_read_reg_bit_bit_pos =
	TOKEN_NUM_INITIALIZER(struct cmd_read_reg_bit_result, bit_pos, UINT8);

cmdline_parse_inst_t cmd_read_reg_bit = {
	.f = cmd_read_reg_bit_parsed,
	.data = NULL,
	.help_str = "read regbit port_id reg_off bit_x (0 <= bit_x <= 31)",
	.tokens = {
		(void *)&cmd_read_reg_bit_read,
		(void *)&cmd_read_reg_bit_regbit,
		(void *)&cmd_read_reg_bit_port_id,
		(void *)&cmd_read_reg_bit_reg_off,
		(void *)&cmd_read_reg_bit_bit_pos,
		NULL,
	},
};

/* *** WRITE PORT REGISTER *** */
struct cmd_write_reg_result {
	cmdline_fixed_string_t write;
	cmdline_fixed_string_t reg;
	uint8_t port_id;
	uint32_t reg_off;
	uint32_t value;
};

static void
cmd_write_reg_parsed(void *parsed_result,
		     __attribute__((unused)) struct cmdline *cl,
		     __attribute__((unused)) void *data)
{
	struct cmd_write_reg_result *res = parsed_result;
	port_reg_set(res->port_id, res->reg_off, res->value);
}

cmdline_parse_token_string_t cmd_write_reg_write =
	TOKEN_STRING_INITIALIZER(struct cmd_write_reg_result, write, "write");
cmdline_parse_token_string_t cmd_write_reg_reg =
	TOKEN_STRING_INITIALIZER(struct cmd_write_reg_result, reg, "reg");
cmdline_parse_token_num_t cmd_write_reg_port_id =
	TOKEN_NUM_INITIALIZER(struct cmd_write_reg_result, port_id, UINT8);
cmdline_parse_token_num_t cmd_write_reg_reg_off =
	TOKEN_NUM_INITIALIZER(struct cmd_write_reg_result, reg_off, UINT32);
cmdline_parse_token_num_t cmd_write_reg_value =
	TOKEN_NUM_INITIALIZER(struct cmd_write_reg_result, value, UINT32);

cmdline_parse_inst_t cmd_write_reg = {
	.f = cmd_write_reg_parsed,
	.data = NULL,
	.help_str = "write reg port_id reg_off reg_value",
	.tokens = {
		(void *)&cmd_write_reg_write,
		(void *)&cmd_write_reg_reg,
		(void *)&cmd_write_reg_port_id,
		(void *)&cmd_write_reg_reg_off,
		(void *)&cmd_write_reg_value,
		NULL,
	},
};

/* *** WRITE PORT REGISTER BIT FIELD *** */
struct cmd_write_reg_bit_field_result {
	cmdline_fixed_string_t write;
	cmdline_fixed_string_t regfield;
	uint8_t port_id;
	uint32_t reg_off;
	uint8_t bit1_pos;
	uint8_t bit2_pos;
	uint32_t value;
};

static void
cmd_write_reg_bit_field_parsed(void *parsed_result,
			       __attribute__((unused)) struct cmdline *cl,
			       __attribute__((unused)) void *data)
{
	struct cmd_write_reg_bit_field_result *res = parsed_result;
	port_reg_bit_field_set(res->port_id, res->reg_off,
			  res->bit1_pos, res->bit2_pos, res->value);
}

cmdline_parse_token_string_t cmd_write_reg_bit_field_write =
	TOKEN_STRING_INITIALIZER(struct cmd_write_reg_bit_field_result, write,
				 "write");
cmdline_parse_token_string_t cmd_write_reg_bit_field_regfield =
	TOKEN_STRING_INITIALIZER(struct cmd_write_reg_bit_field_result,
				 regfield, "regfield");
cmdline_parse_token_num_t cmd_write_reg_bit_field_port_id =
	TOKEN_NUM_INITIALIZER(struct cmd_write_reg_bit_field_result, port_id,
			      UINT8);
cmdline_parse_token_num_t cmd_write_reg_bit_field_reg_off =
	TOKEN_NUM_INITIALIZER(struct cmd_write_reg_bit_field_result, reg_off,
			      UINT32);
cmdline_parse_token_num_t cmd_write_reg_bit_field_bit1_pos =
	TOKEN_NUM_INITIALIZER(struct cmd_write_reg_bit_field_result, bit1_pos,
			      UINT8);
cmdline_parse_token_num_t cmd_write_reg_bit_field_bit2_pos =
	TOKEN_NUM_INITIALIZER(struct cmd_write_reg_bit_field_result, bit2_pos,
			      UINT8);
cmdline_parse_token_num_t cmd_write_reg_bit_field_value =
	TOKEN_NUM_INITIALIZER(struct cmd_write_reg_bit_field_result, value,
			      UINT32);

cmdline_parse_inst_t cmd_write_reg_bit_field = {
	.f = cmd_write_reg_bit_field_parsed,
	.data = NULL,
	.help_str = "write regfield port_id reg_off bit_x bit_y reg_value"
	"(set register bit field between bit_x and bit_y included)",
	.tokens = {
		(void *)&cmd_write_reg_bit_field_write,
		(void *)&cmd_write_reg_bit_field_regfield,
		(void *)&cmd_write_reg_bit_field_port_id,
		(void *)&cmd_write_reg_bit_field_reg_off,
		(void *)&cmd_write_reg_bit_field_bit1_pos,
		(void *)&cmd_write_reg_bit_field_bit2_pos,
		(void *)&cmd_write_reg_bit_field_value,
		NULL,
	},
};

/* *** WRITE PORT REGISTER BIT *** */
struct cmd_write_reg_bit_result {
	cmdline_fixed_string_t write;
	cmdline_fixed_string_t regbit;
	uint8_t port_id;
	uint32_t reg_off;
	uint8_t bit_pos;
	uint8_t value;
};

static void
cmd_write_reg_bit_parsed(void *parsed_result,
			 __attribute__((unused)) struct cmdline *cl,
			 __attribute__((unused)) void *data)
{
	struct cmd_write_reg_bit_result *res = parsed_result;
	port_reg_bit_set(res->port_id, res->reg_off, res->bit_pos, res->value);
}

cmdline_parse_token_string_t cmd_write_reg_bit_write =
	TOKEN_STRING_INITIALIZER(struct cmd_write_reg_bit_result, write,
				 "write");
cmdline_parse_token_string_t cmd_write_reg_bit_regbit =
	TOKEN_STRING_INITIALIZER(struct cmd_write_reg_bit_result,
				 regbit, "regbit");
cmdline_parse_token_num_t cmd_write_reg_bit_port_id =
	TOKEN_NUM_INITIALIZER(struct cmd_write_reg_bit_result, port_id, UINT8);
cmdline_parse_token_num_t cmd_write_reg_bit_reg_off =
	TOKEN_NUM_INITIALIZER(struct cmd_write_reg_bit_result, reg_off, UINT32);
cmdline_parse_token_num_t cmd_write_reg_bit_bit_pos =
	TOKEN_NUM_INITIALIZER(struct cmd_write_reg_bit_result, bit_pos, UINT8);
cmdline_parse_token_num_t cmd_write_reg_bit_value =
	TOKEN_NUM_INITIALIZER(struct cmd_write_reg_bit_result, value, UINT8);

cmdline_parse_inst_t cmd_write_reg_bit = {
	.f = cmd_write_reg_bit_parsed,
	.data = NULL,
	.help_str = "write regbit port_id reg_off bit_x 0/1 (0 <= bit_x <= 31)",
	.tokens = {
		(void *)&cmd_write_reg_bit_write,
		(void *)&cmd_write_reg_bit_regbit,
		(void *)&cmd_write_reg_bit_port_id,
		(void *)&cmd_write_reg_bit_reg_off,
		(void *)&cmd_write_reg_bit_bit_pos,
		(void *)&cmd_write_reg_bit_value,
		NULL,
	},
};

/* *** READ A RING DESCRIPTOR OF A PORT RX/TX QUEUE *** */
struct cmd_read_rxd_txd_result {
	cmdline_fixed_string_t read;
	cmdline_fixed_string_t rxd_txd;
	uint8_t port_id;
	uint16_t queue_id;
	uint16_t desc_id;
};

static void
cmd_read_rxd_txd_parsed(void *parsed_result,
			__attribute__((unused)) struct cmdline *cl,
			__attribute__((unused)) void *data)
{
	struct cmd_read_rxd_txd_result *res = parsed_result;

	if (!strcmp(res->rxd_txd, "rxd"))
		rx_ring_desc_display(res->port_id, res->queue_id, res->desc_id);
	else if (!strcmp(res->rxd_txd, "txd"))
		tx_ring_desc_display(res->port_id, res->queue_id, res->desc_id);
}

cmdline_parse_token_string_t cmd_read_rxd_txd_read =
	TOKEN_STRING_INITIALIZER(struct cmd_read_rxd_txd_result, read, "read");
cmdline_parse_token_string_t cmd_read_rxd_txd_rxd_txd =
	TOKEN_STRING_INITIALIZER(struct cmd_read_rxd_txd_result, rxd_txd,
				 "rxd#txd");
cmdline_parse_token_num_t cmd_read_rxd_txd_port_id =
	TOKEN_NUM_INITIALIZER(struct cmd_read_rxd_txd_result, port_id, UINT8);
cmdline_parse_token_num_t cmd_read_rxd_txd_queue_id =
	TOKEN_NUM_INITIALIZER(struct cmd_read_rxd_txd_result, queue_id, UINT16);
cmdline_parse_token_num_t cmd_read_rxd_txd_desc_id =
	TOKEN_NUM_INITIALIZER(struct cmd_read_rxd_txd_result, desc_id, UINT16);

cmdline_parse_inst_t cmd_read_rxd_txd = {
	.f = cmd_read_rxd_txd_parsed,
	.data = NULL,
	.help_str = "read rxd|txd port_id queue_id rxd_id",
	.tokens = {
		(void *)&cmd_read_rxd_txd_read,
		(void *)&cmd_read_rxd_txd_rxd_txd,
		(void *)&cmd_read_rxd_txd_port_id,
		(void *)&cmd_read_rxd_txd_queue_id,
		(void *)&cmd_read_rxd_txd_desc_id,
		NULL,
	},
};

/* *** QUIT *** */
struct cmd_quit_result {
	cmdline_fixed_string_t quit;
};

static void cmd_quit_parsed(__attribute__((unused)) void *parsed_result,
			    struct cmdline *cl,
			    __attribute__((unused)) void *data)
{
	pmd_test_exit();
	cmdline_quit(cl);
}

cmdline_parse_token_string_t cmd_quit_quit =
	TOKEN_STRING_INITIALIZER(struct cmd_quit_result, quit, "quit");

cmdline_parse_inst_t cmd_quit = {
	.f = cmd_quit_parsed,
	.data = NULL,
	.help_str = "exit application",
	.tokens = {
		(void *)&cmd_quit_quit,
		NULL,
	},
};

/* *** ADD/REMOVE MAC ADDRESS FROM A PORT *** */
struct cmd_mac_addr_result {
	cmdline_fixed_string_t mac_addr_cmd;
	cmdline_fixed_string_t what;
	uint8_t port_num;
	struct ether_addr address;
};

static void cmd_mac_addr_parsed(void *parsed_result,
		__attribute__((unused)) struct cmdline *cl,
		__attribute__((unused)) void *data)
{
	struct cmd_mac_addr_result *res = parsed_result;
	int ret;

	if (strcmp(res->what, "add") == 0)
		ret = rte_eth_dev_mac_addr_add(res->port_num, &res->address, 0);
	else
		ret = rte_eth_dev_mac_addr_remove(res->port_num, &res->address);

	/* check the return value and print it if is < 0 */
	if(ret < 0)
		printf("mac_addr_cmd error: (%s)\n", strerror(-ret));

}

cmdline_parse_token_string_t cmd_mac_addr_cmd =
	TOKEN_STRING_INITIALIZER(struct cmd_mac_addr_result, mac_addr_cmd,
				"mac_addr");
cmdline_parse_token_string_t cmd_mac_addr_what =
	TOKEN_STRING_INITIALIZER(struct cmd_mac_addr_result, what,
				"add#remove");
cmdline_parse_token_num_t cmd_mac_addr_portnum =
		TOKEN_NUM_INITIALIZER(struct cmd_mac_addr_result, port_num, UINT8);
cmdline_parse_token_etheraddr_t cmd_mac_addr_addr =
		TOKEN_ETHERADDR_INITIALIZER(struct cmd_mac_addr_result, address);

cmdline_parse_inst_t cmd_mac_addr = {
	.f = cmd_mac_addr_parsed,
	.data = (void *)0,
	.help_str = "mac_addr add|remove X <address>: "
			"add/remove MAC address on port X",
	.tokens = {
		(void *)&cmd_mac_addr_cmd,
		(void *)&cmd_mac_addr_what,
		(void *)&cmd_mac_addr_portnum,
		(void *)&cmd_mac_addr_addr,
		NULL,
	},
};


/* *** CONFIGURE QUEUE STATS COUNTER MAPPINGS *** */
struct cmd_set_qmap_result {
	cmdline_fixed_string_t set;
	cmdline_fixed_string_t qmap;
	cmdline_fixed_string_t what;
	uint8_t port_id;
	uint16_t queue_id;
	uint8_t map_value;
};

static void
cmd_set_qmap_parsed(void *parsed_result,
		       __attribute__((unused)) struct cmdline *cl,
		       __attribute__((unused)) void *data)
{
	struct cmd_set_qmap_result *res = parsed_result;
	int is_rx = (strcmp(res->what, "tx") == 0) ? 0 : 1;

	set_qmap(res->port_id, (uint8_t)is_rx, res->queue_id, res->map_value);
}

cmdline_parse_token_string_t cmd_setqmap_set =
	TOKEN_STRING_INITIALIZER(struct cmd_set_qmap_result,
				 set, "set");
cmdline_parse_token_string_t cmd_setqmap_qmap =
	TOKEN_STRING_INITIALIZER(struct cmd_set_qmap_result,
				 qmap, "stat_qmap");
cmdline_parse_token_string_t cmd_setqmap_what =
	TOKEN_STRING_INITIALIZER(struct cmd_set_qmap_result,
				 what, "tx#rx");
cmdline_parse_token_num_t cmd_setqmap_portid =
	TOKEN_NUM_INITIALIZER(struct cmd_set_qmap_result,
			      port_id, UINT8);
cmdline_parse_token_num_t cmd_setqmap_queueid =
	TOKEN_NUM_INITIALIZER(struct cmd_set_qmap_result,
			      queue_id, UINT16);
cmdline_parse_token_num_t cmd_setqmap_mapvalue =
	TOKEN_NUM_INITIALIZER(struct cmd_set_qmap_result,
			      map_value, UINT8);

cmdline_parse_inst_t cmd_set_qmap = {
	.f = cmd_set_qmap_parsed,
	.data = NULL,
	.help_str = "Set statistics mapping value on tx|rx queue_id of port_id",
	.tokens = {
		(void *)&cmd_setqmap_set,
		(void *)&cmd_setqmap_qmap,
		(void *)&cmd_setqmap_what,
		(void *)&cmd_setqmap_portid,
		(void *)&cmd_setqmap_queueid,
		(void *)&cmd_setqmap_mapvalue,
		NULL,
	},
};

/* ******************************************************************************** */

/* list of instructions */
cmdline_parse_ctx_t main_ctx[] = {
	(cmdline_parse_inst_t *)&cmd_help,
	(cmdline_parse_inst_t *)&cmd_quit,
	(cmdline_parse_inst_t *)&cmd_showport,
	(cmdline_parse_inst_t *)&cmd_showportall,
	(cmdline_parse_inst_t *)&cmd_showcfg,
	(cmdline_parse_inst_t *)&cmd_start,
	(cmdline_parse_inst_t *)&cmd_start_tx_first,
	(cmdline_parse_inst_t *)&cmd_reset,
	(cmdline_parse_inst_t *)&cmd_set_numbers,
	(cmdline_parse_inst_t *)&cmd_set_txpkts,
	(cmdline_parse_inst_t *)&cmd_set_fwd_list,
	(cmdline_parse_inst_t *)&cmd_set_fwd_mask,
	(cmdline_parse_inst_t *)&cmd_set_fwd_mode,
	(cmdline_parse_inst_t *)&cmd_set_promisc_mode_one,
	(cmdline_parse_inst_t *)&cmd_set_promisc_mode_all,
	(cmdline_parse_inst_t *)&cmd_set_allmulti_mode_one,
	(cmdline_parse_inst_t *)&cmd_set_allmulti_mode_all,
	(cmdline_parse_inst_t *)&cmd_vlan_offload,
	(cmdline_parse_inst_t *)&cmd_vlan_tpid,
	(cmdline_parse_inst_t *)&cmd_rx_vlan_filter_all,
	(cmdline_parse_inst_t *)&cmd_rx_vlan_filter,
	(cmdline_parse_inst_t *)&cmd_tx_vlan_set,
	(cmdline_parse_inst_t *)&cmd_tx_vlan_reset,
	(cmdline_parse_inst_t *)&cmd_tx_cksum_set,
	(cmdline_parse_inst_t *)&cmd_link_flow_control_set,
	(cmdline_parse_inst_t *)&cmd_priority_flow_control_set,
	(cmdline_parse_inst_t *)&cmd_config_dcb,
	(cmdline_parse_inst_t *)&cmd_read_reg,
	(cmdline_parse_inst_t *)&cmd_read_reg_bit_field,
	(cmdline_parse_inst_t *)&cmd_read_reg_bit,
	(cmdline_parse_inst_t *)&cmd_write_reg,
	(cmdline_parse_inst_t *)&cmd_write_reg_bit_field,
	(cmdline_parse_inst_t *)&cmd_write_reg_bit,
	(cmdline_parse_inst_t *)&cmd_read_rxd_txd,
	(cmdline_parse_inst_t *)&cmd_add_signature_filter,
	(cmdline_parse_inst_t *)&cmd_upd_signature_filter,
	(cmdline_parse_inst_t *)&cmd_rm_signature_filter,
	(cmdline_parse_inst_t *)&cmd_add_perfect_filter,
	(cmdline_parse_inst_t *)&cmd_upd_perfect_filter,
	(cmdline_parse_inst_t *)&cmd_rm_perfect_filter,
	(cmdline_parse_inst_t *)&cmd_set_masks_filter,
	(cmdline_parse_inst_t *)&cmd_set_ipv6_masks_filter,
	(cmdline_parse_inst_t *)&cmd_stop,
	(cmdline_parse_inst_t *)&cmd_mac_addr,
	(cmdline_parse_inst_t *)&cmd_set_qmap,
	(cmdline_parse_inst_t *)&cmd_operate_port,
	(cmdline_parse_inst_t *)&cmd_operate_specific_port,
	(cmdline_parse_inst_t *)&cmd_config_speed_all,
	(cmdline_parse_inst_t *)&cmd_config_speed_specific,
	(cmdline_parse_inst_t *)&cmd_config_rx_tx,
	(cmdline_parse_inst_t *)&cmd_config_max_pkt_len,
	(cmdline_parse_inst_t *)&cmd_config_rx_mode_flag,
	(cmdline_parse_inst_t *)&cmd_config_rss,
	(cmdline_parse_inst_t *)&cmd_config_burst,
	(cmdline_parse_inst_t *)&cmd_config_thresh,
	(cmdline_parse_inst_t *)&cmd_config_threshold,
	NULL,
};

/* prompt function, called from main on MASTER lcore */
void
prompt(void)
{
	struct cmdline *cl;

	cl = cmdline_stdin_new(main_ctx, "testpmd> ");
	if (cl == NULL) {
		return;
	}
	cmdline_interact(cl);
	cmdline_stdin_exit(cl);
}

static void
cmd_reconfig_device_queue(portid_t id, uint8_t dev, uint8_t queue)
{
	if (id < nb_ports) {
		/* check if need_reconfig has been set to 1 */
		if (ports[id].need_reconfig == 0)
			ports[id].need_reconfig = dev;
		/* check if need_reconfig_queues has been set to 1 */
		if (ports[id].need_reconfig_queues == 0)
			ports[id].need_reconfig_queues = queue;
	} else {
		portid_t pid;

		for (pid = 0; pid < nb_ports; pid++) {
			/* check if need_reconfig has been set to 1 */
			if (ports[pid].need_reconfig == 0)
				ports[pid].need_reconfig = dev;
			/* check if need_reconfig_queues has been set to 1 */
			if (ports[pid].need_reconfig_queues == 0)
				ports[pid].need_reconfig_queues = queue;
		}
	}
}
