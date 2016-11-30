# Problem: naming conventions for "42ity" project

Naming should cover
 * library name
 * C functions
 * repo / project name
 * package name
 * daemon name
 * cli tool(s) name
 * path names
 * can spill over to systemd unit names (services, timers, etc.) - although several units may be defined for different deliverables of the same source-code repo (e.g. both an agent or server and some house-keeping timer)

# FTY prefix

## Rationale
**FTY** is a shortcut of 42ity, which is short and can be used as C identifier.

The next goal is to remove repetitive and redundant word agent. It have a little information, plus programname in unix is limited to 15 characters, so wasting the first X characters will disable rsyslog filtering.

See bios-agent-nut and bios-agent-nut-configurator. If we'll use longer name (snmp), the programname for both agents will be the same!!

## Example repo

    tree example.git/src/
    example.git/src/
    ├── fty_example_classes.h
    ├── fty_example_selftest
    ├── fty_example_selftest.c
    ├── fty-example
    ├── fty_example.c
    ├── fty-example.cfg.in
    ├── fty-example.service.in
    ├── fty_example_server.c
    ├── libfty_example.la
    └── libfty_example.pc.in

## C functions

    fty_proto_t *msg = fty_proto_new ();
    zactor_t *server = zactor_new (fty_rt_server, NULL);
    zmsg_t *msg = fty_smtp_encode (uuid, to, subject, body, NULL);



# [WIP] Candidates
 * etn_
 * joe_
 * ipm_
 * inf_
 * jmi_
 * xmi_
 * XLII_
 * fortuity_
 * fotify_
 * e
 * eipi_
 * pmi_

# Ideas from AQU
-	Don’t touch binaries names (apps and shared libs), apart if they have “bios” in the name
-	Modify only package names to expose “42ity”, including packages descriptions

- Rule:
    - Lower case “42ity” in the names, upper case “42ITy” for the descriptions and texts
        - Example:
            - agent-asset => 42ity-agent-asset
                - Description: 42ITy - Assets management agent
            - core => 42ity-core
                - Description: 42ITy - Core functionality
            - libbiosproto => lib42ityproto    
                - maybe the “proto” part should be reworded?! => lib42ity-agent or lib42ity-protocol
- system units
    - probably good to expose 42ity in these names... to be discussed

- repository names 
    - not sure if we need to rename to include 42ity (lower case, as for the package name), probably not (apart from libbiosproto...).

# Ideas from Jana:
- repository
    - complete name without any abbreviations or prefixes (like agent-, kpi-, ipc-)
    - rename epfl to proxy
agent - <repo>-agent
server - <repo>-server
CLI - 
    * etn-pi-<name>
    * etn-pmi-<name>
    * etn-ipc-<name>
    * etnipc-<name>
    * eipi-<name>
    (from Karol)
    * etn_<name>_cli
    * joe_<name>_cli

