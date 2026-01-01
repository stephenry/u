ARG VARIANT="jammy"
FROM mcr.microsoft.com/vscode/devcontainers/base:0-${VARIANT}
RUN apt-get update \
  && DEBIAN_FRONTEND=noninterative \
    apt-get install --no-install-recommends -y \
      autoconf \
      bc \
      bison \
      build-essential \
      default-jre \
      flex \
      lcov \
      libfl-dev \
      libgoogle-perftools-dev \
      help2man \
      pkg-config \
      gdb \
      cmake \
      python3-dev \
      python3-orderedmultidict \
      python3-pip \
      python3-psutil \
      python3-venv \
      clangd-14 \
      clang-tidy-14 \
      clang-format-14 \
      tclsh \
      swig \
      uuid-dev \
      zlib1g-dev \
      libreadline-dev \
      yosys \
      curl \
      jq \
      tar \
      wget \
      libeigen3-dev \
      tcl-dev \
      tcl-tclreadline \
  && apt-get clean \
  && rm -rf /var/lib/apt/lists/*

# Switch to vscode user.
USER vscode
WORKDIR $HOME

# Skywater 130nm PDK

# Install volare as user and add its bin to PATH
RUN pip install --user volare \
    && echo 'export PATH="$HOME/.local/bin:$PATH"' >> ~/.bashrc \
    && export PATH="$HOME/.local/bin:$PATH" \
    && volare enable --pdk sky130 78b7bc32ddb4b6f14f76883c2e2dc5b5de9d1cbc
ENV PATH="$HOME/.local/bin:${PATH}"

# synlig - SystemVerilog compatible Yosys
RUN wget https://github.com/chipsalliance/synlig/releases/download/2024-12-10-2d838ed/synlig-2d838ed-2024-12-10.tar.gz \
  && tar -xzf synlig-2d838ed-2024-12-10.tar.gz \
  && rm synlig-2d838ed-2024-12-10.tar.gz
ENV SYNLIG_ROOT=$HOME/synlig
