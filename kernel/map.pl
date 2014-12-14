#!/usr/bin/perl
# (c) Copyright Paul Campbell paul@taniwha.com 2013
# 
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) version 3, or any
# later version accepted by Paul Campbell , who shall
# act as a proxy defined in Section 6 of version 3 of the license.
# 
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
# 
# You should have received a copy of the GNU Lesser General Public 
# License along with this library.  If not, see <http://www.gnu.org/licenses/>.
#


while (<>) {

# BITS
#  rx_crypto
#  rx_broadcast
#  key_down
#  _limit
	if (/^C:.*([0-9A-F][0-9A-F]*).*s_BSEG .*$/ ) {
		$bs = hex($1);
	}
	if (/^C:.*([0-9A-F][0-9A-F]*).*l_BSEG .*$/ ) {
		$bl = hex($1);
	}

# DATA
#  rtx_key
	if (/^ *([0-9A-F][0-9A-F]*) *_rtx_key .*$/ ) {
		printf STDOUT "-g _rtx_key=0x%x\n", hex($1);
	}
#  rx_mac
	if (/^ *([0-9A-F][0-9A-F]*) *_rx_mac .*$/ ) {
		printf STDOUT "-g _rx_mac=0x%x\n", hex($1);
	}
#  rx_len
	if (/^ *([0-9A-F][0-9A-F]*) *_rx_len .*$/ ) {
		printf STDOUT "-g _rx_len=0x%x\n", hex($1);
	}
#  rx_packet
	if (/^ *([0-9A-F][0-9A-F]*) *_rx_packet .*$/ ) {
		printf STDOUT "-g _rx_packet=0x%x\n", hex($1);
	}
#  _limit
	if (/^ *([0-9A-F][0-9A-F]*) *_data_end .*$/ ) {
		printf STDOUT "-b DSEG=0x%x\n", hex($1);
	}


# CODE
#  leds_rgb
	if (/^C: *([0-9A-F][0-9A-F]*) *_leds_rgb .*$/ ) {
		printf STDOUT "-g _leds_rgb=0x%x\n", hex($1);
	}
#  leds_off
	if (/^C: *([0-9A-F][0-9A-F]*) *_leds_off .*$/ ) {
		printf STDOUT "-g _leds_off=0x%x\n", hex($1);
	}
#  rf_receive_on
	if (/^C: *([0-9A-F][0-9A-F]*) *_rf_receive_on .*$/ ) {
		printf STDOUT "-g _rf_receive_on=0x%x\n", hex($1);
	}
#  rf_receive_off
	if (/^C: *([0-9A-F][0-9A-F]*) *_rf_receive_off .*$/ ) {
		printf STDOUT "-g _rf_receive_off=0x%x\n", hex($1);
	}
#  rf_set_channel
	if (/^C: *([0-9A-F][0-9A-F]*) *_rf_set_channel .*$/ ) {
		printf STDOUT "-g _rf_set_channel=0x%x\n", hex($1);
	}
#  rf_set_key_c
	if (/^C: *([0-9A-F][0-9A-F]*) *_rf_set_key_c .*$/ ) {
		printf STDOUT "-g _rf_set_key_c=0x%x\n", hex($1);
	}
#  rf_set_key
	if (/^C: *([0-9A-F][0-9A-F]*) *_rf_set_key .*$/ ) {
		printf STDOUT "-g _rf_set_key=0x%x\n", hex($1);
	}
#  rf_set_mac
	if (/^C: *([0-9A-F][0-9A-F]*) *_rf_set_mac .*$/ ) {
		printf STDOUT "-g _rf_set_mac=0x%x\n", hex($1);
	}
#  rf_send
	if (/^C: *([0-9A-F][0-9A-F]*) *_rf_send .*$/ ) {
		printf STDOUT "-g _rf_send=0x%x\n", hex($1);
	}
	if (/^D: *([0-9A-F][0-9A-F]*) *_rf_send_PARM_2 .*$/ ) {
		printf STDOUT "-g _rf_send_PARM_2=0x%x\n", hex($1);
	}
	if (/^D: *([0-9A-F][0-9A-F]*) *_rf_send_PARM_3 .*$/ ) {
		printf STDOUT "-g _rf_send_PARM_3=0x%x\n", hex($1);
	}
	if (/^D: *([0-9A-F][0-9A-F]*) *_rf_send_PARM_4 .*$/ ) {
		printf STDOUT "-g _rf_send_PARM_4=0x%x\n", hex($1);
	}
#  rf_set_transmit_power
	if (/^C: *([0-9A-F][0-9A-F]*) *_rf_set_transmit_power .*$/ ) {
		printf STDOUT "-g _rf_set_transmit_power=0x%x\n", hex($1);
	}
#  rf_set_promiscuous
	if (/^C: *([0-9A-F][0-9A-F]*) *_rf_set_promiscuous .*$/ ) {
		printf STDOUT "-g _rf_set_promiscuous=0x%x\n", hex($1);
	}
#  rf_set_raw
	if (/^C: *([0-9A-F][0-9A-F]*) *_rf_set_raw .*$/ ) {
		printf STDOUT "-g _rf_set_raw=0x%x\n", hex($1);
	}
#  daylight
	if (/^C: *([0-9A-F][0-9A-F]*) *_daylight .*$/ ) {
		printf STDOUT "-g _daylight=0x%x\n", hex($1);
	}
#  keys_on
	if (/^C: *([0-9A-F][0-9A-F]*) *_keys_on .*$/ ) {
		printf STDOUT "-g _keys_on=0x%x\n", hex($1);
	}
#  keys_off
	if (/^C: *([0-9A-F][0-9A-F]*) *_keys_off .*$/ ) {
		printf STDOUT "-g _keys_off=0x%x\n", hex($1);
	}
#  uart_init
	if (/^C: *([0-9A-F][0-9A-F]*) *_uart_init .*$/ ) {
		printf STDOUT "-g _uart_init=0x%x\n", hex($1);
	}
#  putchar
	if (/^C: *([0-9A-F][0-9A-F]*) *_putchar .*$/ ) {
		printf STDOUT "-g _putchar=0x%x\n", hex($1);
	}
#  system_attributes
	if (/^C: *([0-9A-F][0-9A-F]*) *_system_attributes .*$/ ) {
		printf STDOUT "-g _system_attributes=0x%x\n", hex($1);
	}
#  putstr
	if (/^C: *([0-9A-F][0-9A-F]*) *_putstr .*$/ ) {
		printf STDOUT "-g _putstr=0x%x\n", hex($1);
	}
#  puthex
	if (/^C: *([0-9A-F][0-9A-F]*) *_puthex .*$/ ) {
		printf STDOUT "-g _puthex=0x%x\n", hex($1);
	}
#  queue_task
	if (/^C: *([0-9A-F][0-9A-F]*) *_queue_task .*$/ ) {
		printf STDOUT "-g _queue_task=0x%x\n", hex($1);
	}
	if (/^D: *([0-9A-F][0-9A-F]*) *_queue_task_PARM_2 .*$/ ) {
		printf STDOUT "-g _queue_task_PARM_2=0x%x\n", hex($1);
	}
#  cancel_task
	if (/^C: *([0-9A-F][0-9A-F]*) *_cancel_task .*$/ ) {
		printf STDOUT "-g _cancel_task=0x%x\n", hex($1);
	}
#  wait_us
	if (/^C: *([0-9A-F][0-9A-F]*) *_wait_us .*$/ ) {
		printf STDOUT "-g _wait_us=0x%x\n", hex($1);
	}
#  _limit

# PDATA
#  key
	if (/^D: *([0-9A-F][0-9A-F]*) *_key .*$/ ) {
		printf STDOUT "-g _key=0x%x\n", hex($1);
	}
#  uart_rx_0_vect
	if (/^D: *([0-9A-F][0-9A-F]*) *_uart_rx_0_vect .*$/ ) {
		printf STDOUT "-g _uart_rx_0_vect=0x%x\n", hex($1);
	}
#  uart_rx_1_vect
	if (/^D: *([0-9A-F][0-9A-F]*) *_uart_rx_1_vect .*$/ ) {
		printf STDOUT "-g _uart_rx_1_vect=0x%x\n", hex($1);
	}
#  uart_tx_0_vect
	if (/^D: *([0-9A-F][0-9A-F]*) *_uart_tx_0_vect .*$/ ) {
		printf STDOUT "-g _uart_tx_0_vect=0x%x\n", hex($1);
	}
#  uart_tx_1_vect
	if (/^D: *([0-9A-F][0-9A-F]*) *_uart_tx_1_vect .*$/ ) {
		printf STDOUT "-g _uart_tx_1_vect=0x%x\n", hex($1);
	}
#  p0_vect
	if (/^D: *([0-9A-F][0-9A-F]*) *_p0_vect .*$/ ) {
		printf STDOUT "-g _p0_vect=0x%x\n", hex($1);
	}
#  p1_vect
	if (/^D: *([0-9A-F][0-9A-F]*) *_p1_vect .*$/ ) {
		printf STDOUT "-g _p1_vect=0x%x\n", hex($1);
	}
#  p2_vect
	if (/^D: *([0-9A-F][0-9A-F]*) *_p2_vect .*$/ ) {
		printf STDOUT "-g _p2_vect=0x%x\n", hex($1);
	}
#  t2_vect
	if (/^D: *([0-9A-F][0-9A-F]*) *_t2_vect .*$/ ) {
		printf STDOUT "-g _t2_vect=0x%x\n", hex($1);
	}
#  t3_vect
	if (/^D: *([0-9A-F][0-9A-F]*) *_t3_vect .*$/ ) {
		printf STDOUT "-g _t3_vect=0x%x\n", hex($1);
	}
#  t4_vect
	if (/^D: *([0-9A-F][0-9A-F]*) *_t4_vect .*$/ ) {
		printf STDOUT "-g _t4_vect=0x%x\n", hex($1);
	}
#  adc_vect
	if (/^D: *([0-9A-F][0-9A-F]*) *_adc_vect .*$/ ) {
		printf STDOUT "-g _adc_vect=0x%x\n", hex($1);
	}
#  aec_vect
	if (/^D: *([0-9A-F][0-9A-F]*) *_aec_vect .*$/ ) {
		printf STDOUT "-g _aec_vect=0x%x\n", hex($1);
	}
#  dma_vect
	if (/^D: *([0-9A-F][0-9A-F]*) *_dma_vect .*$/ ) {
		printf STDOUT "-g _dma_vect=0x%x\n", hex($1);
	}
#  _limit
	if (/^C: *([0-9A-F][0-9A-F]*) *l_PSEG.*$/ ) {
		printf STDOUT "-b PSEG=0x%x\n", hex($1);
	}

# XDATA
#  _limit
#	if (/^D: *([0-9A-F][0-9A-F]*) *_xseg_end.*$/ ) {
#		printf STDOUT "-b XSEG=0x%x\n", hex($1);
#	}
	if (/^C: *([0-9A-F][0-9A-F]*) *s_XISEG.*$/ ) {
		 $a=hex($1);
	}
	if (/^C: *([0-9A-F][0-9A-F]*) *l_XISEG.*$/ ) {
		 $b=hex($1);
	}

}
printf STDOUT "-b xSEG=0x%x\n", $a+$b;
printf STDOUT "-b BSEG=0x%x\n", $bs+$bl;
printf STDOUT "-e\n";

