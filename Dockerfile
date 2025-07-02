# Use Ubuntu base image
FROM ubuntu:22.04

# Install necessary packages
RUN apt-get update && apt-get install -y \
    clang \
    lld \
    valgrind \
    python3 \
    python3-pip \
    python3-dev \
    build-essential \
    && apt-get clean

COPY requirements.txt /app/
RUN pip3 install -r /app/requirements.txt


# Set working directory
WORKDIR /app

# Copy project files into container
COPY . /app

# Set ASAN options (optional but useful)
ENV ASAN_OPTIONS=detect_leaks=1:log_path=asan_log:exitcode=1

# Default command when container starts
CMD ["/bin/bash"]
