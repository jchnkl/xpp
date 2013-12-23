DIRS=interface implementation

%: ${DIRS}
	@ true

${DIRS}:
	${MAKE} -C $@ ${MAKECMDGOALS}

.PHONY: ${DIRS}
