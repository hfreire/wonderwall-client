FROM resin/rpi-raspbian:jessie

MAINTAINER hfreire@exec.sh

RUN apt-get update && \
    DEBIAN_FRONTEND=noninteractive apt-get install -y \
        librabbitmq1 \
        libcurl3 \
        libjansson4 \
        libsdl1.2 \
        libsdl-image1.2

ENV WONDERWALL_HOME /opt/wonderwall
ENV PATH $WONDERWALL_HOME/bin:$PATH

RUN mkdir -p $WONDERWALL_HOME/bin
COPY build/wonderwalld $WONDERWALL_HOME/bin/wonderwalld
RUN chmod +x $WONDERWALL_HOME/bin/wonderwalld

COPY share/docker/run.sh /run.sh
RUN chmod +x /run.sh

CMD ["/run.sh"]