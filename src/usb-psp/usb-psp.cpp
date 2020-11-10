#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../qemu-usb/vl.h"
#include "../qemu-usb/desc.h"
#include "usb-psp.h"

namespace usb_psp {

typedef struct PSPState {
	USBDevice dev;
	USBDesc desc;
	USBDescDevice desc_dev;
} PSPState;

static const uint8_t psp_dev_descriptor[] = {
	0x12,       // bLength
	0x01,       // bDescriptorType (Device)
	0x00, 0x02, // bcdUSB 2.00
	0x00,       // bDeviceClass (Use class information in the Interface Descriptors)
	0x00,       // bDeviceSubClass
	0x00,       // bDeviceProtocol
	0x40,       // bMaxPacketSize0 64
	0x4C, 0x05, // idVendor 0x054C
	0xCB, 0x01, // idProduct 0x01CB
	0x00, 0x01, // bcdDevice 2.00
	0x01,       // iManufacturer (String Index)
	0x02,       // iProduct (String Index)
	0x00,       // iSerialNumber (String Index)
	0x01,       // bNumConfigurations 1
	// 18 bytes
};

static const uint8_t psp_config_descriptor[] = {
	0x09,       // bLength
	0x02,       // bDescriptorType (Configuration)
	0x27, 0x00, // wTotalLength 39
	0x01,       // bNumInterfaces 1
	0x01,       // bConfigurationValue
	0x00,       // iConfiguration (String Index)
	0xC0,       // bmAttributes Self Powered
	0x01,       // bMaxPower 2mA

	0x09, // bLength
	0x04, // bDescriptorType (Interface)
	0x00, // bInterfaceNumber 0
	0x00, // bAlternateSetting
	0x03, // bNumEndpoints 3
	0xFF, // bInterfaceClass
	0x01, // bInterfaceSubClass
	0xFF, // bInterfaceProtocol
	0x03, // iInterface (String Index)

	0x07,       // bLength
	0x05,       // bDescriptorType (Endpoint)
	0x81,       // bEndpointAddress (IN/D2H)
	0x02,       // bmAttributes (Bulk)
	0x40, 0x00, // wMaxPacketSize 64
	0x00,       // bInterval 0 (unit depends on device speed)

	0x07,       // bLength
	0x05,       // bDescriptorType (Endpoint)
	0x02,       // bEndpointAddress (OUT/H2D)
	0x02,       // bmAttributes (Bulk)
	0x40, 0x00, // wMaxPacketSize 64
	0x00,       // bInterval 0 (unit depends on device speed)

	0x07,       // bLength
	0x05,       // bDescriptorType (Endpoint)
	0x83,       // bEndpointAddress (IN/D2H)
	0x03,       // bmAttributes (Interrupt)
	0x08, 0x00, // wMaxPacketSize 8
	0x08,       // bInterval 8 (unit depends on device speed)
	// 39 bytes
};

static const USBDescStrings desc_strings = {
	"",
	"",
	"\"PSP\" Type D",
};

static void usb_psp_handle_reset(USBDevice *dev) {
	fprintf(stderr, "Reset\n");
}

static void usb_psp_handle_control(USBDevice *dev, USBPacket *p, int request, int value,
								int index, int length, uint8_t *data) {
	int ret = 0;

	fprintf(stderr, "F: usb_ctrl; req=%4x (%s), val=%4x, idx=%4x, len=%4x : [",
		request,
		(request ==      5) ? "setAddr" :
		(request == 0x8006) ? "getDesc" :
		(request ==      9) ? "setConf" :
		(request == 0x2109) ? "setRepr" :
		"",
		value, index, length);

	ret = usb_desc_handle_control(dev, p, request, value, index, length, data);
	if (ret >= 0) {
		for (int i = 0; i < p->actual_length; i++) {
			fprintf(stderr, "%02x ", data[i]);
		}
		fprintf(stderr, "]\n");
		return;
	}

	switch (request) {
	case VendorInterfaceOutRequest | 0x02:
	case VendorInterfaceOutRequest | 0x03:
	case VendorInterfaceOutRequest | 0x07:
		for (int i = 0; i < length; i++) {
			fprintf(stderr, "%02x ", data[i]);
		}
		fprintf(stderr, "]\n");
		return;
	case VendorInterfaceRequest | 0x08:
		{
		uint8_t ret[] = {0x00, 0x00, 0x00, 0x03};
		memcpy(data, ret, sizeof(ret));
		p->actual_length = sizeof(ret);
		}
		break;
	case VendorInterfaceRequest | 0x01:
		{
		uint8_t ret[] = {0x55, 0x4C, 0x45, 0x53, 0x30, 0x30, 0x33, 0x36, 0x38, 0x00}; // ULES00368
		memcpy(data, ret, sizeof(ret));
		p->actual_length = sizeof(ret);
		}
		break;
	default:
		p->status = USB_RET_STALL;
		break;
	}
	for (int i = 0; i < p->actual_length; i++) {
		fprintf(stderr, "%02x ", data[i]);
	}
	fprintf(stderr, "]\n");
}

static void usb_psp_handle_data(USBDevice *dev, USBPacket *p) {
	static const int max_ep_size = 896;
	uint8_t data[max_ep_size];
	uint8_t devep = p->ep->nr;

	switch (p->pid) {
	case USB_TOKEN_IN:
		//fprintf(stderr, "F: dataIn: pid=%x id=%llx ep=%x : \n", p->pid, p->id, p->ep->nr);
		p->status = USB_RET_NAK;
		break;

	case USB_TOKEN_OUT:
		fprintf(stderr, "F: dataOut: pid=%x id=%llx ep=%x : \n", p->pid, p->id, p->ep->nr);
		break;


	default:
		fprintf(stderr, "Bad token\n");
		p->status = USB_RET_STALL;
		break;
	}
}

static void usb_psp_handle_destroy(USBDevice *dev) {
	PSPState* s = (PSPState*)dev;
	delete s;
}

USBDevice* PSPDevice::CreateDevice(int port) {
	PSPState* s = new PSPState();
	std::string api = *PSPDevice::ListAPIs().begin();

	TSTDSTRING var;

	s->dev.speed = USB_SPEED_FULL;

	s->desc.full = &s->desc_dev;
	s->desc.str = desc_strings;
	if (usb_desc_parse_dev(psp_dev_descriptor, sizeof(psp_dev_descriptor), s->desc, s->desc_dev) < 0)
		goto fail;
	if (usb_desc_parse_config(psp_config_descriptor, sizeof(psp_config_descriptor), s->desc_dev) < 0)
		goto fail;

	s->dev.klass.handle_attach  = usb_desc_attach;
	s->dev.klass.handle_reset   = usb_psp_handle_reset;
	s->dev.klass.handle_control = usb_psp_handle_control;
	s->dev.klass.handle_data    = usb_psp_handle_data;
	s->dev.klass.unrealize      = usb_psp_handle_destroy;
	s->dev.klass.usb_desc       = &s->desc;
	s->dev.klass.product_desc   = desc_strings[2];

	usb_desc_init(&s->dev);
	usb_ep_init(&s->dev);

	usb_psp_handle_reset((USBDevice *)s);
	return (USBDevice *)s;

fail:
	usb_psp_handle_destroy((USBDevice *)s);
	return NULL;
}

const char* PSPDevice::TypeName() {
	return "psp";
}

int PSPDevice::Freeze(int mode, USBDevice* dev, void* data) {
	return 0;
}

int PSPDevice::Configure(int port, const std::string& api, void* data) {
	return RESULT_CANCELED;
}

} //namespace
