DIRS=src src/tests

%: ${DIRS}
	@ true

${DIRS}:
	${MAKE} -C $@ ${MAKECMDGOALS}

.PHONY: ${DIRS}
