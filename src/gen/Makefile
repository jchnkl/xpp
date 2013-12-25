PROTOS=xproto

PYTHON=python2
CLIENT=cpp_client.py
XCBGEN=~/src/xcb/proto/xcbgen
PROTODIR=~/src/xcb/proto/src

%: ${PROTOS}
	@ true

${PROTOS}:
	${PYTHON} ${CLIENT} -p ${XCBGEN} ${PROTODIR}/$@.xml > $@_requests.hpp

.PHONY: ${PROTOS}
