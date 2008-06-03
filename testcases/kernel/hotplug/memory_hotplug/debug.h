// currently unused
/*
 * [ ==========================================================
 * Debug Support:
 */

#ifdef _DEBUG
//TODO -- change to static inline functions...

#define _Developer_Debug glctx.debug

#define DCALL(F,X) if (_Developer_Debug & DBG_##F) X
#define DPRINTF(F,X) if (_Developer_Debug & DBG_##F) _dprintf X
#define	STATIC
#define DENTER(F) DPRINTF(F, ("%s() entered.\n",fname))
#define DEXIT(F)  DPRINTF(F, ("%s() exiting.\n",fname))
#define DEXIT_STATUS(F,S) \
	DPRINTF(F, ("%s() returning status = %d\n", fname, S))


/*
 *     _Developer_Debug - enable/disable debug messages
 *
 *     = 0 turn off debug messages (Normal Case).
 *
 *     OR in one or more DBG_* definitions [see below] to enable
 *     debug messages..
 */

#define DBG_INOUT	0x0001	/* display function entry/exit */
#define DBG_ERRORS	0x0002	/* display various error conditions */
#define DBG_INFO  	0x0004	/* display other debug info */
#define DBG_ARGS  	0x0008

#define DBG_BUFS  	0x0010	/* display buffer configuration info */
#define DBG_XMIT    0x0020	/* display buffer transmits */
#define DBG_RECV    0x0040	/* display buffer receipts */

#define DBG_ALL (DBG_INOUT|DBG_ERRORS|DBG_INFO|DBG_ARGS)

#else  /* !def _DEBUG */

#define DCALL
#define DPRINTF(F,X)
#define	STATIC static
#define DENTER(F)
#define DEXIT(F)
#define DEXIT_STATUS(F,S)

#endif /* _DEBUG */

/*
 * always define FNAME macro for use in non-debug messages
 */
#define FNAME(NAME)	static const char *fname = #NAME

/*
 * End of Debug Support
 * ] ==========================================================
 */

