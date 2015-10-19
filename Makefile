PROGRAM_EMAIL = email-cli
PROGRAM_UPS = ups-cli
PROGRAM_UPSXX = ups-cli++
PROGRAM_MON = monitor-cli
PROGRAM_ZYRE_TRIVIAL = zyre-trivial

PROGRAMS_C = $(PROGRAM_EMAIL) $(PROGRAM_UPS) $(PROGRAM_ZYRE_TRIVIAL)
PROGRAMS_CXX = $(PROGRAM_MON) $(PROGRAM_UPSXX)
PROGRAMS = $(PROGRAMS_C) $(PROGRAMS_CXX)

#SOURCES_C = $(addsuffix .c,$(PROGRAMS_C))
#SOURCES_CXX = $(addsuffix .cc,$(PROGRAMS_CXX))

CFLAGS = -lczmq -lzmq
CXXFLAGS = -lczmq -lzmq -std=c++11 -lstdc++

# Addition for special cases
CFLAGS_ZYRE = -lzyre

PHONY = all, clean

all: $(PROGRAMS)

clean:
	$(RM) -f $(PROGRAMS)

$(PROGRAM_EMAIL): email/email.c
	$(CC) $(CFLAGS) -o $@ $^

$(PROGRAM_UPS): ups/ups.c
	$(CC) $(CFLAGS) -o $@ $^

$(PROGRAM_ZYRE_TRIVIAL): zyre-trivial-src/zyre-trivial.c
	$(CC) $(CFLAGS) $(CFLAGS_ZYRE) -o $@ $^

$(PROGRAM_UPSXX): ups/ups.cxx
	@echo "$(CXX) $(CXXFLAGS) -o $@ $^"; \
	if $(CXX) $(CXXFLAGS) -o $@ $^; then : ; else RES=$$?; \
	    echo "NOTE: ups.cxx requires zmq.hpp available at https://github.com/zeromq/cppzmq/raw/master/zmq.hpp" >&2; \
	    exit $$RES; \
	fi

$(PROGRAM_MON): monitor/monitor.cc
	$(CXX) $(CXXFLAGS) -o $@ $^

testz: $(PROGRAM_ZYRE_TRIVIAL)
	./$(PROGRAM_ZYRE_TRIVIAL)

test: all
	./$(PROGRAM_EMAIL) & PID_E=$$! && \
	./$(PROGRAM_UPS) & PID_U=$$! && \
	./$(PROGRAM_UPSXX) & PID_UXX=$$! && \
	./$(PROGRAM_MON) & PID_M=$$! && \
	kill $$PID_E $$PID_U $$PID_UXX $$PID_M
