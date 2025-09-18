# Optional custom Dockerfile for additional setup
# This extends the base amigadev/crosstools:m68k-amigaos image

FROM amigadev/crosstools:m68k-amigaos

# Install additional development tools
RUN apt-get update && apt-get install -y \
    git \
    cmake \
    make \
    ninja-build \
    gdb \
    valgrind \
    vim \
    nano \
    tree \
    htop \
    curl \
    wget \
    zip \
    unzip \
    && rm -rf /var/lib/apt/lists/*

# Set up environment variables for Amiga development
ENV AMIGA_TOOLCHAIN=/opt/amiga
ENV TOOLCHAIN_PATH=/opt/amiga
ENV CC=m68k-amigaos-gcc
ENV CXX=m68k-amigaos-gcc
ENV AR=m68k-amigaos-ar
ENV STRIP=m68k-amigaos-strip
ENV OBJDUMP=m68k-amigaos-objdump

# Create a non-root user for development
RUN useradd -m -s /bin/bash vscode && \
    usermod -aG sudo vscode && \
    echo "vscode ALL=(ALL) NOPASSWD:ALL" >> /etc/sudoers

# Set the working directory
WORKDIR /workspace

# Switch to the vscode user
USER vscode

# Add helpful aliases
RUN echo 'alias ll="ls -la"' >> ~/.bashrc && \
    echo 'alias la="ls -A"' >> ~/.bashrc && \
    echo 'alias l="ls -CF"' >> ~/.bashrc && \
    echo 'alias agcc="m68k-amigaos-gcc"' >> ~/.bashrc && \
    echo 'alias amake="make CC=m68k-amigaos-gcc"' >> ~/.bashrc

# Display toolchain info on login
RUN echo 'echo "Amiga m68k toolchain ready:"' >> ~/.bashrc && \
    echo 'echo "Compiler: $(m68k-amigaos-gcc --version | head -1)"' >> ~/.bashrc && \
    echo 'echo "Toolchain path: $AMIGA_TOOLCHAIN"' >> ~/.bashrc