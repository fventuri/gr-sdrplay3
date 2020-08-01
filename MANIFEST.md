title: gr-sdrplay3
brief: OOT GNU Radio module for SDRplay RSP device (uses SDRplay API V3)
tags:
  - SDRplay
  - RSP
author:
  - Franco Venturi
copyright_owner:
  - Franco Venturi
license: GPL 3.0
gr_supported_version: 3.9
repo: https://github.com/fventuri/gr-sdrplay3
---
This OOT GNU Radio module supports the following SDRplay RSP devices:
  - RSP1
  - RSP1A
  - RSP2
  - RSPduo (all modes of operation)
  - RSPdx

The module requires SDRplay API V3.X - see https://www.sdrplay.com/linuxdl2.php

The module uses some features of GNU Radio (like pybind11 for Pythin bindings) that are only found in GNU Radio 3.9 (it should build and run without problems using GNU Radio master branch).
