/*
  Define the protocol structure to be used by NetPIPE for PVM.
  */

typedef struct protocolstruct ProtocolStruct;

struct protocolstruct
{
    /* Keep track of our task id */
    int     mytid;

    /* Keep track of the other's task id */
    int     othertid;
};


/*
  Undefine one of the following to determine the type of data
  encoding for the PVM message passing.

  DataDefault means that PVM uses XDR encoding which ensures that
  the data can be packed / unpacked across non-homogeneous machines.

  If you know that the machines are the same, then you can use DataRaw
  and save some time.

  DataInPlace means that the data is not copied at pack time, but is
  copied directly from memory at send time.
  */

#define PVMDATA     PvmDataDefault
/* #define PVMDATA     PvmDataRaw */
/* #define PVMDATA     PvmDataInPlace */
