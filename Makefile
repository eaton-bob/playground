
PHONY = all, clean

all: email-cli ups-cli monitor-cli

clean:
	rm email-cli ups-cli monitor-cli

email-cli: email/email.c
	gcc -lczmq -lzmq email/email.c -o email-cli

ups-cli: ups/ups.c
	gcc -lczmq -lzmq ups/ups.c -o ups-cli

monitor-cli: monitor/monitor.cc
	gcc -lczmq -lzmq monitor/monitor.cc -o monitor-cli
