FROM debian:sid-slim

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && \
    apt-get install -y --no-install-recommends \
    gcc-13 \
    make \
    libc6-dev \
    valgrind && \
    update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-13 100 && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY . .

CMD ["sh", "-c", "make --silent && exec ./dash"]
