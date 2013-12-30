include ../flags.makefile

CXXFLAGS+=-g

CPPSRCS=event.cpp \
        requests.cpp \
        iterator.cpp

all: ${CPPSRCS}

${CPPSRCS}:
	${CXX} ${LDFLAGS} ${CXXFLAGS} -o $(@:%.cpp=%) $@

version:

clean:
	rm -f ${CPPSRCS:%.cpp=%}

.PHONY: ${CPPSRCS} clean
