all:
	${MAKE} -C src/proto

examples:
	${MAKE} -C src/examples

.PHONY: ${DIRS}
