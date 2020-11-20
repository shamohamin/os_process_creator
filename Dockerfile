FROM gcc:4.9

ENV WORKPLACE=~/Documents
# just for caution
RUN apt-get -y update && apt-get install -y build-essential make

WORKDIR ${WORKPLACE}

COPY . ${WORKPLACE}

RUN gcc -v

EXPOSE 8080

CMD ["make", "run"]