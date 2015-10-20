PROGRAM_EMAIL = email-cli
PROGRAM_UPS = ups-cli
PROGRAM_UPSXX = ups-cli++
PROGRAM_MON = monitor-cli
PROGRAM_MLM = mlm
PROGRAM_MLMZ_JIM = mlmz-jim
PROGRAM_MLM_TEST = mlm-test
PROGRAM_ZYRE_TRIVIAL = zyre-trivial
PROGRAM_MALAMUTEZ = malamute-z

PROGRAMS_C = $(PROGRAM_EMAIL) $(PROGRAM_UPS) $(PROGRAM_ZYRE_TRIVIAL) $(PROGRAM_MLM_TEST) $(PROGRAM_MLMZ_JIM)
# $(PROGRAM_MALAMUTEZ) $(PROGRAM_MLM)
PROGRAMS_CXX = $(PROGRAM_MON) $(PROGRAM_UPSXX)
PROGRAMS = $(PROGRAMS_C) $(PROGRAMS_CXX)

#SOURCES_C = $(addsuffix .c,$(PROGRAMS_C))
#SOURCES_CXX = $(addsuffix .cc,$(PROGRAMS_CXX))

RM = rm -f

CFLAGS = -I./
CXXFLAGS = -std=c++11 -lstdc++ -I./ -Icppzmq/

ifdef DEBUG
CFLAGS += -g
CXXFLAGS += -g
endif

# Common LDFLAGS
LDFLAGS = -lczmq -lzmq -lzyre

# Special linking for some (not all) programs
LDFLAGS_MLM = -lmlm

PHONY = all, clean

all: check-zmq-hpp-presence $(PROGRAMS)

clean:
	$(RM) $(PROGRAMS)

$(PROGRAM_EMAIL): email/email.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(PROGRAM_UPS): ups/ups.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(PROGRAM_ZYRE_TRIVIAL): zyre-trivial-src/zyre-trivial.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(PROGRAM_UPSXX): ups/ups.cc
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

$(PROGRAM_MON): monitor/monitor.cc
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

$(PROGRAM_MLM): mlm.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) $(LDFLAGS_MLM)

$(PROGRAM_MLMZ_JIM): mlmz/mlmz-jim.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) $(LDFLAGS_MLM)

$(PROGRAM_MLM_TEST): temp/test.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) $(LDFLAGS_MLM)

$(PROGRAM_MALAMUTEZ): malamutez/malamute.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) $(LDFLAGS_MLM)

check-zmq-hpp-presence:
	@if (echo '#define __ZMQ_HPP_INCLUDED__'; echo '#include <zmq.hpp>' ) | gcc -E $(CXXFLAGS) -x 'c++' - > /dev/null ; then : ; else RES=$$?; \
	    echo "=============================================================================" >&2; \
	    echo "NOTE: ups.cxx requires zmq.hpp available at" >&2; \
	    echo "  https://github.com/zeromq/cppzmq/raw/master/zmq.hpp" >&2; \
	    echo "Just do 'git checkout --recurse' to get a local copy into ./cppzmq/zmq.hpp" >&2; \
	    echo "=============================================================================" >&2; \
	    exit $$RES; \
	fi

testz: $(PROGRAM_ZYRE_TRIVIAL)
	./$(PROGRAM_ZYRE_TRIVIAL)

test: all
	./$(PROGRAM_EMAIL) & PID_E=$$! && \
	./$(PROGRAM_UPS) & PID_U=$$! && \
	./$(PROGRAM_UPSXX) & PID_UXX=$$! && \
	./$(PROGRAM_MON) & PID_M=$$! && \
	kill $$PID_E $$PID_U $$PID_UXX $$PID_M
