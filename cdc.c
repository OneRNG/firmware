#include <mcs51reg.h>
#include <cc2530.h>
#include "usb.h"
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

extern void wake_5();
const unsigned char __code descs[] = {
//deviceDesc:     ; Device descriptor
                18,
                DESC_TYPE_DEVICE,   // bDescriptorType
                0, 2,        	 // bcdUSB
                2,       	// bDeviceClass			
                0,              // bDeviceSubClass
                0,              // bDeviceProtocol
                EP0_PACKET_SIZE,
                //0x51, 4,         // idVendor Texas Instruments
                //0xA8, 0x16,       // idProduct CC2531
		0x50, 0x1d,	// openmoko
		0x86, 0x60,
                9, 0,         // bcdDevice
                1,              // iManufacturer
                3,              // iProduct
                3,              // iSerialNumber
                1,              // bNumConfigurations


//configDesc:     ; Configuration descriptor
                9,
                DESC_TYPE_CONFIG,// bDescriptorType
                0x43, 0,
                2,              // NumInterfaces
                1,              // bConfigurationValue
                0,              // iConfiguration
                0x80,              // bmAttributes
                100,              // MaxPower
//
//               INTERFACE 0		// 
//


//interface0Desc: ; Interface descriptor
                9,
                DESC_TYPE_INTERFACE,    // bDescriptorType
                0,                    	// bInterfaceNumber
                0,                    	// bAlternateSetting
                1,                    	// bNumEndpoints
                COMM_INTF,              // bInterfaceClass
                ABSTRACT_CONTROL_MODEL, // bInterfaceSubClass
                V25TER,                 // bInterfaceProcotol
                0,                    	// iInterface

// CDC Class-Specific Descriptors
//headerFunctionalDesc: ; Header Functional Descriptor
                5,
                CS_INTERFACE,
                DSC_FN_HEADER,
                0x10, 0x1,
//absCtrlManFuncDesc: ; Abstract Control Management Functional Descriptor
                4,
                CS_INTERFACE,
                DSC_FN_ACM,
                0x6,
//unionFunctionalDesc: ; Union Functional Descriptor
                5,
                CS_INTERFACE,
                DSC_FN_UNION,
                0,
                1,
//callMngFuncDesc: ; Call Management Functional Descriptor
                5,
                CS_INTERFACE,
                DSC_FN_CALL_MGT,
                0,
                1,

//endpoint0Desc:  ; Endpoint descriptor (EP2 IN)
                7,
                DESC_TYPE_ENDPOINT,    // bDescriptorType
                0x82,                  // bEndpointAddress
                EP_ATTR_INT,           // bmAttributes
                0x20, 0,               // wMaxPacketSize
                0x40,                  // bInterval

//
//              INTERFACE 1	- EP 5 - random
//

//interface1Desc: ; Interface descriptor
                9,
                DESC_TYPE_INTERFACE,   // Interface descriptor type
                1,                     // Interface Number
                0,                     // Alternate Setting Number
                2,                     // Number of endpoints in this intf
                DATA_INTF,             // Class code
                0,                     // Subclass code
                NO_PROTOCOL,           // Protocol code
                4,                     // Interface string index


//endpoint1Desc:  ; Endpoint descriptor (EP5 OUT)
                7,
                DESC_TYPE_ENDPOINT,    // bDescriptorType
                0x85,                  // bEndpointAddress
                EP_ATTR_BULK,          // bmAttributes
                64, 0,                // wMaxPacketSize
                1,                     // bInterval

//endpoint2Desc:  ; Endpoint descriptor (EP5 IN)
                7,
                DESC_TYPE_ENDPOINT,    // bDescriptorType
                5,                     // bEndpointAddress
                EP_ATTR_BULK,          // bmAttributes
                64, 0,                // wMaxPacketSize
                1,                     // bInterval


//--------------
// String descriptors
//string0Desc:    ; Language ID	
                4,
                DESC_TYPE_STRING,      // bDescriptorType
                9,                     //  US-EN
                4,

//string1Desc:    ; Manufacturer
                102,
                DESC_TYPE_STRING,       // bDescriptorType
                'M', 0,
                'o', 0,
                'o', 0,
                'n', 0,
                'b', 0,
                'a', 0,
                's', 0,
                'e', 0,
                ' ', 0,
                'O', 0,
                't', 0,
                'a', 0,
                'g', 0,
                'o', 0,
                ' ', 0,
                'h', 0,
                't', 0,
                't', 0,
                'p', 0,
                ':', 0,
                '/', 0,
                '/', 0,
                'w', 0,
                'w', 0,
                'w', 0,
                '.', 0,
                'm', 0,
                'o', 0,
                'o', 0,
                'n', 0,
                'b', 0,
                'a', 0,
                's', 0,
                'e', 0,
                'o', 0,
                't', 0,
                'a', 0,
                'g', 0,
                'o', 0,
                '.', 0,
                'c', 0,
                'o', 0,
                'm', 0,
                '/', 0,
                'r', 0,
                'a', 0,
                'n', 0,
                'd', 0,
                'o', 0,
                'm', 0,

//string2Desc:    ; Product
                70,
                DESC_TYPE_STRING,      // bDescriptorType
                'C', 0,
                'C', 0,
                '2', 0,
                '5', 0,
                '3', 0,
                '1', 0,
                ' ', 0,
                'U', 0,
                'S', 0,
                'B', 0,
                ' ', 0,
                'R', 0,
                'a', 0,
                'n', 0,
                'd', 0,
                'o', 0,
                'm', 0,
                ' ', 0,
                'N', 0,
                'u', 0,
                'm', 0,
                'b', 0,
                'e', 0,
                'r', 0,
                ' ', 0,
                'G', 0,
                'e', 0,
                'n', 0,
                'e', 0,
                'r', 0,
                'a', 0,
                't', 0,
                'o', 0,
                'r', 0,

//string3Desc:    ; Serial number
                6,
                DESC_TYPE_STRING,       // bDescriptorType
                '0', 0,
                '0', 0,

//string4Desc:    ; interface 1 - "random"
                14,
                DESC_TYPE_STRING,       // bDescriptorType
                'R', 0,
                'a', 0,
                'n', 0,
                'd', 0,
                'o', 0,
                'm', 0,

		0	// 0 len is end tag
};

struct lc __xdata line_config;
static struct lc __xdata line_config_alt;
__bit cdcRTS;

void class_out(void)
{
	if (setup_header.request == CDC_SET_CONTROL_LINE_STATE) {	// 0x22
		if(ep0Status == EP_IDLE) {
			unsigned char ep = setup_header.index&0x1f;
			unsigned char v = setup_header.value&1;
 			cdcRTS = v;
			usb_can_send_5();
			ep0Status = EP_RX;
		}
	} else
	if (setup_header.request == CDC_SET_LINE_CODING) {
		if(ep0Status == EP_IDLE) {
			unsigned char ep = setup_header.index&0x1f;
			if (ep == 0) {
				pData = (unsigned char __xdata *) &line_config;
			} else {
				pData = (unsigned char __xdata *) &line_config_alt;
			}
			
			ep0Status = EP_RX;
		} else
		if (ep0Status == EP_RX) {
			unsigned char ep = setup_header.index&0x1f;
			if (ep == 0)
				change_uart_mode();
		}
	} else {
		ep0Status = EP_STALL;
   	}
}

void class_in(void)
{
	if (setup_header.request == CDC_GET_LINE_CODING) {
		if (ep0Status == EP_IDLE) {
			unsigned char ep = setup_header.index;
			if (ep == 0) {
				pData = (unsigned char __xdata *) &line_config;
			} else {
				pData = (unsigned char __xdata *) &line_config_alt;
			}
			bytes_left = 7;
			ep0Status = EP_TX;
		} else
		if (ep0Status == EP_TX) {
		}
	} else {
		ep0Status = EP_STALL;
	}
}

void cdc_init(unsigned long baudrate)
{
    // Set default line coding.
    line_config.baudRate = baudrate;
    line_config.charFormat = CDC_CHAR_FORMAT_1_STOP_BIT;
    line_config.parityType = CDC_PARITY_TYPE_NONE;
    line_config.dataBits = 8;
    cdcRTS = 0;

    // Initialise hardware flow control
    // Initialize the USB interrupt handler with bit mask containing all processed USBIRQ events
    usb_init();

    P1DIR |= 1<<0;
    P1INP |= 1<<0;
    P1_0 = 1;
}

