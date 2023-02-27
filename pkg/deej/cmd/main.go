package main

// #cgo CFLAGS: -m64
// #cgo LDFLAGS: -lmmdevapi -lole32
//#define COBJMACROS // Allow INTERFACE_METHOD(This, xxx)
//#include <initguid.h> // https://stackoverflow.com/a/31757757
//#include <windows.h>
//#include <stdio.h>
//#include <mmdeviceapi.h> // IMMDevice, IMMDeviceEnumerator
//#include <endpointvolume.h> // IAudioMeterInformation
//#include "../volumemeter.h"
import "C"
import (
	"flag"
	"fmt"

	"github.com/omriharel/deej/pkg/deej"
)

var (
	gitCommit  string
	versionTag string
	buildType  string

	verbose bool
)

func init() {
	flag.BoolVar(&verbose, "verbose", false, "show verbose logs (useful for debugging serial)")
	flag.BoolVar(&verbose, "v", false, "shorthand for --verbose")
	flag.Parse()
}

func main() {
	new_volume_meter();
	get_meter_level();	

	// first we need a logger
	logger, err := deej.NewLogger(buildType)
	if err != nil {
		panic(fmt.Sprintf("Failed to create logger: %v", err))
	}

	named := logger.Named("main")
	named.Debug("Created logger")

	named.Infow("Version info",
		"gitCommit", gitCommit,
		"versionTag", versionTag,
		"buildType", buildType)

	// provide a fair warning if the user's running in verbose mode
	if verbose {
		named.Debug("Verbose flag provided, all log messages will be shown")
	}

	// create the deej instance
	d, err := deej.NewDeej(logger, verbose)
	if err != nil {
		named.Fatalw("Failed to create deej object", "error", err)
	}

	// if injected by build process, set version info to show up in the tray
	if buildType != "" && (versionTag != "" || gitCommit != "") {
		identifier := gitCommit
		if versionTag != "" {
			identifier = versionTag
		}

		versionString := fmt.Sprintf("Version %s-%s", buildType, identifier)
		d.SetVersion(versionString)
	}

	// onwards, to glory
	if err = d.Initialize(); err != nil {
		named.Fatalw("Failed to initialize deej", "error", err)
	}
}

func new_volume_meter(){
	C.init_device();
	C.init_meter();
}

func get_meter_level() {
	level := C.float(C.get_meter_level());
	fmt.Println("Level: ", float32(level));
}
