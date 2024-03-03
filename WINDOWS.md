# How to install GNU Radio via radioconda and this OOT module 'gr-sdrplay3' on Windows

This document contains the detailed steps on how to install GNU Radio via radioconda and this OOT module 'gr-sdrplay3' on Windows

The installation process is comprised of the following parts:
- install GNU Radio using radioconda
- install the latest version of the SDRplay API
- (optional) build the gr-sdrplay3 OOT module from source using the conda recipe
- use the conda package from the previous step or download the prebuilt conda package under 'Releases' and install it
- use GNU Radio Companion to run some examples from the folder `%CONDA_PREFIX%\share\gr-sdrplay3\examples`


## step 0 - remove any previous installations of GNU Radio

For this whole process to be successful and avoid a lot of wasted time and frustration, it is imperative to remove any previous installation of GNU Radio (via conda, miniforge3, or otherwise) including GNU Radio installations with PothosSDR or similar applications


## step 1 - install GNU Radio via radioconda

For this step we'll follow the recommendation in GNU Radio wiki and use Ryan Volz excellent radioconda to install GNU Radio: https://wiki.gnuradio.org/index.php/CondaInstall

- we'll follow the intructions in the section 'Installation using radioconda (recommended)'
- open the page: https://github.com/ryanvolz/radioconda
- download the file 'radioconda-Windows-x86_64.exe'
- double-click the installer file to install radioconda (possibly clicking on 'Run anyway' in Microsoft Defender)
- in the installation steps, choose 'Just me', and accept the default installation path 'C:\Users\<username>\radioconda'
- open radioconda terminal:
    Start menu -> radioconda -> Conda Prompt
- update all packages (this command will take a while to run):
```
    mamba update --all -y
```
- close and reopen the radioconda terminal


## step 2 - install the latest version of the SDRplay API

- open the page: https://www.sdrplay.com/api/
- download the SDRplay API for Windows and follow the steps to install it
- accept the default installation path 'C:\Program Files\SDRplay\API'


## step 3 - (optional) build gr-sdrplay3 from source

This step requires Microsoft Visual Studio 2019; I used VS2109 Community Edition, which is free (https://visualstudio.microsoft.com/vs/older-downloads/)

- we'll follow the intructions from [here](.conda/README.md#building-the-package)
- create and activate the build environment; in the radioconda terminal from step 1 run these commands:
```
    mamba create -n build
    mamba activate build
    conda config --env --add channels ryanvolz
    conda config --env --add channels conda-forge
    conda config --env --remove channels defaults
    conda config --env --set channel_priority strict
```
- install conda build tools
```
    mamba install -y -n build conda-build conda-forge-pinning conda-verify
```
- close and reopen the radioconda terminal
- build the gr-sdrplay3 module
```
    mamba activate build
    git clone https://github.com/fventuri/gr-sdrplay3/
    cd gr-sdrplay3
    git checkout conda     # <-- this step is only needed until this branch is merged into the main branch
    conda build .conda\recipe
```
- the last step will take several minutes (10-12 on my computer)
- if successful, it should create a ready to install conda package under 'C:\Users\<username>\radioconda\envs\build\conda-bld\win-64'; the conda package has a long name that begins with 'gnuradio-sdrplay3' and ends with '.tar.bz2'. If you don't see that file there, look at the output from the 'conda build' command and see what failed and why


## step 4 - install the gnuradio-sdrplay3 package

In this step we'll use either the conda package built in the previous step or the prebuilt one available from here: https://github.com/fventuri/gr-sdrplay3/releases

- make sure you are in the base environment; in a radioconda terminal run these commands:
```
    mamba activate base   # (or open a new radioconda terminal)
```
- A. if you built the package from source in the previous step, change directory to the location where you built the package (`cd radioconda\envs\build\conda-bld\win-64`)
- B. if you downloaded the prebuilt package, change directory to the folder where you downloaded it
- install the gnuradio-sdrplay3 package (the full name of the package might be slightly different in the future):
```
    mamba install --offline "gnuradio-sdrplay3-3.11.0.2.post2+ga4071c6-py311hf0272db_0.tar.bz2"
```


## step 5 - use GNU Radio Companion to run some examples

- In the start menu click on GNU Radio -> GNU Radio Companion
- select File -> Open and open a GRC flowgraph (file with the '.grc' extension) in the examples folder `%CONDA_PREFIX%\Library\share\gr-sdrplay3\examples`
- make sure it compiles to Python and it runs without errors


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
