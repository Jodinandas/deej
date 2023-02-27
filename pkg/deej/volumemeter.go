package deej

// #cgo CFLAGS: -m64
// #cgo LDFLAGS: -lmmdevapi -lole32
//#define COBJMACROS // Allow INTERFACE_METHOD(This, xxx)
//#include <initguid.h> // https://stackoverflow.com/a/31757757
//#include <windows.h>
//#include <stdio.h>
//#include <mmdeviceapi.h> // IMMDevice, IMMDeviceEnumerator
//#include <endpointvolume.h> // IAudioMeterInformation
//#include "volumemeter.h"
import "C"
import (
	"fmt"
)

func new_volume_meter(){
	C.init_device();
	C.init_meter();
}

func get_meter_level() {
	level := C.float(C.get_meter_level());
	fmt.Println(float32(level));
}