struct iovec;
extern void fillbuf(char *buf, int count, char value);
extern void vfillbuf(struct iovec *iv, int vcnt, char value);
extern int filecmp(char *f1, char *f2);
extern int bufcmp(char *b1, char *b2, int bsize);
extern int vbufcmp(struct iovec *iv1, struct iovec *iv2, int vcnt);

extern int forkchldrn(int **pidlst, int numchld, int action, int (*chldfunc)());
extern int waitchldrn(int **pidlst, int numchld);
extern int killchldrn(int **pidlst, int numchld, int sig);


