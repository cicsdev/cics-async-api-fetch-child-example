# How to use the CICS Asynchronous API Commands

In [an earlier article][prad], Prad introduced the new asynchronous API commands
available in the CICS TS 5.4 Open Beta. In this article we're going to get straight down
to business: we'll develop a very simple pair of programs to demonstrate the passing of
information from a parent to a child program using the new functionality. The full source
code for these programs is [in a GitHub repository][github] for you to get up and running
quickly.

The example, written in C, is very simple. The parent reads an integer from a CICS
terminal screen, and passes this value to its child. The child (running asynchronously
with the parent) takes this value and increments it by 1, before passing it back to the
parent. To show this working, we'll print various statements. I've defined the following
resources in my CICS region's CSD:

    DEFINE PROGRAM(ASPARENT) GROUP(ASYNC) STATUS(ENABLED)
    DEFINE TRANSACTION(ASPA) GROUP(ASYNC) PROGRAM(ASPARENT)
        
    DEFINE PROGRAM(ASCHILD) GROUP(ASYNC) STATUS(ENABLED)
    DEFINE TRANSACTION(ASCH) GROUP(ASYNC) PROGRAM(ASCHILD)

A basic principle of this kind of concurrency model is that we should treat the different
computational entities in the system as discrete systems that have their own non-shared
memory, and communicate by passing messages. This type of model will be the subject of a
future article, but for now all we need to know is that it means we can write the parent
and child programs separately, as long as we agree on the interfaces. We'll start with
the parent program, and then the child program.

## The Parent Program

Since this example is highlighting the two new API commands, I'll skip over the screen
reading parts of the program. We just need to assume that when we invoke the parent
program by running a transaction from a CICS terminal, we'll also pass the name of the
child transaction to run, as well as an integer. We'll start with a struct to store this
integer (which we pull from the screen input):

    struct container {
      int num;
    };
    struct container numCon;

Not strictly necessary (just the int on its own would suffice), but I've used a struct to
make it very easy to extend. Let's print that integer to screen, and store the struct in
a container:

    printf("ASPARENT: sending %d\n", numCon.num);
    
    EXEC CICS PUT CONTAINER("num") CHANNEL("AS")
                      FROM(&numCon)  FLENGTH(len_numCon) BIT
                      RESP(response) RESP2(reason);

The name of both the container and its channel is arbitrary. As this is C, don't forget
that the `FROM` parameter wants the _address_ of the struct in memory. Now, here's some
new functionality:

    EXEC CICS RUN TRANSID(input.child) ASYNCHRONOUS CHILD(child)
              CHANNEL("AS") RESP(response) RESP2(reason);

Here, `input.child` is the name of the child transaction, read from the screen. For
example, `ASCH`. The `CHILD` parameter specifies an output field for CICS to store a
child token. Much like for channel names, this is a variable of length `char[16]`. We
also specify a channel name as the way to pass information to the child program. Note
that here CICS makes a copy of the channel, and sends that rather than the original
channel. This way, the parent is free to keep making changes in its channel, and there's
no name confusion.

The `EXEC CICS RUN TRANSID` command is non-blocking, so the parent is free to continue
with other logic while the child program does its own processing. At the parent program's
convenience, it can fetch the results from the child:

    EXEC CICS FETCH CHILD(child)
                    ABCODE(abcode)
                    COMPSTATUS(child_status)
                    CHANNEL(fetch_chan)
                    RESP(response) RESP2(reason);

The `FETCH` command will block until the child has completed: either normally, or with an
abend. Once control is given back to the parent program, it can be sure that the child
has completed. `child` is an input field: just specify the variable you used on the
corresponding `RUN`. We can collect the abend code (if there is one; blanks if not), and
the completion station of the child (stored in a `long`). The `CHANNEL` parameter
specifies an output field here: this is to ensure we don't clobber over an existing
channel that the parent is using.

If the command worked without issue (check the `reason` as usual), we can
confirm that the child completed normally, and get its container (in the channel
specified by the `FETCH` command):

    if (reason == 0) {
      switch (child_status) {
        case DFHVALUE(NORMAL):
          EXEC CICS GET CONTAINER("num") CHANNEL(fetch_chan)
                        INTO(&numCon) FLENGTH(len_numCon)
                        RESP(response) RESP2(reason);
          printf("ASPARENT: received %d\n", numCon.num);
    ...

In this case, I'm just writing the result straight back into the struct that we started
with. I then print this result out (which should have been incremented by 1).

## The Child Program

Carrying on from what I said at the start of the article, the child program can be
written separately (you might say, concurrently...) to the parent program, as long as
we agree on the interfaces. In this example, the interface to agree on is that we're
communicating with channels and containers, so we must agree on the structure of the data
being passed, and the name of the container. In the child program, let's start by
defining the same struct as in the parent:

    struct container {
        int num;
    } numCon;

The first thing we need to do is get the container that we're going to be passed by the
parent:

    EXEC CICS GET CONTAINER("num")
                  INTO(&numCon) FLENGTH(len_numCon)
                  RESP(response) RESP2(reason);

Now, let's increment the int in this struct by 1, and print that value:

    numCon.num += 1;
    printf("ASCHILD:  incremented number\n");

Finally, we can put that struct back into the container (replacing the old value), to
send it back to the parent:

    EXEC CICS PUT CONTAINER("num")
                  FROM(&numCon) FLENGTH(len_numCon) BIT
                  RESP(response) RESP2(reason);

  And that's all there is to it. The important bit is that you can treat the child as a
  unit of its own. As long as the interfaces stay the same, the implementation of the
  child can change and the parent doesn't need to know or care.

## Running the Programs

If we compile these programs, we can start the parent from a CICS terminal by invoking
its transaction:

      ASPA ASCH 9

Since we used `printf`, the output will be written to `CEEOUT` for the region running
these programs. Checking there, we see the following messages printed:

      ASPARENT: sending 9
      ASCHILD:  incremented number
      ASPARENT: received 10

Which shows that it worked! The parent set the value of its integer to 9
initially (from terminal input), and passed it to the child. The child read this
value, incremented it by 1, before returning that new number to the parent. A
very simple example, but it shows how easy it is to get going with these new
APIs.

Don't forget that you can find the complete source code on [GitHub][github] for
this example, and you can read more about these APIs in the
[CICS TS Knowledge Center][kc].


[prad]: http://developer.ibm.com/cics
[github]: https://github.com/cicsdev/cics-async-api-fetch-child-example
[kc]: https://www.ibm.com/support/knowledgecenter/SSGMCP_5.4.0/fundamentals/asynchronous/async-api.html?pos=2
