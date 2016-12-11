FROM resin/rpi-raspbian:jessie

MAINTAINER hfreire@exec.sh

RUN apt-get update && \
    DEBIAN_FRONTEND=noninteractive apt-get install -y \
        librabbitmq1 \
        libcurl3 \
        libjansson4 \
        fbi

ENV WONDERWALL_HOME /opt/wonderwall
ENV PATH WONDERWALL_HOME/bin:$PATH

RUN mkdir -p $WONDERWALL_HOME/bin
COPY build/wonderwalld $WONDERWALL_HOME/bin/wonderwalld
RUN chmod +x $WONDERWALL_HOME/bin/wonderwalld

COPY share/docker/start.sh /start.sh
RUN chmod +x /start.sh

CMD ["/start.sh"]