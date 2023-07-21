# STM32L071CZT6 HelloWorld!

### Preparation

```bash
sudo apt-get install \
  libncurses-dev \
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

Update your programmer firmware using `CubeProgrammer`

```
Found 1 stlink programmers
  version:    V2J42S7
  serial:     030030000800003550334D4E
  flash:      0 (pagesize: 0)
  sram:       0
  chipid:     0x000
  dev-type:   unknown
```

#### Toolchain

```bash
./scripts/install-toolchain.sh
```

Reboot the system, then:

#### Build & Debug

```bash
mkdir build && cd build
cmake -DCMAKE_TOOLCHAIN_FILE=../crosstool.cmake ..
make clean all
make flash
make openocd & make gdb
```
