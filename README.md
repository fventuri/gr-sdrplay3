# gr-sdrplay3: GNU Radio OOT module for SDRplay RSP devices (SDRplay API V3.x) - EXPERIMENTAL

**Please note that this module is still experimental and has not ben thoroughly been tested - for a stable GNU Radio module for SDRplay RSP devices, please see below.**

This OOT GNU Radio module supports the following SDRplay RSP devices:
  - RSP1
  - RSP1A
  - RSP1B
  - RSP2
  - RSPduo (all modes of operation)
  - RSPdx

This module uses some features of GNU Radio (like pybind11 for Pythin bindings) that are only found in GNU Radio 3.9 (it should build and run without problems using GNU Radio master branch).

## Important notice

As mentioned above this module requires GNU Radio version 3.9 (preferably master branch).

For those running previous versions of GNU Radio the recommended OOT module for SDRplay RSP devices can be found here: https://github.com/fventuri/gr-sdrplay/tree/API3+RSPduo

## Dependencies

This module requires SDRplay API V3.X.

The latest API can be downloaded from SDRplay here: https://www.sdrplay.com/api

The SDRplay API/HW driver should be installed before this module is built.

## Build and installation

```
cd gr-sdrplay3
mkdir build && cd build && cmake .. && make
sudo make install
```


## Credits

- Tom Breyer, DJ6TB for providing the very useful FM receiver example


## License

GPLv3
