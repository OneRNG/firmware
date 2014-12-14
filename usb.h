#ifndef USB_H
#define USB_H
// USBADDR
#define USBADDR_UPDATE              0x80
#define USBADDR_USBADDR             0x7F

// USBPOW
#define USBPOW_ISO_WAIT_SOF         0x80
#define USBPOW_RST                  0x08
#define USBPOW_RESUME               0x04
#define USBPOW_SUSPEND              0x02
#define USBPOW_SUSPEND_EN           0x01

// USBIIF
#define USBIIF_INEP5IF              0x20
#define USBIIF_INEP4IF              0x10
#define USBIIF_INEP3IF              0x08
#define USBIIF_INEP2IF              0x04
#define USBIIF_INEP1IF              0x02
#define USBIIF_EP0IF                0x01

// USBOIF
#define USBOIF_OUTEP5IF             0x20
#define USBOIF_OUTEP4IF             0x10
#define USBOIF_OUTEP3IF             0x08
#define USBOIF_OUTEP2IF             0x04
#define USBOIF_OUTEP1IF             0x02

// USBCIF
#define USBCIF_SOFIF                0x08
#define USBCIF_RSTIF                0x04
#define USBCIF_RESUMEIF             0x02
#define USBCIF_SUSPENDIF            0x01

// USBIIE
#define USBIIE_INEP5IE              0x20
#define USBIIE_INEP4IE              0x10
#define USBIIE_INEP3IE              0x08
#define USBIIE_INEP2IE              0x04
#define USBIIE_INEP1IE              0x02
#define USBIIE_EP0IE                0x01

// USBOIE
#define USBOIE_OUTEP5IE             0x20
#define USBOIE_OUTEP4IE             0x10
#define USBOIE_OUTEP3IE             0x08
#define USBOIE_OUTEP2IE             0x04
#define USBOIE_OUTEP1IE             0x02

// USBCIE
#define USBCIE_SOFIE                 0x08
#define USBCIE_RSTIE                 0x04
#define USBCIE_RESUMEIE              0x02
#define USBCIE_SUSPENDIE             0x01

// USBCS0
#define USBCS0_CLR_SETUP_END         0x80
#define USBCS0_CLR_OUTPKT_RDY        0x40
#define USBCS0_SEND_STALL            0x20
#define USBCS0_SETUP_END             0x10
#define USBCS0_DATA_END              0x08
#define USBCS0_SENT_STALL            0x04
#define USBCS0_INPKT_RDY             0x02
#define USBCS0_OUTPKT_RDY            0x01

// USBCSIL
#define USBCSIL_CLR_DATA_TOG         0x40
#define USBCSIL_SENT_STALL           0x20
#define USBCSIL_SEND_STALL           0x10
#define USBCSIL_FLUSH_PACKET         0x08
#define USBCSIL_UNDERRUN             0x04
#define USBCSIL_PKT_PRESENT          0x02
#define USBCSIL_INPKT_RDY            0x01

// USBCSIH
#define USBCSIH_AUTOSET              0x80
#define USBCSIH_ISO                  0x40
#define USBCSIH_FORCE_DATA_TOG       0x08
#define USBCSIH_IN_DBL_BUF           0x01

// USBCSOL
#define USBCSOL_CLR_DATA_TOG         0x80
#define USBCSOL_SENT_STALL           0x40
#define USBCSOL_SEND_STALL           0x20
#define USBCSOL_FLUSH_PACKET         0x10
#define USBCSOL_DATA_ERROR           0x08
#define USBCSOL_OVERRUN              0x04
#define USBCSOL_FIFO_FULL            0x02
#define USBCSOL_OUTPKT_RDY           0x01

// USBCSOH
#define USBCSOH_AUTOCLEAR            0x80
#define USBCSOH_ISO                  0x40
#define USBCSOH_OUT_DBL_BUF          0x01

#define SLEEP_USB_EN                 0x80

// USBCTRL
#define USBCTRL_PLL_LOCKED           0x80
#define USBCTRL_PLL_EN               0x02
#define USBCTRL_USB_EN               0x01

struct device_desc {
	unsigned char bLength;
	unsigned char bDescriptorType;
	unsigned int bcdUSB;
	unsigned char bDeviceClass;
	unsigned char bDeviceSubClass;
	unsigned char bDeviceProtocol;
	unsigned char bMaxPacketSize0;
	unsigned int idVendor;
	unsigned int idProduct;
	unsigned int bcdDevice;
	unsigned char iManufacturer;
	unsigned char iProduct;
	unsigned char iSerialNumber;
	unsigned char bNumConfigurations;
};

struct configuration_desc {
	unsigned char bLength;
	unsigned char bDescriptorType;
	unsigned int  wTotalLength;
	unsigned char bNumInterfaces;
	unsigned char bConfigurationValue;
	unsigned char iConfiguration;
	unsigned char bmAttributes;
	unsigned char bMaxPower;
};

struct interface_desc {
	unsigned char bLength;
	unsigned char bDescriptorType;
	unsigned char bInterfaceNumber;
	unsigned char bAlternateSetting;
	unsigned char bNumEndpoints;
	unsigned char bInterfaceClass;
	unsigned char bInterfaceSubClass;
	unsigned char bInterfaceProtocol;
	unsigned char iInterface;
} ;

struct endpoint_desc {
	unsigned char bLength;
	unsigned char bDescriptorType;
	unsigned char bEndpointAddress;
	unsigned char bmAttributes;
	unsigned int  wMaxPacketSize;
	unsigned char bInterval;
};

struct string_desc {
	unsigned char bLength;
	unsigned char bDescriptorType;
	unsigned int  pString[1];
};

#define EP0_PACKET_SIZE		32  

#define DESC_TYPE_DEVICE	1
#define DESC_TYPE_CONFIG	2
#define DESC_TYPE_STRING	3
#define DESC_TYPE_INTERFACE	4
#define DESC_TYPE_ENDPOINT	5 

#define EP_ATTR_CTRL		0
#define EP_ATTR_ISO		1
#define EP_ATTR_BULK		2
#define EP_ATTR_INT		3
#define EP_ATTR_TYPE_BM		3

#define CDC_SEND_ENCAPSULATED_COMMAND      0x00
#define CDC_GET_ENCAPSULATED_RESPONSE      0x01
#define CDC_SET_COMM_FEATURE               0x02     //optional
#define CDC_GET_COMM_FEATURE               0x03     //optional
#define CDC_CLEAR_COMM_FEATURE             0x04     //optional
#define CDC_SET_LINE_CODING                0x20     //optional
#define CDC_GET_LINE_CODING                0x21     //optional
#define CDC_SET_CONTROL_LINE_STATE         0x22     //optional
#define CDC_SEND_BREAK                     0x23     //optional

#define CDC_CHAR_FORMAT_1_STOP_BIT     0
#define CDC_CHAR_FORMAT_1_5_STOP_BIT   1
#define CDC_CHAR_FORMAT_2_STOP_BIT     2

#define CDC_PARITY_TYPE_NONE           0
#define CDC_PARITY_TYPE_ODD            1
#define CDC_PARITY_TYPE_EVEN           2
#define CDC_PARITY_TYPE_MARK           3
#define CDC_PARITY_TYPE_SPACE          4

#define CDC_DEVICE                  0x02
#define COMM_INTF                   0x02

#define ABSTRACT_CONTROL_MODEL      0x02
#define V25TER                      0x01    
#define DATA_INTF                   0x0A
#define NO_PROTOCOL                 0x00   

#define ABSTRACT_STATE              0x01
#define COUNTRY_SETTING             0x02

#define CS_INTERFACE                0x24
#define CS_ENDPOINT                 0x25

#define DSC_FN_HEADER               0x00
#define DSC_FN_CALL_MGT             0x01
#define DSC_FN_ACM                  0x02    
#define DSC_FN_DLM                  0x03   
#define DSC_FN_TELEPHONE_RINGER     0x04
#define DSC_FN_RPT_CAPABILITIES     0x05
#define DSC_FN_UNION                0x06
#define DSC_FN_COUNTRY_SELECTION    0x07
#define DSC_FN_TEL_OP_MODES         0x08
#define DSC_FN_USB_TERMINAL         0x09

#define CDC_COMM_INTF_ID            0x00
#define CDC_DATA_INTF_ID            0x01



extern unsigned char __code descs[];
extern unsigned char __xdata * __data pData;
extern unsigned int __data bytes_left;
extern unsigned char __pdata usbState;             ///< USB device state
extern unsigned char __xdata configurationValue;   ///< Current configuration value
extern unsigned char __xdata pAlternateSetting[5];  ///< Current alternate settings
extern unsigned char __pdata ep0Status;             ///< Endpoint 0 status
extern unsigned char __pdata pEpInStatus[5];        ///< Endpoint 1-5 IN status
extern unsigned char __pdata pEpOutStatus[5];
extern unsigned char __pdata remoteWakeup;  
#define selfPowered  0  

struct sh {
	unsigned char requestType;
	unsigned char request;
	unsigned int value;
	unsigned int index;
	unsigned int length;
};
extern struct sh  __pdata setup_header;

struct lc {
	unsigned long baudRate;
	unsigned char charFormat;
	unsigned char parityType;
	unsigned char dataBits;
};

extern struct lc __xdata line_config;

void usb_read_fifo_5(unsigned char __xdata *p, unsigned char l);
void usb_read_fifo_4(unsigned char __xdata *p, unsigned char l);
void usb_read_fifo_3(unsigned char __xdata *p, unsigned char l);
void usb_write_fifo_5(unsigned char __xdata *p, unsigned char l);
void usb_write_fifo_4(unsigned char __xdata *p, unsigned char l);
void usb_write_fifo_3(unsigned char __xdata *p, unsigned char l);
void usb_has_data_5();
void usb_has_data_4();
void usb_has_data_3();
void usb_can_send_5();
void usb_can_send_4();
void usb_can_send_3();
void flush_in_3();
void flush_in_4();
void flush_in_5();
void cdc_init(unsigned long baudrate);
void usb_init();

extern __bit cdcRTS_3, cdcRTS_4, cdcRTS_5;

#define EP_IDLE      0
#define EP_TX        1
#define EP_RX        2
#define EP_HALT      3
#define EP_STALL     4
#define EP_MANUAL_TX 5
#define EP_MANUAL_RX 6
#define EP_CANCEL    7

#define DEV_ATTACHED   0
#define DEV_POWERED    1
#define DEV_DEFAULT    2
#define DEV_ADDRESS    3
#define DEV_CONFIGURED 4
#define DEV_SUSPENDED  5

#define RT_MASK_DIR       0x80  
#define RT_MASK_TYPE      0x60 
#define RT_MASK_RECIP     0x1F

#define RT_DIR_IN         0x80
#define RT_DIR_OUT        0x00

#define RT_TYPE_STD       0x00
#define RT_TYPE_CLASS     0x20
#define RT_TYPE_VEND      0x40

#define RT_RECIP_DEV      0x00
#define RT_RECIP_IF       0x01
#define RT_RECIP_EP       0x02
#define RT_RECIP_OTHER    0x03

#define RT_STD_OUT        (RT_TYPE_STD | RT_DIR_OUT)
#define RT_STD_IN         (RT_TYPE_STD | RT_DIR_IN)
#define RT_VEND_OUT       (RT_TYPE_VEND | RT_DIR_OUT)
#define RT_VEND_IN        (RT_TYPE_VEND | RT_DIR_IN)
#define RT_CLASS_OUT      (RT_TYPE_CLASS | RT_DIR_OUT)
#define RT_CLASS_IN       (RT_TYPE_CLASS | RT_DIR_IN)

#define RT_OUT_DEVICE     (RT_DIR_OUT | RT_RECIP_DEV)
#define RT_IN_DEVICE      (RT_DIR_IN | RT_RECIP_DEV)
#define RT_OUT_INTERFACE  (RT_DIR_OUT | RT_RECIP_IF)
#define RT_IN_INTERFACE   (RT_DIR_IN | RT_RECIP_IF)
#define RT_OUT_ENDPOINT   (RT_DIR_OUT | RT_RECIP_EP)
#define RT_IN_ENDPOINT    (RT_DIR_IN | RT_RECIP_EP)


#define GET_STATUS           0x00
#define CLEAR_FEATURE        0x01
#define SET_FEATURE          0x03
#define SET_ADDRESS          0x05
#define GET_DESCRIPTOR       0x06
#define SET_DESCRIPTOR       0x07
#define GET_CONFIGURATION    0x08
#define SET_CONFIGURATION    0x09
#define GET_INTERFACE        0x0A
#define SET_INTERFACE        0x0B
#define SYNCH_FRAME          0x0C

#define ENDPOINT_HALT        0x00
#define DEVICE_REMOTE_WAKEUP 0x01

extern void class_in();
extern void class_out();
extern void change_uart_mode();
extern void prog_setup();
#endif
       
