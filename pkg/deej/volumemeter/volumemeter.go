package volumemeter

// #cgo CFLAGS: -m64
// #cgo LDFLAGS: -lmmdevapi -lole32
//#define COBJMACROS // Allow INTERFACE_METHOD(This, xxx)
//#include <initguid.h> // https://stackoverflow.com/a/31757757
//#include <windows.h>
//#include <stdio.h>
//#include <mmdeviceapi.h> // IMMDevice, IMMDeviceEnumerator
//#include <endpointvolume.h> // IAudioMeterInformation
//#include <volumemeter.h>
import "C"
import (
	"time"
)

func NewVolumeMeter() {
	C.init_default_playback_device()
	C.init_playback_meter()
}

func GetMeterLevel() (float32) {
	level := C.float(C.get_playback_meter_level());
	return float32(level);
}

// opens a new channel that contains all new meter level events
func GetMeterLevelChannel() (chan float32){
	ch := make(chan float32);
	go func() {
	NewVolumeMeter();
	for {
			ch <- GetMeterLevel();
			time.Sleep(50 * time.Millisecond);
		}
	}()
	return ch
}