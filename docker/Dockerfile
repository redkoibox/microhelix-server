# micro-helix powered container image.
# @author Fabio Greotti <f.greotti@redkoibox.it>
FROM ubuntu:16.04
LABEL maintainer="Fabio Greotti <f.greotti@redkoibox.it>"
RUN apt-get update && apt-get install -y libsasl2-2 libgcc1 libstdc++6
ADD microhelix.tar.gz /usr/local/
WORKDIR /usr/local/microhelix
VOLUME [ "/usr/local/microhelix/scripts" ]
EXPOSE 8080
ENTRYPOINT ["bin/microhelix"]
