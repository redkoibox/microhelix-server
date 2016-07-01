# micro-helix powered container image.
# @author Fabio Greotti <fabio.greotti@iseocom.it>
FROM ubuntu:16.04
MAINTAINER Fabio Greotti <fabio.greotti@iseocom.it>
ADD microhelix.tar.gz /usr/local/
COPY launch-microhelix.sh /usr/local/microhelix
ENTRYPOINT /usr/local/microhelix/launch-microhelix.sh