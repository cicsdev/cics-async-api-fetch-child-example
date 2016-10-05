/* Licensed Materials - Property of IBM                               */
/*                                                                    */
/* SAMPLE                                                             */
/*                                                                    */
/* (c) Copyright IBM Corp. 2016 All Rights Reserved                   */
/*                                                                    */
/* US Government Users Restricted Rights - Use, duplication or        */
/* disclosure restricted by GSA ADP Schedule Contract with IBM Corp   */

#include <stdio.h>
#include <stdlib.h>

long reason = 0;
long response = 0;

void main()
{
  char child??(16??);
  char fetch_chan??(16??);
  char abcode??(4??);
  long child_status;

  struct screen {
    char transid??(4??);
    char space;
    char child??(4??);
    char space2;
    char num??(10??);
  } input;

  short len_input = sizeof(input);

  struct container {
    int num;
  };

  struct container numCon;
  long len_numCon = sizeof(numCon);

  EXEC CICS ADDRESS EIB(dfheiptr)
                    RESP(response) RESP2(reason);

  if (response != 0) {
    EXEC CICS RETURN;
  }

  /* Get the value from the terminal, and convert it to an int. */

  EXEC CICS RECEIVE INTO(&input) LENGTH(len_input);

  numCon.num = atoi(input.num);

  /* Put the struct containing the number to be passed to the child
     program in a container, within a channel. */

  printf("ASPARENT: sending %d\n", numCon.num);

  EXEC CICS PUT CONTAINER("num") CHANNEL("AS")
                    FROM(&numCon)  FLENGTH(len_numCon) BIT
                    RESP(response) RESP2(reason);

  /* Start the child transaction named on input, passing it the channel
     we've just created. CICS will fill in the child token for us. CICS
     will copy the channel before passing it to the child, so it can
     still be used by the parent. */

  EXEC CICS RUN TRANSID(input.child) CHILD(child)
            CHANNEL("AS") RESP(response) RESP2(reason);

  if (response != 0) {
    EXEC CICS ABEND;
  }

  /* The parent is free to carry on with its own logic here, while the
     child gets on with its own work. When the parent requires the
     results from the child, it can fetch them.

     Here we specify which child's results we're wanting to fetch, and
     CICS will tell us which channel it's put the results in. (This is
     to ensure we don't overwrite any changes in the original channel,
     made by the parent after it started the child.) */

  EXEC CICS FETCH CHILD(child)
                  ABCODE(abcode)
                  COMPSTATUS(child_status)
                  CHANNEL(fetch_chan)
                  RESP(response) RESP2(reason);

  /* If the command worked without issue, confirm the status of the
     child transaction. If it completed normally, we can get its results
     (stored in the channel that fetch told us about). In this case, we
     just overwrite the struct we used at the start.

     Proper error checking should be employed here, but for the purposes
     of this example, if we find any problems we'll just abend this
     parent transaction. */

  if (response == 0) {
    switch (child_status) {
      case DFHVALUE(NORMAL):
        EXEC CICS GET CONTAINER("num") CHANNEL(fetch_chan)
                      INTO(&numCon) FLENGTH(len_numCon)
                      RESP(response) RESP2(reason);
        printf("ASPARENT: received %d\n", numCon.num);
        break;
      case DFHVALUE(ABEND):
        EXEC CICS ABEND;
      default:
        EXEC CICS ABEND;
    }
  } else {
    EXEC CICS ABEND;
  }

  EXEC CICS RETURN;
}
