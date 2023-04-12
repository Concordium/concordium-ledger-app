# Build: docker build -t concordium/ledger-app-builder .

# Some setup for running might be required: https://developers.ledger.com/docs/nano-app/load/#2-load-the-application-from-inside-the-container-image
# Run: docker run --rm -ti -v "$(realpath .):/app" --privileged concordium/ledger-app-builder

# Run commands from makefile (load, delete, release, etc.)

FROM ghcr.io/ledgerhq/ledger-app-builder/ledger-app-builder:latest

RUN apk add --update zip

ENV WORK_DIR=/app
ENV TARGET=nanos

CMD ["/bin/bash"]
