FROM --platform=$BUILDPLATFORM ubuntu:22.04

# The following line is needed to set the $TARGETARCH variable to amd64 or arm64 depending on the target architecture. If you need alternative formulations (x86_64 or aarch64) use $(arch) in a RUN command.
ARG TARGETARCH

# This can be changed to "ci" to remove developer tools and reduce image size. Currently this only saves about 200MB of space
ARG CONTAINER_MODE=dev

# The version of clang to use
ARG CLANG_VERSION=17

# Setup container environment
ENV \
    # Change locale to suppress some warnings
    LC_ALL=C \
    #
    # Set VSCode as default editor
    EDITOR="code --wait" \
    GIT_EDITOR="code --wait"

# Setup container
RUN \
    #
    # Disable apt interactive prompts for this RUN command
    export DEBIAN_FRONTEND="noninteractive" && \
    #
    # Get release information (used to determine the correct URL for the LLVM apt repository)
    . /etc/lsb-release && \
    #
    # Get package list
    apt-get update && \
    #
    # Add LLVM/Clang repository so we can install newer versions of clang
    apt-get install -y software-properties-common gnupg curl && \
    curl -L https://apt.llvm.org/llvm-snapshot.gpg.key | apt-key add - && \
    apt-add-repository "deb http://apt.llvm.org/$DISTRIB_CODENAME/ llvm-toolchain-$DISTRIB_CODENAME-$CLANG_VERSION main" && \
    # 
    # Update again now that we've added the LLVM repository
    apt-get update && \
    # 
    # Install required packages
    apt-get install -y \
      #
      # The following packages are used to build CIRCT
      ccache \
      # cmake \ (Using custom build)
      make \
      ninja-build \
      clang-$CLANG_VERSION \
      clang-tools-$CLANG_VERSION \
      clang-format-$CLANG_VERSION \
      clang-tidy-$CLANG_VERSION \
      clangd-$CLANG_VERSION \
      lld-$CLANG_VERSION \
      lldb-$CLANG_VERSION \
      #
      # The following packages are required by Swift
      binutils \
      git \
      gnupg2 \
      libc6-dev \
      libcurl4-openssl-dev \
      libedit2 \
      libgcc-9-dev \
      libpython3.8 \
      libsqlite3-0 \
      libstdc++-9-dev \
      libxml2-dev \
      libz3-dev \
      pkg-config \
      tzdata \
      unzip \
      zlib1g-dev \
      #
      # The following packages are needed by support scripts
      jq \
      rsync \
      #
      # sourcekit-lsp dependencies
      libsqlite3-dev \
      libncurses5-dev \
      python3 \
      ninja-build \
      #
      # ShellCheck is useful for validating bash scripts
      shellcheck \
    && \
    #
    # Remove GCC (installed as a dependency of CMake) and the MSVC version of clang to reduce CMake confusion
    apt remove -y gcc && \
    apt autoremove -y && \
    rm /usr/bin/clang-cl-$CLANG_VERSION && \
    #
    # Create suffix-free links to clang binaries
    for FILE in $(find /usr/bin -name "*-$CLANG_VERSION"); do ln -s ${FILE} ${FILE%-$CLANG_VERSION}; done && \
    #
    # Install Swift
    case $TARGETARCH in \
      "amd64") \
        export SWIFT_ARCH_SUFFIX="" \
        ;; \
      "arm64") \
        export SWIFT_ARCH_SUFFIX="-aarch64" \
        ;; \
      *) \
        echo "Unsupported TARGETARCH: $TARGETARCH" && \
        exit 1 \
        ;; \
    esac && \
    mkdir /swift && \
    curl -sL "https://download.swift.org/swift-5.9.1-release/ubuntu2204${SWIFT_ARCH_SUFFIX}/swift-5.9.1-RELEASE/swift-5.9.1-RELEASE-ubuntu22.04${SWIFT_ARCH_SUFFIX}.tar.gz" | tar xz --strip-components=1 -C /swift && \
    #
    # Link interesting Swift binaries to /usr/bin (don't link everything since Swift ships with an old version of clang and friends)
    ln -s /swift/usr/bin/swift /usr/bin/swift && \
    ln -s /swift/usr/bin/swiftc /usr/bin/swiftc && \
    ln -s /swift/usr/bin/sourcekit-lsp /usr/bin/sourcekit-lsp && \
    #
    # Verilator hardcodes "g++" as the C++ compiler, so we symlink clang++ to g++ to avoid having gcc installed
    cd /usr/bin && ln -s clang++-$CLANG_VERSION g++ && \
    #
    # Based on the container mode, either remove unnecessary packages or install additional developer-oriented packages
    case "$CONTAINER_MODE" in \
      "ci") \
        #
        # Remove packages we only needed for this step
        apt-add-repository -r "deb http://apt.llvm.org/$UBUNTU_VERSION_NAME/ llvm-toolchain-$UBUNTU_VERSION_NAME-$CLANG_VERSION main" && \
        apt remove -y software-properties-common gnupg curl && \
        #
        # Clean up apt to reduce image size
        apt autoremove -y && \
        rm -rf /var/lib/apt/lists/* \
        ;; \
      "dev") \
        # Install additional development tools
        apt-get install -y git \
        ;; \
      *) \
        echo "Invalid CONTAINER_MODE: $CONTAINER_MODE" && \
        exit 1 \
        ;; \
    esac

#
# Install CMake
RUN \
  git clone https://gitlab.kitware.com/etcwilde/cmake -b ewilde/swiftly-remodelling /cmake && \
  cd /cmake && \
  ./bootstrap -- -DCMAKE_USE_OPENSSL=OFF && make &&  make install
