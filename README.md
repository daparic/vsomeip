### vsomeip

##### Copyright
Copyright (C) 2015-2017, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)

##### License

This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

##### vsomeip Overview
----------------
The vsomeip stack implements the http://some-ip.com/ (Scalable service-Oriented
MiddlewarE over IP (SOME/IP)) protocol. The stack consists out of:

* a shared library for SOME/IP (`libvsomeip3.so`)
* a second shared library for SOME/IP's service discovery (`libvsomeip3-sd.so`)
  which is loaded during runtime if the service discovery is enabled.

##### Build Instructions for Linux

###### Dependencies

- A C++11 enabled compiler like gcc >= 4.8 is needed.
- vsomeip uses CMake as buildsystem.
- vsomeip uses Boost >= 1.55:

Ubuntu 14.04:

`sudo apt-get install libboost-system1.55-dev libboost-thread1.55-dev libboost-log1.55-dev`

Ubuntu 12.04: a PPA is necessary to use version 1.54 of Boost:
-- URL: https://launchpad.net/~boost-latest/+archive/ubuntu/ppa
--`sudo add-apt-repository ppa:boost-latest/ppa`
--`sudo apt-get install libboost-system1.55-dev libboost-thread1.55-dev
    libboost-log1.55-dev`

For the tests Google's test framework https://code.google.com/p/googletest/[gtest] in version 1.7.0 is needed.
-- URL: https://googletest.googlecode.com/files/gtest-1.7.0.zip

To build the documentation asciidoc, source-highlight, doxygen and graphviz is needed:
--`sudo apt-get install asciidoc source-highlight doxygen graphviz`

###### Compilation

For compilation call:

```bash
mkdir build
cd build
cmake ..
make
```

To specify a installation directory (like `--prefix=` if you're used to autotools) call cmake like:
```bash
cmake -DCMAKE_INSTALL_PREFIX:PATH=$YOUR_PATH ..
make
make install
```

###### Compilation with predefined unicast and/or diagnosis address
To predefine the unicast address, call cmake like:
```bash
cmake -DUNICAST_ADDRESS=<YOUR IP ADDRESS> ..
```

To predefine the diagnosis address, call cmake like:
```bash
cmake -DDIAGNOSIS_ADDRESS=<YOUR DIAGNOSIS ADDRESS> ..
```
The diagnosis address is a single byte value.

###### Compilation with custom default configuration folder
To change the default configuration folder, call cmake like:
```bash
cmake -DDEFAULT_CONFIGURATION_FOLDER=<DEFAULT CONFIGURATION FOLDER> ..
```
The default configuration folder is /etc/vsomeip.

###### Compilation with custom default configuration file
To change the default configuration file, call cmake like:
```bash
cmake -DDEFAULT_CONFIGURATION_FILE=<DEFAULT CONFIGURATION FILE> ..
```
The default configuration file is /etc/vsomeip.json.

###### Compilation with signal handling

To compile vsomeip with signal handling (SIGINT/SIGTERM) enabled, call cmake like:
```bash
cmake -DENABLE_SIGNAL_HANDLING=1 ..
```
In the default setting, the application has to take care of shutting down vsomeip in case these signals are received.


##### Build Instructions for Android

###### Dependencies

- vsomeip uses Boost >= 1.55. The boost libraries (system, thread and log) must be included in the Android source tree and integrated into the build process with an appropriate Android.bp file.

###### Compilation

In general for building the Android source tree the instructions found on the pages from the Android Open Source Project (AOSP) apply (https://source.android.com/setup/build/requirements).

To integrate the vsomeip library into the build process, the source code together with the Android.bp file has to be inserted into the Android source tree (by simply copying or by fetching with a custom platform manifest).
When building the Android source tree, the Android.bp file is automatically found and considered by the build system.

In order that the vsomeip library is also included in the Android image, the library has to be added to the PRODUCT_PACKAGES variable in one of a device/target specific makefile:

```
PRODUCT_PACKAGES += \
    libvsomeip \
    libvsomeip_cfg \
    libvsomeip_sd
```
----------------
##### Build Instructions for Visual Studio 2019/2022 IDE

```
cd /d/
mkdir -p src/dxlab/ local 

# Download, double-click then install to D:\local\boost_1_76_0\
wget https://boost.teeks99.com/bin/1.76.0/boost_1_76_0-msvc-14.2-64.exe 

# Clone two github repositories
cd src/dxlab/
git clone https://github.com/google/googletest.git
git clone https://github.com/daparic/vsomeip.git

# Open DOS "Visual Studio Development Command Prompt for VS2019"
set BOOST_ROOT=D:\local\boost_1_76_0\
set BOOST_INCLUDEDIR=D:\local\boost_1_76_0\
set BOOST_LIBRARYDIR=D:\local\boost_1_76_0\lib64-msvc-14.2
set GTEST_ROOT=D:\src\dxlab\googletest\
cd vsomeip
git checkout dxlab
mkdir build/
cd build
cmake ..

# Open Visual Studio 2019 IDE and open the generated vsomeip.sln solution file
# Select projects together: vsomeip_ctrl, vsomeip3, vsomeip3-cfg, vsomeip3-e2e, vsomeip3-sd, ZERO_CHECK
# and set accordingly the value for "Platform Toolset". For example, "Visual Studio 2019 (v142)".  
# Then build of these projects should produce their corresponding DLLs.

# Build "application_test" and "Set As Startup Project". Locate and copy the following DLLs and 
# put beside this exe:
vsomeip3-cfg.dll
vsomeip3-e2e.dll
vsomeip3-sd.dll
vsomeip3.dll
boost_filesystem-vc142-mt-gd-x64-1_76.dll

# Set breakpoints in application_test.cpp and press F5 to start debugging.
```


