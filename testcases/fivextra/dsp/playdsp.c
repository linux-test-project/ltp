#include <stdlib.h>
#include <stdio.h>
 
#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <linux/soundcard.h>
 
main()
{
int fd, tmp;
int size;
short sample;
char data;
 
fd=open("/dev/dsp", O_WRONLY );
if (fd<0) {
       fprintf(stderr,"Error opening /dev/dsp \n");
       return;
}

tmp=16;                       /* 16 bits by sample */
if (ioctl(fd, SNDCTL_DSP_SAMPLESIZE, &tmp)<0 || tmp!=16) {
       fprintf(stderr,"Error setting SNDCTL_DSP_SAMPLESIZE \n");
       return;
}

tmp=0;                        /* Mono */
if ((ioctl(fd, SNDCTL_DSP_STEREO, &tmp)<0) || tmp!=0) {
       fprintf(stderr,"Error setting SNDCTL_DSP_STEREO\n");
       return;
}
     
tmp=16000;    /* Sample Rate */
if (ioctl(fd, SNDCTL_DSP_SPEED, &tmp)<0 || tmp!=16000) {
       fprintf(stderr,"Error setting SNDCTL_DSP_SPEED\n");
       return;
}

while (!feof(stdin))
       {
              sample=  fgetc(stdin) + fgetc(stdin)*256;
              if ((-1== write(fd,&sample,2)) && errno==EINTR) {
                fprintf(stderr,"Error writing to /dev/dsp\n");
                return;
              }
       }

ioctl(fd, SNDCTL_DSP_SYNC);
ioctl(fd, SNDCTL_DSP_RESET);
  close(fd);
}
