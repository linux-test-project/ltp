/*
 *
 *   Copyright (c) International Business Machines  Corp., 2001
 *
 *   This program is free software;  you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY;  without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 *   the GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program;  if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

/*
 * libsctp_test.c 
 *
 * HISTORY
 *	04/2002 Created by Mingqin Liu	
 *
 * RESTRICTIONS:
 *
 */

#include <stdio.h>
#include <netdb.h>

#include "libsctp_test.h"


extern int errno;


/*
 * Read the interfaces from file /proc/net/dev and /proc/net/if_inet6.
 * Retrieve the IPv4 or IPv6 addresses for the interfaces. 
 *
 */
void get_ip_addresses(local_addr_t *local_addrs, int * count)
{

        int fd; 
        FILE *in_file;
        int *ia; 
        struct ifreq ifr;

        struct in_addr ina; 
        char buf[16];
        char addr[16]; 
        char *pos;

        char address[64];

        int i = 0;

        if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) { 
		printf("Can't talk to kernel! (%d)\n", errno);
		return;
        } 

        in_file = fopen("/proc/net/dev", "r");
        if (!in_file) {
		printf("Unable to open file /proc/net/dev.\n");
		return;
        }

        while ( fscanf(in_file, "%s\n", buf) == 1) {

                *addr = 0; /* remove old address  */
                if (!(pos=(char*)strchr(buf, ':'))) {
                        continue;
                } else {
                        *pos = '\0';    
                } 
                strcpy(local_addrs[i].if_name, buf);     
		//printf("if_name: %s\n", buf);
                strcpy(ifr.ifr_name, buf); 
	
	
                if (ioctl(fd, SIOCGIFFLAGS, &ifr) < 0) { 
			printf("Can't get status for %s. (%d)\n", 
                                buf, errno);
                        close(fd); 
			return;
                } 

		/* if the interface is not up, skip it */
                if ((ifr.ifr_flags & IFF_UP) == 0) { 
                        continue; 
                } 

                if (ioctl(fd, SIOCGIFADDR, &ifr) != 0) { 
			printf("Can't get IP address for %s. (%d)\n", 
                                buf, errno);
                        close(fd); 
			return;
                } 

                ia = (int *)&ifr.ifr_addr; /* seems to be off by 2 bytes  */
                ina.s_addr = ia[1]; 
                strcpy(addr, (char*)inet_ntoa(ina)); 

                if (addr != NULL) {
                        local_addrs[i].has_v4 = 1;
                        strcpy(local_addrs[i].v4_addr, addr);
                } else {
                        local_addrs[i].has_v4 = 0;
                        local_addrs[i].v4_addr[0] = '\0';
                }
		//printf("addr: %s\n", local_addrs[i].v4_addr);
                i++;
        }  /* while */

        *count = i;
        fclose(in_file);

        in_file = fopen("/proc/net/if_inet6", "r");
        if (!in_file) {
		printf("Unable to open file /proc/net/if_inet6.\n");
		return;
        }

        while (fscanf(in_file, "%s\n", address) == 1) {

                int j = 0;
                char *addr_ptr;
		char colon_addr[64];
                int has_double_colon = 0;

                addr_ptr = &address[0];

                colon_addr[0] = '\0';
        
                while (*addr_ptr){
                        char tmp_str[6];
        
                        strncpy(&tmp_str[0], addr_ptr, 4);
                        tmp_str[4] = '\0';

                        strcat(tmp_str, ":");
                        strcat(colon_addr, tmp_str);
        
                        addr_ptr += 4;  
                }
                colon_addr[strlen(colon_addr)-1] = '\0';

                *addr = 0; /* remove old address  */

                fscanf(in_file, "%s\n", buf);
                fscanf(in_file, "%s\n", buf);
                fscanf(in_file, "%s\n", buf);
                fscanf(in_file, "%s\n", buf);
                fscanf(in_file, "%s\n", buf);

                for (i = 0; i < *count; i++) {
                        if (!strcmp(buf, local_addrs[i].if_name)){
                                local_addrs[i].has_v6 = 1;
                                strcpy(local_addrs[i].v6_addr, colon_addr);
				break;
                        }
			
                }       

                if (i >= *count) {

                        local_addrs[*count].has_v6 = 1;
                        local_addrs[*count].has_v4 = 0;
                        local_addrs[*count].v4_addr[0] = '\0';
                        strcpy(local_addrs[*count].v6_addr, colon_addr);
                        (*count)++;
                }
        }

        fclose(in_file);
        close(fd); 
} /* get_ip_addresses */

/*
 * Add another address represented as the string 'parm' to the list
 * addrs.  The argument count is the length of addrs on input and is
 * adjusted for output.
 * 
 * NOTE: This function is harvested from sctp_darn.c.
 */

struct sockaddr_storage *
append_addr(const char *parm,
            struct sockaddr_storage *addrs, 
	    int *ret_count, 
	    int port) 
{
        struct sockaddr_storage *addr_list;
        struct sockaddr_storage *xap;
        struct sockaddr_in *b4ap;
        struct sockaddr_in6 *b6ap;
        struct hostent *hst4 = NULL;
        struct hostent *hst6 = NULL;
        int i4 = 0;
        int i6 = 0;
        int j;
        int count = *ret_count;

        /* Get the entries for this host.  */
        hst4 = (struct hostent*) gethostbyname(parm);
        hst6 = (struct hostent*) gethostbyname2(parm, AF_INET6);
        if ((NULL == hst4 || hst4->h_length < 1)
            && (NULL == hst6 || hst6->h_length < 1)) {
                printf("bad hostname: %s\n", parm);
                goto finally;
        }

        /* Figure out the number of addresses.  */
        if (NULL != hst4) {
                for (i4 = 0; NULL != hst4->h_addr_list[i4]; ++i4) {
                        count++;
                }
        }
        if (NULL != hst6) {
                for (i6 = 0; NULL != hst6->h_addr_list[i6]; ++i6) {
                        count++;
                }
        }

        /* Expand memory for the new addresses.  */
        addr_list = (struct sockaddr_storage *)
                realloc(addrs, sizeof(struct sockaddr_storage) * count);

        if (NULL == addr_list) {
		//printf("NO mem\n");
                count = *ret_count;
                goto finally;
        }

       /* Put the new addresses away.  */
        xap = &addr_list[*ret_count];

        if (NULL != hst4) {
                for (j = 0; j < i4; ++j) {
                        b4ap = (struct sockaddr_in *)xap++;
                        bzero(b4ap, sizeof(*b4ap));
                        b4ap->sin_family = AF_INET;
                        b4ap->sin_port = ntohs(port);
                        bcopy(hst4->h_addr_list[j], &b4ap->sin_addr,
                              hst4->h_length);
                } /* for (loop through the new v4 addresses) */
        }
        if (NULL != hst6) {
                for (j = 0; j < i6; ++j) {
                        b6ap = (struct sockaddr_in6 *)xap++;
                        bzero(b6ap, sizeof(*b6ap));
                        b6ap->sin6_family = AF_INET6;
                        b6ap->sin6_port = ntohs(port);
                        bcopy(hst6->h_addr_list[j], &b6ap->sin6_addr,
                              hst6->h_length);
                } /* for (loop through the new v6 addresses) */
        }

 finally:

        *ret_count = count;

        return(addr_list);
} /* append_addr() */


int set_nonblock(int s) 
{
        int nonblock_on = 1;
#ifdef O_NONBLOCK
        int flags = fcntl(s, F_GETFL, 0);
        if (-1 == flags) {
                return -1;
        }
        
        return fcntl(s, F_SETFL, flags | O_NONBLOCK);
#else
        
        return ioctl(s, FIONBIO, &nonblock_on);
#endif
}

