FROM mcr.microsoft.com/devcontainers/base:ubuntu-24.04

# Defines target architecture to support multi-platform builds.
ARG TARGETARCH

# Prevent interactive prompts during package installations.
ENV DEBIAN_FRONTEND=noninteractive

################################################################################
# Common packages
################################################################################

# MBSE requires Graphviz's dot command for its local web viewer.
RUN apt-get update && apt-get install -y \
    curl \
    git \
    wget \
    xz-utils \
    sudo \
    locales \
    python3 \
    python3-pip \
    python3-dev \
    python3-setuptools \
    python-is-python3 \
    graphviz \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*
    
# Install common Python tooling globally in the devcontainer so it is available
# to the whole workspace without introducing an additional virtual environment
# layer.  --break-system-packages is intentional here because this is a controlled
# development container image rather than a shared host Python installation.
RUN python -m pip install --break-system-packages \
    Jinja2==3.1.6 \
    jsonschema==4.26.0 \
    markdown-it-py==4.0.0 \
    mdit-py-plugins==0.5.0 \
    pytest==9.1.1 \
    ruff==0.15.20 \
    mbse

# Configure locale (UTF-8).
ENV LANG=C.UTF-8
ENV LC_ALL=C.UTF-8

################################################################################
# Software domain
################################################################################

# udev is required by VS Code Serial Monitor extension
# https://github.com/microsoft/vscode-serial-monitor/issues/137#issuecomment-1804710798
RUN apt-get update && apt-get install -y \
    build-essential \
    automake \
    bison \
    cmake \
    clang-18 \
    clang-format \
    clang-tidy \
    doxygen \
    flex \
    gdb \
    gcc-14-plugin-dev \
    libstdc++-14-dev \
    lld-18 \
    llvm-18 \
    llvm-18-dev \
    udev \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*

# Download and install AFL++ from the GitHub release tarball
ARG AFLPP_VERSION=4.40c
RUN set -eux; \
    cd /tmp; \
    curl -fsSLO "https://github.com/AFLplusplus/AFLplusplus/archive/refs/tags/v${AFLPP_VERSION}.tar.gz"; \
    tar -xzf "v${AFLPP_VERSION}.tar.gz"; \
    cd "AFLplusplus-${AFLPP_VERSION}"; \
    make source-only LLVM_CONFIG=llvm-config-18; \
    make install; \
    rm -rf "/tmp/AFLplusplus-${AFLPP_VERSION}" "/tmp/v${AFLPP_VERSION}.tar.gz"

# Download and install the upstream AFLplusplus/cov-analysis helper used by the
# source-coverage VS Code task.  This script wraps LLVM source coverage replay
# and report generation for AFL++ corpora.
ARG COV_ANALYSIS_REF=v1.0
RUN set -eux; \
    curl -fsSL "https://raw.githubusercontent.com/AFLplusplus/cov-analysis/${COV_ANALYSIS_REF}/cov-analysis" \
      -o /usr/local/bin/cov-analysis; \
    chmod 0755 /usr/local/bin/cov-analysis

# Download and install ARM GNU Toolchain.
ARG ARM_GCC_VERSION=15.2.rel1
RUN set -eux; \
    arch="${TARGETARCH:-$(dpkg --print-architecture)}"; \
    case "${arch}" in \
      amd64) arm_gcc_arch="x86_64" ;; \
      arm64) arm_gcc_arch="aarch64" ;; \
      *) echo "Unsupported architecture for ARM GNU Toolchain: ${arch}"; exit 1 ;; \
    esac; \
    arm_gcc_archive="arm-gnu-toolchain-${ARM_GCC_VERSION}-${arm_gcc_arch}-arm-none-eabi.tar.xz"; \
    wget "https://developer.arm.com/-/media/Files/downloads/gnu/${ARM_GCC_VERSION}/binrel/${arm_gcc_archive}"; \
    tar -xf "${arm_gcc_archive}" -C /opt; \
    ln -sfn "/opt/arm-gnu-toolchain-${ARM_GCC_VERSION}-${arm_gcc_arch}-arm-none-eabi" "/opt/arm-gnu-toolchain-arm-none-eabi"; \
    rm "${arm_gcc_archive}"

# Add toolchain to PATH.
ENV PATH="/opt/arm-gnu-toolchain-arm-none-eabi/bin:${PATH}"

# Install software-domain Python tooling globally in the devcontainer so it is
# available to the whole workspace without introducing an additional virtual
# environment layer.  --break-system-packages is intentional here because this
# is a controlled development container image rather than a shared host Python
# installation.
RUN python -m pip install --break-system-packages \
    cmakelang==0.6.13

################################################################################
# Hardware domain
################################################################################
    
# Install KiCad.
RUN apt-get update && \
    apt-get install -y software-properties-common apt-utils && \
    add-apt-repository --yes ppa:kicad/kicad-8.0-releases && \
    apt-get update && \
    apt-get install -y --install-recommends kicad && \
    apt-get clean && rm -rf /var/lib/apt/lists/*

################################################################################
# AI support with OpenCode
################################################################################

# xdg-open is required by OpenCode.
RUN apt-get update && apt-get install -y \
    xdg-utils \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*

# Switch to container user so AI tooling installs in the correct user paths.
USER vscode
ENV HOME=/home/vscode

# Install OpenCode.
RUN curl -fsSL https://opencode.ai/install | bash

# Create OpenCode state directory for vscode user.
RUN mkdir -p /home/vscode/.local/state
