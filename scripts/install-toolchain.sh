#! /usr/bin/env bash


# TODO: Uncomment
VER=12.2.MPACBTI-Rel1
# INDEX_URL=https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads
# VER=$(curl -s ${INDEX_URL} | grep -Po '<h4>Version \K.+(?=</h4>)')

TAR=arm-gnu-toolchain-${VER}-x86_64-arm-none-eabi.tar.xz
DOWNLOAD_URL=https://developer.arm.com/-/media/Files/downloads/gnu/${VER}/binrel/${TAR}


echo "Downloading: ${DOWNLOAD_URL}"
TAR_DEST="${HOME}/Downloads/${TAR}"
if [ ! -f "${TAR_DEST}" ]; then
  wget -O "${TAR_DEST}" "${DOWNLOAD_URL}"
else 
  echo "The file: ${TAR_DEST} is already exists."
fi


sudo mkdir /opt/gcc-arm-none-eabi
sudo tar -xvf ${TAR_DEST} --strip-components=1 -C /opt/gcc-arm-none-eabi
echo 'export PATH=$PATH:/opt/gcc-arm-none-eabi/bin' | \
  sudo tee -a /etc/profile.d/gcc-arm-none-eabi.sh

echo "Reboot your system to apply the changes."
