# STM32L0 HelloWorld!

This project is configured for target device: `STM32L071CZT6`. Use 
`STMCubeIDE` to generate startup assembly: `stm32l0xxx.s` and flash linker 
script: `tm32l0xxx_flash.ld` for other devices.

### Preparation

```bash
sudo apt-get install \
  libncurses-dev \
  libusb-dev \
  libncursesw5 \
  openocd
```

#### STLink programmer utilities

```bash
git clone git@github.com:pylover/stlink.git
cd stlink
make clean
make release
sudo make install
sudo ldconfig
```

##### STlink v2 firmware version

Update your programmer firmware using `CubeProgrammer` if required:

```
st-info --probe
Found 1 stlink programmers
  version:    V2J42S7
  serial:     030030000800003550334D4E
  flash:      196608 (pagesize: 128)
  sram:       20480
  chipid:     0x447
  dev-type:   STM32L0xxx_Cat_5
```

#### Toolchain

Install the latest ARM toolchain.

```bash
./scripts/install-toolchain.sh
```

Reboot the system, then:

#### Build & Debug

```bash
./rollup.sh
cd build
make clean all
make flash
```

##### Analize *.elf file

```bash
make analyze
```

##### On-chip debug using GDB

First, start `OpenOCD` server:
```bash
make openocd
```

Then, run `make gdb` in another terminal.

```bash
make gdb
```

#### Lint

```bash
pip install prettyc
cd build
make lint
```
