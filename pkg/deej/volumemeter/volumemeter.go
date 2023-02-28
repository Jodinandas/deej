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
	"fmt"
)

func NewPlaybackVolumeMeter() {
	C.init_default_playback_device()
	C.init_playback_meter()
}

func NewCaptureVolumeMeter(){
	C.init_default_capture_device()
	C.init_capture_meter()
}

func GetPlaybackMeterLevel() (float32) {
	level := C.float(C.get_playback_meter_level());
	return float32(level);
}

func GetCaptureMeterLevel() (float32) {
	level := C.float(C.get_capture_meter_level());
	return float32(level)
}

// opens a new channel that contains all new meter level events
func GetMeterLevelChannel() (chan string){
	ch := make(chan string);
	go func() {
	NewPlaybackVolumeMeter();
	//NewCaptureVolumeMeter(); -> uncommenting this leads to crashing for some reason TODO
	for {
			pb := firstN(fmt.Sprintf("%v", GetPlaybackMeterLevel()), 4)
			//ct := firstN(fmt.Sprintf("%v", GetCaptureMeterLevel()), 4)
			ct := "0.0"
			ch <- ct + "|" + pb
			time.Sleep(50 * time.Millisecond);
		}
	}()
	return ch
}
func firstN(s string, n int) string {
     if len(s) > n {
          return s[:n]
     }
     return s
}