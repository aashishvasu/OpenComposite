FROM ubuntu:jammy
RUN dpkg --add-architecture i386 && \
	apt-get update && \
	apt-get install -y --no-install-recommends g++ g++-multilib cmake ninja-build python3 libgl-dev libvulkan-dev libgl-dev:i386 libvulkan-dev:i386 && \
	rm -r /var/lib/apt/lists/*
