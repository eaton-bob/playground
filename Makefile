PHONY = all, clean

PROGRAMS=email-cli ups-cli ups-clixx monitor-cli  zyre-trivial

all: $(PROGRAMS)

clean:
	$(RM) -f $(PROGRAMS) ups-clixx

email-cli: email/email.c
	$(CC) $(CFLAGS) -o $@ $^

ups-cli: ups/ups.c
	$(CC) $(CFLAGS) -o $@ $^

zyre-trivial: zyre-trivial-src/zyre-trivial.c
	$(CC) $(CFLAGS) -o $@ $^

ups-clixx: ups/ups.cxx
	@echo "$(CXX) $(CXXFLAGS) -o $@ $^"; \
	if $(CXX) $(CXXFLAGS) -o $@ $^; then : ; else RES=$$?; \
	    echo "NOTE: ups.cxx requires zmq.hpp available at https://github.com/zeromq/cppzmq/raw/master/zmq.hpp" >&2; \
	    exit $$RES; \
	fi

monitor-cli: monitor/monitor.cc
	$(CXX) $(CXXFLAGS) -o $@ $^

testz: $(PROGRAM_ZYRE_TRIVIAL)
	./$(PROGRAM_ZYRE_TRIVIAL)

test: all
	./$(PROGRAM_EMAIL) & PID_E=$$! && \
	./$(PROGRAM_UPS) & PID_U=$$! && \
	./$(PROGRAM_UPSXX) & PID_UXX=$$! && \
	./$(PROGRAM_MON) & PID_M=$$! && \
	kill $$PID_E $$PID_U $$PID_UXX $$PID_M
