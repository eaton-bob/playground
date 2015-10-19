PROGRAM_EMAIL = email-cli
PROGRAM_UPS = ups-cli
PROGRAM_MON = monitor-cli

PROGRAMS_C = $(PROGRAM_EMAIL) $(PROGRAM_UPS)
PROGRAMS_CXX = $(PROGRAM_MON)
PROGRAMS = $(PROGRAMS_C) $(PROGRAMS_CXX)

#SOURCES_C = $(addsuffix .c,$(PROGRAMS_C))
#SOURCES_CXX = $(addsuffix .cc,$(PROGRAMS_CXX))

CFLAGS = -lczmq -lzmq
CXXFLAGS = -lczmq -std=c++11 -lstdc++

#PHONY = all, clean

all: $(PROGRAMS)

clean:
	$(RM) -f $(PROGRAMS)

$(PROGRAM_EMAIL): email/email.c
	$(CC) $(CFLAGS) -o $@ $<

$(PROGRAM_UPS): ups/ups.c
	$(CC) $(CFLAGS) -o $@ $<

$(PROGRAM_MON): monitor/monitor.cc
	$(CXX) $(CXXFLAGS) -o $@ $<

test: all
	./$(PROGRAM_EMAIL) & PID_E=$$! && \
	./$(PROGRAM_UPS) & PID_U=$$! && \
	./$(PROGRAM_MON) & PID_M=$$! && \
	kill $$PID_E $$PID_U $$PID_M
