CC=gcc
CFLAGS=-m32

C4=xc
BIN=${C4}.out
SRC=${C4}.c
L=bear --

${BIN}: ${SRC}
	${CC} ${CFLAGS} -o $@ $<

.PHONY: l
l: ${SRC}
	${L} ${CC} ${CFLAGS} -o ${BIN} $<

.PHONY: clean
test: ${BIN}
	./${BIN} hello.c

.PHONY: clean
clean:
	rm *.out *.o *.d -rf

# bootstrap
.PHONY: bs
bs: ${BIN}
	./${BIN} ${SRC}
