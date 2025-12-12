#!/bin/bash

# Check if device parameter is provided
if [ -z "$1" ]; then
    echo "Usage: $0 [nanosp|nanox|flex|stax]"
    exit 1
fi

# Validate device parameter
case "$1" in
    nanosp|nanox|flex|stax)
        DEVICE="$1"
        ;;
    *)
        echo "Invalid device. Use one of: nanosp, nanox, flex, stax"
        exit 1
        ;;
esac

# Map nanosp to nanos2 for build path
if [ "$DEVICE" = "nanosp" ]; then
    BUILD_DEVICE="nanos2"
else
    BUILD_DEVICE="$DEVICE"
fi

# Export variables for docker-compose
export DEVICE
export BUILD_DEVICE

docker-compose up -d

# Wait for the container to start
until docker inspect -f '{{.State.Status}}' concordium-ledger-app-nanosp-1 | grep -q "running"; do
    sleep 1
done

docker exec -it concordium-ledger-app-nanosp-1 ./tools/debug.sh "build/${BUILD_DEVICE}/bin/app.elf"

docker-compose down
