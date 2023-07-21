# STM32L071CZT6 HelloWorld!


```bash
./scripts/install-toolchain.sh
```

Reboot the system, then:

```bash
mkdir build && cd build
cmake -DCMAKE_TOOLCHAIN_FILE=../crosstool.cmake ..
```
