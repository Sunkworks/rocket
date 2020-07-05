# Sunkworks ~~rocket~~ missile version 1
It needs a better name.

## Getting started with the project
Make sure to have `clang-format` installed.
After cloning the project, run `clang-format --dump-config --style=Google > .clang-format` in the base directory of the project.
The reason for this is that including a `.clang-format` file leads to all sorts of issues with compatibility depending on which version of `clang-format` that is installed.

## Coding style
This project adheres to the Google C++ Style Guide (atleast the sections on naming, comments and formatting).
Use the cppstyle plugin for Eclipse in order to format the code correctly.
Read the style guide for more info on naming and such.  
The style guide: https://google.github.io/styleguide/cppguide.html

## Current code progress
* [x] The project has been converted to C++  
* [x] 16-bit pressure and temperature readouts working  
* [ ] Oversampling support for pressure and temperature  
* [ ] Proper filter settings for BMP280  
* [ ] Interrupt based sampling for BMP280  
* [ ] IMU :running:
	- [x] Basic interface to accelerometer and gyroscope data
	- [ ] Support for different configs (resolution, range, power mode etc) :gear:  
	- [ ] Self-tests
* [ ] LoRa communication with ground station :satellite: :rocket:   
	- [ ] Remote launch  
	- [ ] Remote telemetry  

	