#ifndef USBPSP_H
#define USBPSP_H
#include "../deviceproxy.h"

namespace usb_psp {

static const char *APINAME = "socket";

class PSPDevice
{
public:
	virtual ~PSPDevice() {}
	static USBDevice* CreateDevice(int port);
	static const char* TypeName();
	static const TCHAR* Name()
	{
		return TEXT("PlayStation Portable device link");
	}
	static std::list<std::string> ListAPIs()
	{
		return std::list<std::string> { APINAME };
	}
	static const TCHAR* LongAPIName(const std::string& name)
	{
		return TEXT("TCP Socket");
	}
	static int Configure(int port, const std::string& api, void *data);
	static int Freeze(int mode, USBDevice *dev, void *data);
	static std::vector<std::string> SubTypes() { return std::vector<std::string>(); }
};

}
#endif
