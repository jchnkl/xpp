include ../flags.makefile

# find ${PROTODIR} -iname '*.xml' -exec basename {} .xml \;
# without ge, xf86vidmode
PROTOS= \
	present \
	dpms \
	shm \
	damage \
	dri3 \
	render \
	xfixes \
	screensaver \
	bigreq \
	xprint \
	xinerama \
	xkb \
	xc_misc \
	record \
	xproto \
	xvmc \
	dri2 \
	xinput \
	sync \
	randr \
	xtest \
	res \
	shape \
	xevie \
	glx \
	xv \
	xselinux \
	xf86dri \
	composite

PYTHON=python2
CLIENT=cpp_client.py
XCBGEN=/usr/lib/python3.3/site-packages
PROTODIR=/usr/share/xcb
XMLPROTOS=$(PROTOS:%=%.xml)
HPPPROTOS=$(XMLPROTOS:%.xml=%.hpp)

all: ${XMLPROTOS}

gch: ${HPPPROTOS}

${HPPPROTOS}:
	-${CXX} ${CXXFLAGS} ${LDFLAGS} -c $(if $(filter $@,xproto.hpp),x.hpp,$(@))

${XMLPROTOS}:
	-${PYTHON} ${CLIENT} -p ${XCBGEN} ${PROTODIR}/$@ > $(if $(filter $@,xproto.xml),x.hpp,$(@:%.xml=%.hpp))

find:
	@ find ${PROTODIR} -iname '*.xml' -exec basename {} .xml \;

clean:
	rm -f ${PROTOS:%=%.hpp}
	rm -f $(PROTOS:%=%.hpp.gch)

.PHONY: clean find ${XMLPROTOS} ${HPPPROTOS}
