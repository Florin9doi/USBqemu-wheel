#include "deviceproxy.h"
#include "usb-pad/usb-pad.h"
#include "usb-msd/usb-msd.h"
#include "usb-psp/usb-psp.h"
#include "usb-mic/usb-mic-singstar.h"
#include "usb-mic/usb-headset.h"
#include "usb-hid/usb-hid.h"
#include "usb-eyetoy/usb-eyetoy-webcam.h"

void RegisterDevice::Register()
{
	auto& inst = RegisterDevice::instance();
	if (inst.Map().size()) // FIXME Don't clear proxies, singstar keeps a copy to uninit audio
		return;
	inst.Add(DEVTYPE_PAD, new DeviceProxy<usb_pad::PadDevice>());
	inst.Add(DEVTYPE_MSD, new DeviceProxy<usb_msd::MsdDevice>());
	inst.Add(DEVTYPE_PSP, new DeviceProxy<usb_psp::PSPDevice>());
	inst.Add(DEVTYPE_SINGSTAR, new DeviceProxy<usb_mic::SingstarDevice>());
	inst.Add(DEVTYPE_LOGITECH_MIC, new DeviceProxy<usb_mic::LogitechMicDevice>());
	inst.Add(DEVTYPE_LOGITECH_HEADSET, new DeviceProxy<usb_mic::HeadsetDevice>());
	inst.Add(DEVTYPE_HIDKBD, new DeviceProxy<usb_hid::HIDKbdDevice>());
	inst.Add(DEVTYPE_HIDMOUSE, new DeviceProxy<usb_hid::HIDMouseDevice>());
	inst.Add(DEVTYPE_RBKIT, new DeviceProxy<usb_pad::RBDrumKitDevice>());
	inst.Add(DEVTYPE_BUZZ, new DeviceProxy<usb_pad::BuzzDevice>());
	inst.Add(DEVTYPE_GAMETRAK, new DeviceProxy<usb_pad::GametrakDevice>());
	inst.Add(DEVTYPE_REALPLAY, new DeviceProxy<usb_pad::RealPlayDevice>());
	inst.Add(DEVTYPE_EYETOY, new DeviceProxy<usb_eyetoy::EyeToyWebCamDevice>());
	inst.Add(DEVTYPE_BEATMANIA_DADADA, new DeviceProxy<usb_hid::BeatManiaDevice>());
	inst.Add(DEVTYPE_SEGA_SEAMIC, new DeviceProxy<usb_pad::SeamicDevice>());
	inst.Add(DEVTYPE_KEYBOARDMANIA, new DeviceProxy<usb_pad::KeyboardmaniaDevice>());
	inst.Add(DEVTYPE_AKM_AK5370, new DeviceProxy<usb_mic::AK5370MicDevice>());

	RegisterAPIs();
}

void RegisterDevice::Unregister()
{
	/*for (auto& i: registerDeviceMap)
		delete i.second;*/
	registerDeviceMap.clear();
	delete registerDevice;
	registerDevice = nullptr;

	UnregisterAPIs();
}
