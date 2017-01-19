# 42ity Current Refactoring Task

## What is this non-sense?

Our team had the fortune to participate in a very good, no-bullshit, Scrum
training.  We learned that once certain amount of points is established for any
given sprint, you don't add new tasks (points) to the sprint. Rather than that,
this time should be spent working on any of the following things:

* work on new user stories (preparation; technical, business related
discussions; specifications etc...) 
* overhead 
* technical debt

This "document" is about the last bullet point - technical debt. 

Here we'll maintain information about the current refactoring task we'd like to
undergo, if such writeup is needed.

## Process

Due to our experience with bad actors we want to make it absolutely clear how
this is going to work. 

1. Each sprint the whole team, through a magical process called voting, will
decide what is the next priority regarding technical debt. 
2. It will be **one** topic, not two.
3. This topic will be written on the whiteboard. The rest of the ideas are wiped.
4. At least the title of "Current topic:" is updated in this document.
5. Optionally, if it's needed, use this document to write down information
about the task (design, etc..).
6. In the rare case when the topic has just been fixed and there is still
reasonably enough time until the end of sprint, this process is repeated.

## Current Topic: Get rid of agent-autoconfig

Agent autoconfig resides in fty-rest.git in `src/agents/autoconfig`.
It performs auto configuration for:
* rule templates
* kpi uptime

### Rule templates
* Relevant code in `TemplateRuleConfigurator.(h|cc)`, templates in
`src/agents/autoconfig/rule_templates`.
* Get rid of the c++ inheritance (Configurator -> RuleConfigurator ->
TemplateRuleConfigurator) and AutoConfigurationInfo class, cut out the relevant
code, and hook it into fty-alert-engine by subscribing to asset messages.
* Don't forget to move directory .../rule_templates into fty-alert-engine repo
and modify packaging accordingly.

### Kpi uptime
* Relevant code in `UptimeConfigurator. (h|cc)`
* There are two possible ways to do this:
  1. move functionality of UptimeConfigurator.(h|cc) into fty-asset and reuse
the protocol `SET/<dc>/<ups^1>/.../<ups^2>` already present in
`fty-kpi-power-uptime`.
  2. move functionality of UptimeConfigurator.(h|cc) into fty-asset and use asset message
to convey required information to `fty-kpi-power-uptime`. For this, you'd use `aux` field of
`fty_proto_t` in the following way, e.g.: name = "DC-Roztoky", aux ["mainups.1"] = "UPS1", ...
