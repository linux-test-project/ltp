/*
  Define the protocol structure to be used by NetPIPE for MPI.

  $Id: MPI.h,v 1.1 2003/02/05 15:44:54 robbiew Exp $
  */

typedef struct protocolstruct ProtocolStruct;
struct protocolstruct
{
	int nbor, iproc;
};

