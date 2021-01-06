FROM gcc:4.9

ENV WORKPLACE=~/Documents/app
# insalling depedencies
RUN apt-get install -y make git

COPY . ${WORKPLACE}

WORKDIR ${WORKPLACE}

RUN gcc -v

EXPOSE 8080

CMD ["make", "run"]