
PHONY = all, clean

all: email ups-cli monitor-cli

clean:
	rm email ups-cli monitor-cli

email: email.c
	gcc -lczmq -lzmq email.c -o email

ups-cli: ups/ups.c
	gcc -lczmq -lzmq ups/ups.c -o ups-cli

monitor-cli: monitor/monitor.cc
	gcc -lczmq -lzmq monitor/monitor.cc -o monitor-cli
