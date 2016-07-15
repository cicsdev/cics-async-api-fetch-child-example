/* Licensed Materials - Property of IBM                               */
/*                                                                    */
/* SAMPLE                                                             */
/*                                                                    */
/* (c) Copyright IBM Corp. 2016 All Rights Reserved                   */
/*                                                                    */
/* US Government Users Restricted Rights - Use, duplication or        */
/* disclosure restricted by GSA ADP Schedule Contract with IBM Corp   */

#include <stdio.h>

long reason = 0;
long response = 0;

void main()
{
  struct container {
    int num;
  } numCon;

  long len_numCon = sizeof(numCon);

  EXEC CICS ADDRESS EIB(dfheiptr)
                    RESP(response) RESP2(reason)
                    NOHANDLE;

  if (response != 0) {
    EXEC CICS RETURN;
  }

  EXEC CICS GET CONTAINER("num")
                INTO(&numCon) FLENGTH(len_numCon)
                RESP(response) RESP2(reason);

  numCon.num += 1;

  printf("ASCHILD:  incremented number\n");

  EXEC CICS PUT CONTAINER("num")
                FROM(&numCon) FLENGTH(len_numCon) BIT
                RESP(response) RESP2(reason);

  EXEC CICS RETURN;
}
