# How to install GNU Radio via radioconda and this OOT module 'gr-sdrplay3' on Windows

This document contains the detailed steps on how to install GNU Radio via radioconda and this OOT module 'gr-sdrplay3' on Windows

The installation process is comprised of the following parts:
- install GNU Radio using radioconda
- install the latest version of the SDRplay API
- (optional) build the gr-sdrplay3 OOT module from source using the conda recipe
- A) install use the conda package from the previous step or B) download the prebuilt conda package under 'Releases' and install it
- use GNU Radio Companion to run some examples from the folder `%CONDA_PREFIX%\share\gr-sdrplay3\examples`


## step 0 - remove any previous installations of GNU Radio

For this whole process to be successful and avoid a lot of wasted time and frustration, I strongly recommend to remove any previous installation of GNU Radio (via conda, miniforge3, or otherwise) including GNU Radio installations with PothosSDR or similar applications.


## step 1 - install GNU Radio via radioconda

Following the recommendation in GNU Radio wiki (https://wiki.gnuradio.org/index.php/CondaInstall) use radioconda to install GNU Radio.

- these specific instructions are in the section 'Installation using radioconda (recommended)'
- open the page: https://github.com/ryanvolz/radioconda
- download the file 'radioconda-Windows-x86_64.exe'
- double-click the installer file to install radioconda (possibly clicking on 'Run anyway' in Microsoft Defender)
- in the installation steps, choose 'Just me', and accept the default installation path 'C:\Users\<username>\radioconda'
- open radioconda terminal:
    Start menu -> radioconda -> radioconda Prompt
- update only conda:
```
    conda update conda -y
```

Important notes:
- in previous versions of this document I used to recommend to update all conda packages; not anymore, since in the last year or so one or more of those updates break GNU Radio. The error message I was getting was something like this: 'DLL load failed while importing ...: The specified procedure could not be found'. If you see that error message, it is likely due to the 'update all' issue.
- in previous versions of this document I used to use 'mamba' instead of 'conda', but last time I had an error caused by a typo in an 'IF' command in a bat script somewhere, so for now I am just sticking with 'conda'.
- at the time of this writing the current version of radioconda is 2025.03.14. All the commands and instructions in this document should work with this version without errors.


## step 2 - install the latest version of the SDRplay API

- open the page: https://www.sdrplay.com/api/
- download the SDRplay API for Windows and follow the steps to install it
- accept the default installation path 'C:\Program Files\SDRplay\API'
- the current version of the SDRplay API is 3.15. This OOT module will not build or work with older versions of the SDRplay API.


## step 3 - (optional) build gr-sdrplay3 from source

This step requires Microsoft Visual Studio 2022; I used VS2022 Community Edition, which is free (https://visualstudio.microsoft.com/downloads/)

- use the conda base environment to build 'gr-sdrplay3' as recommended here: https://docs.conda.io/projects/conda-build/en/latest/install-conda-build.html
- install conda build tools
```
    conda install conda-build -y
```
- build the gr-sdrplay3 module
```
    git clone https://github.com/fventuri/gr-sdrplay3/
    cd gr-sdrplay3
    conda build .conda\recipe
```
- if you want to build this module for a specific version (tag) number, before the `conda build` command run `git checkout vA.B.C.D`, where 'vA.B.C.D' is the version/tag (for instance 'v3.11.0.8' as the time of this writing, but it will change as new versions are released)
- this step will take several minutes to complete (10-12 on my computer)
- if successful, it should create a ready to install conda package under 'C:\Users\<username>\radioconda\conda-bld\win-64'; the file name is something like this 'gnuradio-sdrplay3-3.11.0.8-py312h702a0ab_0.conda'. If you don't see that file there, look at the output from the 'conda build' command and see what failed and why
- more details can be found [here](.conda/README.md)


## step 4 - install the gnuradio-sdrplay3 package

In this step we'll use either the conda package built in the previous step or the prebuilt one available from here: https://github.com/fventuri/gr-sdrplay3/releases

- A. if you built the package from source in the previous step, change directory to the location where you built the package (`cd radioconda\conda-bld\win-64`)
- B. if you downloaded the prebuilt package, change directory to the folder where you downloaded it
- install the gnuradio-sdrplay3 package (the exact name of the package will change as new versions are released):
```
    conda install gnuradio-sdrplay3-3.11.0.8-py312h702a0ab_0.conda
```


## step 5 - use GNU Radio Companion to run some examples

- In the start menu click on GNU Radio -> GNU Radio Companion
- select File -> Open and open a GRC flowgraph (file with the '.grc' extension) in the examples folder `%CONDA_PREFIX%\Library\share\gr-sdrplay3\examples`
- make sure it compiles to Python and runs without errors


## Troubleshooting

If the GNU Radio Companion test fails with the error message "ImportError: DLL load failed while importing sdrplay3_python: The specified module could not be found." or "ModuleNotFoundError: No module named 'gnuradio.sdrplay3.sdrplay3_python'", it probably means that radioconda/conda can't find the DLL 'sdrplay_api.dll' in one of the folders where it searches for that DLL:
- `%CONDA_PREFIX%\Lib\site-packages\gnuradio\sdrplay3`
- `%CONDA_PREFIX%`
- `%CONDA_PREFIX%\Library\mingw-w64\bin`
- `%CONDA_PREFIX%\Library\bin`
- `%CONDA_PREFIX%\Scripts`
- `C:\Windows\System32`

The installer should already have taken care of copying the file 'sdrplay_api.dll' under `%CONDA_PREFIX%\Library\bin`. In case the DLL is not there, you can copy it manually with the following command:
```
    copy /B /Y "%ProgramFiles%\SDRplay\API\x64\sdrplay_api.dll" "%CONDA_PREFIX%\Library\bin"
```
