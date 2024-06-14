# gr-sdrplay3: GNU Radio OOT module for SDRplay RSP devices (SDRplay API V3.x)


This OOT GNU Radio module supports the following SDRplay RSP devices:
  - RSP1
  - RSP1A
  - RSP1B
  - RSP2
  - RSPduo (all modes of operation)
  - RSPdx
  - RSPdx-R2

This module uses some features of GNU Radio (like pybind11 for Pythin bindings) that are only found in GNU Radio 3.9 and newer versions (it should build and run without problems using GNU Radio 'main' branch).


## Important notices

- As mentioned above this module requires GNU Radio version 3.9 or a newer version (ideally GNU Radio 'main' branch).

- For GNU Radio versions 3.10.x and higher (for instance on Ubuntu), please use the code in the 'main' branch. The 'maint-3.10' branch is there only for historical reasons, but I don't see any good reasons to use it. 

- The code in the 'main' branch requires the latest version of the SDRplay API (v3.15 at the time of writing). If you have to run this module with an older version of the SDRplay API, please use the code in the 'sdrplay-api-3.07' branch (which will not have further updates).

- detailed instructions on how to run this OOT module 'gr-sdrplay3' on Windows with the radioconda GNU Radio installation are in [this document](WINDOWS.md).


## Dependencies

This module requires SDRplay API V3.X (prefereably the most recent version).

The latest API can be downloaded from SDRplay here: https://www.sdrplay.com/api

The SDRplay API should be installed before this module is built.


## Build and installation

```
cd gr-sdrplay3
mkdir build && cd build && cmake .. && make
sudo make install
sudo ldconfig
```


## Credits

- Tom Breyer, DJ6TB for providing the very useful FM receiver example


## License

GNU GPL V3 (see [LICENSE](LICENSE))
