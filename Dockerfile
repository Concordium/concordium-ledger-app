# Build: docker build -t concordium/ledger-app-builder .

# Some setup for running might be required: https://developers.ledger.com/docs/nano-app/load/#2-load-the-application-from-inside-the-container-image
# Run: docker run --rm -ti -v "$(realpath .):/app" --privileged concordium/ledger-app-builder

# Run commands from makefile (load, delete, release, etc.)

FROM ghcr.io/ledgerhq/ledger-app-builder/ledger-app-builder:sha-229b03c

RUN apt-get update --allow-releaseinfo-change && apt-get upgrade -qy && \
    apt-get install -qy zip

ENV NANOS_SDK=/app/nanos-secure-sdk
ENV NANOS_PLUS_SDK=/app/nanosplus-secure-sdk
ENV NANOX_SDK=/app/nanox-secure-sdk

ENV BOLOS_SDK=${NANOS_SDK}

CMD ["/bin/bash"]
