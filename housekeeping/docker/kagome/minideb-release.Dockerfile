FROM bitnami/minideb:buster

WORKDIR /kagome
ENV PATH $PATH:/kagome

RUN apt-get update && \
    apt-get install -y --no-install-recommends software-properties-common && \
    add-apt-repository -y "deb http://deb.debian.org/debian testing main" && \
    apt-get update && \
    apt-get install --no-install-recommends -y libstdc++6 libc6 curl libnsl2 && \
    rm -rf /var/lib/apt/lists/*

COPY kagome /usr/local/bin/
