FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    python3 \
    python3-pip \
    ocl-icd-opencl-dev \
    opencl-headers \
    clinfo \
    libboost-all-dev
    
WORKDIR /app

COPY . .

CMD ["bash"]
