# ID2202 Student Repository

This is your private Git repository, where you will submit all assignments in the course. For information on how to use this repository, see the [Assignment Tasks](https://canvas.kth.se/courses/50168/pages/assignment-tasks) page on Canvas. Please read those instructions carefully before starting to work on your assignments.

## Setting up your development environment

Our proposed development environment involves using either Podman or Docker. After the course starts, any complementary instructions will be posted on the [Container-Based Environment](https://canvas.kth.se/courses/50168/pages/container-based-environment) page on Canvas.

The instructions assume that you have `make` available on your system.

### Install Docker/Podman

#### Windows

1. Enable [Windows Subsystem for Linux (WSL)](https://learn.microsoft.com/en-us/windows/wsl/install).
2. Follow the instructions for Linux.

#### Linux and Intel Mac

Install [Podman](https://podman.io/docs/installation).

#### ARM Mac

Install [Docker Desktop](https://www.docker.com/products/docker-desktop/).

### Workflow (all)

In this directory, run

```
make dev-shell
```

This will spawn a container with the development environment and mount the current working directory with read-write access to `/id2202` inside the container. This means that changes to files and folders are synced between your current working directory and `/id2202` in the container. The first time you run this command it will take some time to download the image. If you want to leave the container, you can exit by pressing `Ctrl-D`.

We suggest that you edit source code in your editor of choice on your host system. You can use the container to compile and run your compiler or x86 assembly code.

#### Working with x86 assembly on ARM Mac

This processor uses an ARM architecture and we are compiling to x86 so you need to use another container that emulates x86 when assembling and running the assembly code in modules 2 and 3. The reason for not emulating the whole dev-shell is that emulation is slow. We provide a script, `assemble-and-run.sh`, that you should run on your host (i.e., NOT inside the container) to assemble and run an assembly file.

First, make the script executable with

```
chmod +x assemble-and-run.sh
```

Then, you can assemble and run the resulting binary for any x86 assembly file `FILE` with

```
./assemble-and-run.sh FILE
```

The first time you run this command it will take a little time to download the container. You can test it with

```
./assemble-and-run.sh template-projects/asm/main-asm.asm
```

which should print

```
Hello world!
```

##### Interactive shell

You can also spawn an interactive shell to the emulated x86 environment, where you have access to `make`, `nasm`, and `gcc`, by running

```
make mini-shell
```

Inside *mini-shell* you will be able to build [template-projects/asm](./template-projects/asm) with

```
cd template-projects/asm
make clean && make
```

and run the resulting `main-asm` with

```
./main-asm
```

which again should print

```
Hello world!
```

## Removing and getting the latest container images

The container images we work with in this course are located [here](https://hub.docker.com/r/johnwikman/id2202). To remove the images from your system run

```
make clean
```

To get the latest container images you can just run `make clean` followed by any of the commands above which will then pull the latest image.

## Additional resources

It is not necessary for this course, but if you are interested, here is additional documentation on working with containers in [docker](https://docs.docker.com/manuals/) and [podman](https://docs.podman.io/en/latest/).
