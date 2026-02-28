.PHONY: dev-shell mini-shell clean
# Automatically use docker if on ARM
ifeq ($(shell uname -m),arm64)
CONTAINER_RUNTIME ?= docker
else
CONTAINER_RUNTIME ?= podman
endif
# Enter a ID2202 development shell
dev-shell:
	$(CONTAINER_RUNTIME) run --rm -it \
	--hostname id2202-shell \
	--name id2202-shell \
	-v "$$(pwd -P):/id2202:z" \
	docker.io/johnwikman/id2202 bash

# Enter a minimal ID2202 emulated x86 development shell
mini-shell:
	$(CONTAINER_RUNTIME) run --rm -it \
	--hostname id2202-mini-shell \
	--name id2202-mini-shell \
	--platform "linux/amd64" \
	-v "$$(pwd -P):/id2202:z" \
	-w /id2202 \
	docker.io/johnwikman/id2202:minimal \
	bash

# Removes the container images from your system
clean:
	$(CONTAINER_RUNTIME) rmi -f "docker.io/johnwikman/id2202"
	$(CONTAINER_RUNTIME) rmi -f "docker.io/johnwikman/id2202:minimal"
