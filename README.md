```
HeatShield Overview

HeatShield is a solution designed to operate the heater in the house. It is an unnessary but fun/pet project I created
to help me try to restore and keep up with my electronics and software engineering skills after the stroke.
The HeatShield solution currently consists of three projects each of which focuses on a specific function needed to
complete solution. These projects are Atlantis, Discovery and Endeavor.

Atlantis
Atlantis is the external interface to the solution. It provides web access for presentation and control. Atlantis
uses C# to provide its services.

Discovery
Discovery is the middleware project that receives data from Endeavor and provides data to Atlantis. Discovery uses a SQL
database to provide its services.

Endeavor
Endeavor is the workhorse of the solution. It receives input through Discovery and controls the physical operation of
the heater. It reports its operation to Discovery for analysis.
Endeavor is the 23-24 project for heater operation is a micro-controller solution based on the Espressif ESP32 micro-controller.
Its high-level language is C/C++, middleware is Python and is assembler at its lowest level.

Odessey
Odessey is to be the 24-25 project intended to replace Endeavor. It is micro-computer project based on the Raspberry Pi.

At this point I think this will be a combination of C/C++ and Python for scripting.
```
