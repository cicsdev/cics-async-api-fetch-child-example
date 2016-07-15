# CICS Asynchronous API Fetch Child Example

This project is a very basic example demonstrating the passing of information
from a parent to a child program using the CICS Asynchronous API, written in C.

## Overview

The project consists of two programs: a parent program, and a child program (run
by the parent).

The parent reads an integer from a 3270 terminal screen, and passes this value
to its child. The child---running asynchronously with the parent---takes this
value and increments it by 1, before passing it back to the parent. These values
are output to `CEEOUT`.

## Example

Assuming the following definitions in a CICS region's CSD:

    DEFINE PROGRAM(ASPARENT) GROUP(AS) STATUS(ENABLED)
    DEFINE TRANSACTION(ASPA) GROUP(AS) PROGRAM(ASPARENT)
    
    DEFINE PROGRAM(ASCHILD) GROUP(AS) STATUS(ENABLED)
    DEFINE TRANSACTION(ASCH) GROUP(AS) PROGRAM(ASCHILD)

the parent program is invoked with transaction `ASPA`, and will call the named
child transaction. Pass an integer after it too. From a CICS terminal, run:

    ASPA ASCH 9

Checking `CEEOUT` for the region running these programs, we see the following
messages printed:

    ASPARENT: sending 9
    ASCHILD:  incremented number
    ASPARENT: received 10

## License

This project is licensed under [Apache License Version 2.0](LICENSE).
