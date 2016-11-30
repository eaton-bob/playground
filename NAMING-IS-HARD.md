# Problem: naming conventions for "42ity" project

Naming should cover
 * library name
 * C functions
 * repo / project name
 * daemon name
 * cli tool(s) name
 * path names
 * can spill over to systemd unit names (services, timers, etc.) - although several units may be defined for different deliverables of the same source-code repo (e.g. both an agent or server and some house-keeping timer)


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

# Ideas from AQU
-	Don’t touch binaries names (apps and shared libs), apart if they have “bios” in the name
-	Modify only package names to expose “42ity”, including packages descriptions
o	Rule:
	Lower case “42ity” in the names, upper case “42ITy” for the descriptions and texts
o	Example:
	agent-asset => 42ity-agent-asset
•	Description: 42ITy - Assets management agent
	core => 42ity-core
•	Description: 42ITy - Core functionality
	libbiosproto => lib42ityproto    
•	maybe the “proto” part should be reworded?! => lib42ity-agent or lib42ity-protocol
-	system units
o	probably good to expose 42ity in these names… to be discussed
-	repository names 
o	not sure if we need to rename to include 42ity (lower case, as for the package name), probably not (apart from libbiosproto…).
