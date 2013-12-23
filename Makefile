DIRS=src

%: ${DIRS}
	@ true

${DIRS}:
	${MAKE} -C $@ ${MAKECMDGOALS}

.PHONY: ${DIRS}
