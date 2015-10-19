#include <zyre.h>

int main(int argc, char**argv) {
	zsys_info("NOTE: If you do not see any peers, try\n"
		"    export ZSYS_INTERFACE=br1\n"
		"  (whatever is correct for you) and re-run %s", argv[0]);
	char *znodename = "jim";
	char *zchatname = "BIOS";
	int i;

	for (i=1; i<argc; i++) {
		if (streq(argv[i],"-n")) {
			assert ( (i+1) < argc);
			znodename = argv[++i];
		} else if (streq(argv[i],"-c")) {
			assert ( (i+1) < argc);
			zchatname = argv[++i];
		} else {
			printf("Usage: %s [-n nodename] [-c chatname]\n", argv[0]);
			exit(1);
		}
	}

	zyre_t *node = zyre_new(znodename);
	assert(node);
	zyre_start(node);
	zyre_join(node,zchatname);

	while (!zsys_interrupted) {
		zyre_event_t *event = zyre_event_new (node);
		if (!event)
			break ;
		zyre_event_print(event);
		zyre_event_destroy(&event);
	}
	zyre_destroy(&node);
	return 0;
}

