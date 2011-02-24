#include "global.h"
#include "MouseDevice.h"

using __gnu_cxx::hash_map;

Mouse::Mouse() : id( InputDevice_Invalid ),
				x_axis( 0 ), x_min( 0 ), x_max( 0 ),
				y_axis( 0 ), y_min( 0 ), y_max( 0 ),
				z_axis( 0 ), z_min( 0 ), z_max( 0 )
{
}

bool MouseDevice::AddLogicalDevice( int usagePage, int usage )
{
	if( usagePage != kHIDPage_GenericDesktop )
		return false;

	switch( usage )
	{
		// Mice can either be kHIDUsage_GD_Mouse or kHIDUsage_GD_Pointer...
		// Let's just go with kHIDUsage_GD_Mouse for now. -aj
		case kHIDUsage_GD_Mouse:
		//case kHIDUsage_GD_Pointer:
			break;
		default:
			return false;
	}
	// Init only a single mouse for now.
	m_Mouse = Mouse();
	return true;
}

void MouseDevice::AddElement( int usagePage, int usage, IOHIDElementCookie cookie, const CFDictionaryRef properties )
{
	if( usagePage >= kHIDPage_VendorDefinedStart )
		return;

	ASSERT(m_Mouse);
	Mouse& m = m_Mouse;
	int iMin = 0;
	int iMax = 0;
	switch( usagePage )
	{
		case kHIDPage_GenericDesktop:
			iMin = 0;
			IntValue( CFDictionaryGetValue(properties, CFSTR(kIOHIDElementMinKey)), iMin );
			iMax = 0;
			IntValue( CFDictionaryGetValue(properties, CFSTR(kIOHIDElementMaxKey)), iMax );

			// based on usage
			switch( usage )
			{
				case kHIDUsage_GD_X:
					m.x_axis = cookie;
					m.x_min = iMin;
					m.x_max = iMax;
					break;
				case kHIDUsage_GD_Y:
					m.y_axis = cookie;
					m.y_min = iMin;
					m.y_max = iMax;
					break;
				case kHIDUsage_GD_Z:
					m.z_axis = cookie;
					m.z_min = iMin;
					m.z_max = iMax;
					break;
				default:
					//LOG->Warn( "Unknown usagePage usage pair: (kHIDPage_GenericDesktop, %d).", usage );
					break;
			}
			break;
		case kHIDPage_Button:
			{
				const DeviceButton buttonID = enum_add2( MOUSE_LEFT, usage - kHIDUsage_Button_1 );

				if( buttonID <= MOUSE_MIDDLE )
					m_Mapping[cookie] = buttonID;
				else
					LOG->Warn( "Button id too large: %d.", int(buttonID) );
				break;
			}
			break;
		default:
			//LOG->Warn( "Unknown usagePage usage pair: (%d, %d).", usagePage, usage );
			break;
	}

}

void MouseDevice::Open()
{
	//const Mouse& m = m_Mouse;
	for( hash_map<IOHIDElementCookie,DeviceButton>::const_iterator i = m_Mapping.begin(); i != m_Mapping.end(); ++i )
		AddElementToQueue( i->first );
}

void MouseDevice::GetButtonPresses( vector<DeviceInput>& vPresses, IOHIDElementCookie cookie, int value, const RageTimer& now ) const
{
	// todo: add mouse axis stuff -aj
	hash_map<IOHIDElementCookie, DeviceButton>::const_iterator iter = m_Mapping.find( cookie );
	if( iter != m_Mapping.end() )
		vPresses.push_back( DeviceInput(DEVICE_MOUSE, iter->second, value, now) );
}

void MouseDevice::GetDevicesAndDescriptions( vector<InputDeviceInfo>& vDevices ) const
{
	vDevices.push_back( InputDeviceInfo(DEVICE_MOUSE, "Mouse") );
}