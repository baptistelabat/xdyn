#!/bin/sh
docker run -w /data -v $(pwd):/data -it --rm deploy-xdyn xdyn $*
