#!/bin/sh

if [ $# -ne 1 ]
then
    echo "USAGE: $(basename "$0") FILE";
    echo "assembles FILE and runs the resuling binary in an x86 environment";
    exit 1
fi

if [ "$(uname -m)" = "arm64" ]
then
    container_runtime="docker"
else
    container_runtime="podman"
fi

"$container_runtime" run --rm -it \
    --platform "linux/amd64" \
    -v "$(pwd -P):/id2202:z,ro" \
    docker.io/johnwikman/id2202:minimal \
    bash -c " nasm -felf64 -o /root/obj.o /id2202/$1 \
           && gcc-10 -z noexecstack -no-pie -o /root/a.out /root/obj.o \
           && /root/a.out"
