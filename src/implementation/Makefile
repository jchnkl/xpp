LIBS=xcb
CXXFLAGS=-g -std=c++11 -Wall -O3 $(shell pkg-config --cflags ${LIBS})
LDFLAGS=$(shell pkg-config --libs ${LIBS})

HPPSRCS=$(shell find . -name '*.hpp')
HPPOBJS=$(HPPSRCS:%.hpp=%.hpp.gch)

all: ${DIRS} ${HPPOBJS}

%.hpp.gch: %.hpp
	${CXX} ${CXXFLAGS} -MMD -c $<

-include $(HPPSRCS:%.hpp=%.d)

version: ${LIBS}
	@echo -e "\nGCC version:"
	@gcc -v 2>&1

${LIBS}:
	@printf "%-20s" "$@:"
	@pkg-config --modversion $@

clean:
	rm -f ${HPPOBJS}

.PHONY: clean version
