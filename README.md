# gr-sdrplay3: GNU Radio OOT module for SDRplay RSP devices (SDRplay API V3.x)

This OOT GNU Radio module supports the following SDRplay RSP devices:
  - RSP1
  - RSP1A
  - RSP2
  - RSPduo (all modes of operation)
  - RSPdx

This module uses some features of GNU Radio (like pybind11 for Pythin bindings) that are only found in GNU Radio 3.9 (it should build and run without problems using GNU Radio master branch).

## Dependencies

This module requires SDRplay API V3.X.

The latest API/HW driver can be downloaded from SDRplay here: https://www.sdrplay.com/linuxdl2.php

The SDRplay API/HW driver should be installed before this module is built.

## Build and installation

```
cd gr-sdrplay3
mkdir build && cd build && cmake .. && make
sudo make install
```

## License

GPLv3
