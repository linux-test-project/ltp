/*
  Define the protocol structure to be used by NetPIPE for MPI.

  $Id: MPI.h,v 1.1 2002/03/18 21:39:34 robbiew Exp $
  */

typedef struct protocolstruct ProtocolStruct;
struct protocolstruct
{
	int nbor, iproc;
};

