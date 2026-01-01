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

# Switch to vscode user
USER vscode
WORKDIR $HOME

# Create a tools directory (writable and clean)
RUN mkdir tools && cd tools

# Skywater 130nm PDK
RUN pip install --user volare \
    && echo 'export PATH="$HOME/.local/bin:$PATH"' >> ~/.bashrc \
    && export PATH="$HOME/.local/bin:$PATH" \
    && volare enable --pdk sky130  # Or keep your hash if it works

ENV PATH="$HOME/.local/bin:${PATH}"
ENV PDK_ROOT=$HOME/.volare

# Synlig - Download and extract to the tools subdir
RUN cd tools \
    && wget https://github.com/chipsalliance/synlig/releases/download/2024-12-10-2d838ed/synlig-2d838ed-2024-12-10.tar.gz \
    && tar -xzf synlig-2d838ed-2024-12-10.tar.gz \
    && rm synlig-2d838ed-2024-12-10.tar.gz

ENV SYNLIG_ROOT=$HOME/tools/synlig
