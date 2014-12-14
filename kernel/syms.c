// (c) Copyright Paul Campbell paul@taniwha.com 2013
// 
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) version 3, or any
// later version accepted by Paul Campbell , who shall
// act as a proxy defined in Section 6 of version 3 of the license.
// 
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public 
// License along with this library.  If not, see <http://www.gnu.org/licenses/>.
//

void ____x___() __naked
{
	__asm;
	.globl	_rx_len
	.globl	_rx_packet
	.globl	_rx_mac
	.globl	_rtx_key
	.globl	_uart_rx_0_vect
	.globl	_uart_rx_1_vect
	.globl	_uart_tx_0_vect
	.globl	_uart_tx_1_vect
	.globl	_p0_vect
	.globl	_p1_vect
	.globl	_p2_vect
	.globl	_t2_vect
	.globl	_t3_vect
	.globl	_t4_vect
	.globl	_adc_vect
	.globl	_aec_vect
	.globl	_dma_vect
	.globl	_queue_task_PARM_2
	.globl	_rf_send_PARM_2
	.globl	_rf_send_PARM_3
	.globl	_rf_send_PARM_4
#ifdef DRV_KEYS
	.globl	_key
#endif
#ifdef DRV_DAYLIGHT
	.globl	_daylight
#endif
	.globl	_putchar
	.globl	_putstr
	.globl	_puthex
	.globl	_wait_us
	.globl	_queue_task
	.globl	_cancel_task
	.globl	_uart_init
	.globl	_rf_set_key
	.globl	_rf_set_key_c
	.globl	_rf_set_promiscuous
	.globl	_rf_set_raw
	.globl	_rf_set_mac
	.globl	_rf_set_channel
	.globl	_rf_set_transmit_power
	.globl	_rf_receive_on
	.globl	_rf_receive_off
	.globl	_rf_send
#ifdef DRV_LEDS
	.globl	_leds_off
	.globl	_leds_rgb
#endif
#ifdef DRV_KEYS
	.globl	_keys_off
#endif
	.globl	_system_attributes
#ifdef DRV_KEYS
	.globl	_keys_on
#endif
	.area CSEG    (CODE)
	.globl	__2
__2=_rx_len
	.globl	__3
__3=_rx_packet
	.globl	__4
__4=_rx_mac
	.globl	__5
__5=_rtx_key
	.globl	__6
__6=_uart_rx_0_vect
	.globl	__7
__7=_uart_rx_1_vect
	.globl	__8
__8=_uart_tx_0_vect
	.globl	__9
__9=_uart_tx_1_vect
	.globl	__10
__10=_p0_vect
	.globl	__11
__11=_p1_vect
	.globl	__12
__12=_p2_vect
	.globl	__13
__13=_t2_vect
	.globl	__14
__14=_t3_vect
	.globl	__15
__15=_t4_vect
	.globl	__16
__16=_adc_vect
	.globl	__17
__17=_aec_vect
	.globl	__18
__18=_dma_vect
	.globl	__19
__19=_queue_task_PARM_2
	.globl	__20
__20=_rf_send_PARM_2
	.globl	__21
__21=_rf_send_PARM_3
	.globl	__22
__22=_rf_send_PARM_4
#ifdef DRV_KEYS
	.globl	__23
__23=_key
#endif
#ifdef DRV_DAYLIGHT
	.globl	__24
__24=_daylight
#endif
	.globl	__25
__25=_putchar
	.globl	__26
__26=_putstr
	.globl	__27
__27=_puthex
	.globl	__28
__28=_wait_us
	.globl	__29
__29=_queue_task
	.globl	__30
__30=_cancel_task
	.globl	__31
__31=_uart_init
	.globl	__32
__32=_rf_set_key
	.globl	__33
__33=_rf_set_promiscuous
	.globl	__34
__34=_rf_set_raw
	.globl	__35
__35=_rf_set_mac
	.globl	__36
__36=_rf_set_channel
	.globl	__37
__37=_rf_set_transmit_power
	.globl	__38
__38=_rf_receive_on
	.globl	__39
__39=_rf_receive_off
	.globl	__40
__40=_rf_send
#ifdef DRV_LEDS
	.globl	__41
__41=_leds_off
	.globl	__42
__42=_leds_rgb
#endif
#ifdef DRV_KEYS
	.globl	__43
__43=_keys_off
	.globl	__44
__44=_keys_on
	.globl	__45
#endif
__45=_system_attributes
	.globl	__46
__46=_rf_set_key_c
	.globl	__33
	__endasm;
}
