# ofVideo2OSC
This is a little utility program which grabs the camera input, extracts featues and outputs them via OSC.
It is meant be used with wekinator.

## status
This is a work in progress. Color tracking is broken.

## Installation

* clone this repo into your apps directory inside an openFrameworks installation, e.g. `cd of_v0.11.2_osx_release/apps/myApps && git clone https://github.com/thomasgeissl/ofVideo2OSC.git`
* install dependencies: `bash -c "$(curl -sSL https://raw.githubusercontent.com/thomasgeissl/ofPackageManager/master/scripts/ofPackageManager.sh)" install`
* compile: `make`
* run: `make run`