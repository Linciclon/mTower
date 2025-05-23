# ![](docs/images/icon/mTower-logo-81_128.png) mTower

## Feel free to run BW example by following the next steps:

### Step #0 - Turn on Putty
Open a terminal and call putty in sudo to open serial port /dev/ttyUS0, baudrate 115200:
```
sudo putty 
```

### Step #1 - Connect UART wires
Connect:
- Tx (FTDI)---> Rx (Board) PIN 119
- Rx (FTDI)---> Tx (Board) PIN 118

### Step #1 - Connect to the boards
Connect USB to board and look to putty port;

### Step #2 - Compile mTower and BW together
On other terminal compile the components through the following commands:
```
cd <path to mTower> 
make clean
make PLATFORM=numaker_pfm_m2351 create_context
make toolchain
make
```
### Step #3 - FLASH Board NUVOTON

Witth openocd flash the board. It will flash two parts: part 1 BL2 in 0x0 and part 2 BL3 in  0x10040000 at the end we expect to see FreeRTOS running on Putty;

```
<path to openocd folder>/OpenOCD-Nuvoton/src/openocd -f <path to nulink folder>/OpenOCD-Nuvoton/tcl/interface/nulink.cfg -f <path to config>/OpenOCD-Nuvoton/tcl/target/numicroM23.cfg -c "init; reset halt; numicro M2351_erase; flash write_image <path to mTower>/mTower/mtower_s.bin; flash write_image <path to mTower>/mTower/mtower_ns.bin 0x10040000; exit"
```

You should see a BW option to be called from CA of FreeRTOS;



[![Build](https://github.com/samsung/mtower/workflows/Build/badge.svg)](https://github.com/samsung/mtower/actions?query=workflow%3ABuild)
[![CII Best Practices](https://bestpractices.coreinfrastructure.org/projects/6108/badge)](https://bestpractices.coreinfrastructure.org/projects/6108)
[![OpenSSF Scorecard](https://api.securityscorecards.dev/projects/github.com/Samsung/mTower/badge)](https://api.securityscorecards.dev/projects/github.com/Samsung/mTower)
[![cpp-linter](https://github.com/cpp-linter/cpp-linter-action/actions/workflows/cpp-linter.yml/badge.svg)](https://github.com/cpp-linter/cpp-linter-action/actions/workflows/cpp-linter.yml)
[![RepoSize](https://img.shields.io/github/repo-size/samsung/mtower.svg)](https://github.com/samsung/mtower)
[![Release](https://img.shields.io/github/v/release/samsung/mtower.svg)](https://github.com/samsung/mtower/releases)
[![LICENSE](https://img.shields.io/github/license/samsung/mtower.svg)](https://github.com/samsung/mtower/blob/master/LICENSE)

## Contents
1. [Introduction](#1-introduction)
2. [License](#2-license)
3. [Platforms supported](#3-platforms-supported)
4. [Get and build mTower software](#4-get-and-build-mtower-software)
5. [Source code structure](#5-source-code-structure)
6. [Coding standards](#6-coding-standards)
7. [Documentation](#7-documentation)
8. [Contributing](#8-contributing)

## 1. Introduction
The `mTower` is a new Trusted Execution Environment (TEE) specially designed
to protect size-constrained IoT devices based on Cortex-m23 MCU. Usage mTower
pre-embedded into the microcontroller, a module developer can use a simple SDK
that based on Global Platform API standards to add security to their solution.

---
## 2. License
mTower software consists of multiple components that are individually available
under different licensing terms. Terms for each individual file are listed at
the beginnings of corresponding files; also, all licenses are listed in
[COPYING] file.

--- 
## 3. Platforms supported

| **NuMaker-PFM-M2351** <br> Cortex-M23 | **M2351-Badge** <br> Cortex-M23 | **V2M-MPS2** <br> Cortex-M33 (Qemu) | **SparkFun RedBoard** <br> RISC-V |
|:----------------------:|:--------------------------:|:-------------:|:-----------:|
|[![](docs/images/platforms/numaker_pfm_m2351/numaker_pfm_m2351.png)](docs/numaker_pfm_m2351.md)|[![](docs/images/platforms/m2351_badge/m2351_badge.png)](docs/m2351_badge.md) |[![V2M-MPS2](docs/images/platforms/v2m-mps2/v2m-mps2.png)](docs/v2m-mps2-qemu.md)|[![](docs/images/platforms/sparkfun_redboard/sparkfun_redboard.png)](docs/sparkfun_redboard.md)|
| **Pine64 Ox64** <br> RISC-V | **How to add a platform** |||
|[![](docs/images/platforms/pine64_ox64/pine64-ox64.jpg)](docs/pine64_ox64.md)|[![](docs/images/platforms/add_new_board.jpg)](docs/port-new-platform.md)|||

Several platforms are supported. In order to manage slight differences
between platforms, a `PLATFORM` flag has been introduced.

| Platform                  | Composite PLATFORM flag            | Maintained |
|---------------------------|------------------------------------|------------|
| [NuMaker-PFM-M2351]       |`PLATFORM=numaker_pfm_m2351`        | v0.6.0     |
| [M2351-Badge]             |`PLATFORM=m2351_badge`              | v0.6.0     |
| [V2M-MPS2]                |`PLATFORM=mps2_an505_qemu`          | v0.6.0     |
| [SparkFun RED-V RedBoard] |`PLATFORM=sparkfun_redboard`        | v0.6.0     |
| [Pine64 Ox64]             |`PLATFORM=pine64_ox64`              | v0.6.0     |

For information on adding a new platform see the [how to add a platform].

---
## 4. Get and build mTower software
Please see [build] for instructions how to run mTower on various devices.

---
## 5. Source code structure
The general [source code structure] for mTower is similar to the structure of the
multy platforms source code.

---
## 6. Coding standards
In this project we are trying to adhere to the mTower coding convention 
(see [CodingStyle]). However there are a few exceptions that we had to make since
the code also uses other open source components.

---
## 7. Documentation
There is a brief overall [functionality description](docs/mtower_functionality_description.md) of mTower. Other mTower documentation for the project is located in the [docs] folder. The latest version of the specification that describes the mTower source code can be generated using [doxygen] tool from command line. To generate documentation, use

```sh
make docs_gen
```
command, and to view generated docs use

```sh
make docs_show
```
> Note that documentation on mTower is work in progress, and right now doxygen does not provide much documentation.

---
## 8. Contributing
If you want to contribute to the mTower project and make it better, your help is
very welcome. Contributing is also a great way to learn more about social
coding on Github, new technologies and and their ecosystems. [How to contribute
you can find here](.github/CONTRIBUTING.md).

---

[docs]: ./docs
[COPYING]: COPYING
[build]: docs/build.md
[how to add a platform]: docs/port-new-platform.md
[CodingStyle]: docs/mtower-coding-standard.md
[source code structure]: docs/source-code-structure.md
[doxygen]: http://www.doxygen.nl
[NuMaker-PFM-M2351]: http://www.nuvoton.com.cn/hq/products/iot-solution/iot-platform/numaker-maker-platform/numaker-pfm-m2351?__locale=en
[M2351-Badge]: docs/schemes/m2351_badge
[V2M-MPS2]: https://developer.arm.com/documentation/100964/1114/Microcontroller-Prototyping-System-2?lang=en
[SparkFun RED-V RedBoard]: https://www.sparkfun.com/products/15594
[Pine64 Ox64]: https://wiki.pine64.org/wiki/Ox64
