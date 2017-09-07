## Notes on FTY Refactoring 2017

### Mailbox Protocol
* fty_proto.xml - new mailbox protocol based on Gerald's proposal send by email on 25.8.2017

### Automatic Generation of FTY Documentation
* feature provided by zproject
* docs are generated during compilation in .txt and .doc (we just don't do it, our doc/ contains some templates)
* asciidoc and xmlto are packages needed
* it is generated from .h and .c files by default is includes also self-tests as a example of usage
* .h and .c file should contain tags @interface/head/discuss/... - @end (automaticaly done by zproject)
* manual intervention will be needed to add some info that cannot be retrieved from code


* fty_asset_server.txt - sample of file generated from fty-asset

## Agents - Configuration
* fty_agents_configuration.csv - description of agents configuration in simple table

## Agents - Interface
* fty_agents_headers.csv - description of agents headers in simple table
