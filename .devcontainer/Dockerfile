# Use the official Ubuntu 22.04 image as the base
FROM ubuntu:22.04

# Prevent interactive prompts during package installations
ENV DEBIAN_FRONTEND=noninteractive

################# Common Libraries and Packages #############################
# Update package list and install general dependencies
RUN apt-get update && apt-get install -y \
    python3 \
    python3-pip \
    python3-setuptools \
    python3-dev \
    build-essential \
    git \
    ffmpeg \
    unzip \
    xvfb \
    libsdl2-ttf-2.0-0 \
    libmtdev-dev \
    xclip \
    libgl1-mesa-glx \
    libgles2-mesa \
    libgl1-mesa-dri \
    mesa-utils \
    usbutils \
    libsdl2-dev \
    libsdl2-image-dev \
    libsdl2-mixer-dev \
    libsdl2-ttf-dev \
    libportmidi-dev \
    libswscale-dev \
    libavformat-dev \
    libavcodec-dev \
    zlib1g-dev \
    libgstreamer1.0-dev \
    gstreamer1.0-plugins-base \
    gstreamer1.0-plugins-good \
    gstreamer1.0-plugins-bad \
    gstreamer1.0-plugins-ugly \
    gstreamer1.0-libav \
    gstreamer1.0-tools \
    gstreamer1.0-x \
    gstreamer1.0-alsa \
    build-essential \ 
    cmake \
    openjdk-21-jre \
    libusb-1.0-0-dev \
    wget \
    libssl-dev \
    libcurl4-openssl-dev \
    asciidoc \
    sudo \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*

# Create an alias for python3 as python
RUN ln -s /usr/bin/python3 /usr/bin/python

# Install MegaTools
WORKDIR /tmp
RUN wget https://megatools.megous.com/builds/megatools-1.10.3.tar.gz && \
    tar -xvf megatools-1.10.3.tar.gz
WORKDIR /tmp/megatools-1.10.3
RUN ./configure && \
    make -j$(nproc) && \
    make install && \
    rm -rf /tmp/megatools-1.10.3.tar.gz /tmp/megatools-1.10.3


################# Kivy Installation #############################
# Install Cython and Kivy (full installation)
RUN pip install cython
RUN pip install Kivy[full]==2.3.0

# Install additional libraries for app development
RUN pip install pyserial


################# KiCad Installation #############################
# Install necessary packages and add the PPA for KiCad
RUN apt-get update && \
    apt-get install -y software-properties-common apt-utils && \
    add-apt-repository --yes ppa:kicad/kicad-8.0-releases && \
    apt-get update && \
    apt-get install -y --install-recommends kicad && \
    apt-get clean && rm -rf /var/lib/apt/lists/*


################# C Embedded Software Environment #############################
# Download STM32Cube CTL zip file and set license acceptance
WORKDIR /tmp
RUN /usr/local/bin/megadl https://mega.nz/file/qJ1AUajI#WoG7Ck_5MT9JdClJJZEZqjhBOGZ-t1JspkWK9FozZWw
ENV LICENSE_ALREADY_ACCEPTED=1

# Unzip, install, and clean STM32Cube CTL files
RUN unzip -o en.st-stm32cubeclt_1.16.0_21983_20240628_1741_amd64.deb_bundle.sh.zip && \
    chmod +x st-stm32cubeclt_1.16.0_21983_20240628_1741_amd64.deb_bundle.sh && \
    apt-get update && \
    ./st-stm32cubeclt_1.16.0_21983_20240628_1741_amd64.deb_bundle.sh && \
    apt-get clean && rm -rf /var/lib/apt/lists/* && \
    rm -rf en.st-stm32cubeclt_1.16.0_21983_20240628_1741_amd64.deb_bundle.sh.zip

# Install STM32CubeMX
RUN /usr/local/bin/megadl https://mega.nz/file/6AUSCZCS#XX5y5OA-NYerxH0ThHey89RUEUDdyNNDl6KjllWHYqE && \
    unzip -o en.stm32cubemx-lin-v6-12-1.zip -d /root && \
    printf "1\n1\n\n\n\nY\n\n" | /root/SetupSTM32CubeMX-6.12.1 -console && \
    rm -rf en.stm32cubemx-lin-v6-12-1.zip /root/Readme.html /root/SetupSTM32CubeMX-6.12.1 /root/jre


################# User Configuration #############################
# Create a non-root user 'fbs' with password '1234', grant sudo access without password
RUN useradd -m fbs && echo "fbs:1234" | chpasswd && adduser fbs sudo
RUN echo "%sudo ALL=(ALL) NOPASSWD: ALL" >> /etc/sudoers

# Add user 'fbs' to plugdev and dialout groups for USB access
RUN sudo usermod -aG plugdev fbs && sudo usermod -aG dialout fbs

# Switch to 'fbs' user
USER fbs

# Preload STM32 MCU files to avoid repeated downloads to generate code with STM32CubeMX
WORKDIR /home/fbs/
RUN /usr/local/bin/megadl  https://mega.nz/file/SVElGY4C#QR2NXd1jwOedZNHIM6E2rJ-XiqGLjlQ2m0fLkAeCLmc && \
    unzip -o STM32Cube.zip && \
    rm -rf /home/fbs/STM32Cube.zip

# Set default workdir
WORKDIR /workspaces/STM32WLE5_RF_dongle

# Copy KiCad's default configuration to avoid reconfiguring the entire Kicad environment on every container build
RUN mkdir -p /home/fbs/.config/kicad/8.0/
COPY assets/KiCad/kicad_default_conf/ /home/fbs/.config/kicad/8.0/

# Update .bashrc with necessary exports and aliases
RUN echo 'export PATH=$PATH:~/.local/bin/' >> ~/.bashrc
RUN echo "alias STM32CubeMX='/usr/local/STMicroelectronics/STM32Cube/STM32CubeMX/STM32CubeMX'" >> ~/.bashrc