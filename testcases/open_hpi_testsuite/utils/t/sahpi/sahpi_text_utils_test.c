/*      -*- linux-c -*-
 *
 * (C) Copyright IBM Corp. 2004, 2005
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  This
 * file and program are licensed under a BSD style license.  See
 * the Copying file included with the OpenHPI distribution for
 * full licensing terms.
 *
 * Author(s):
 *      Sean Dague <sldague@us.ibm.com>
 */

#include <stdio.h>
#include <stdlib.h>

#include <SaHpi.h>
#include <oh_utils.h>
#include <oh_error.h>

#define fill_text_buffer(b, chartype, string)                           \
        do {                                                            \
                b.DataType = chartype;                                  \
                b.Language = SAHPI_LANG_ENGLISH;                        \
                b.DataLength = sizeof(string) - 1;                      \
                strncpy((char *)b.Data,string,sizeof(string) - 1);              \
        } while(0)

#define TEXTT SAHPI_TL_TYPE_TEXT
#define UNICODET SAHPI_TL_TYPE_UNICODE 
#define BCDPLUST SAHPI_TL_TYPE_BCDPLUS
#define ASCII6T SAHPI_TL_TYPE_ASCII6
#define BINARYT SAHPI_TL_TYPE_BINARY 

#define failed(num)                         \
        do {                                \
                failcount++;                \
                err("Failed Test %d", num); \
        } while(0)


int main(int argc, char **argv) 
{
        /* const char *expected_str; */
        // SaErrorT   expected_err, err;
        SaHpiTextBufferT buffer, buffer2;
        oh_big_textbuffer oh_buff, oh_buff2;
        int failcount = 0;
       
        setenv("OPENHPI_ERROR","YES",1);
        // Test1 - valid text string
        
        fill_text_buffer(buffer, TEXTT, "a brown fox! ");
        if(!oh_valid_textbuffer(&buffer)) {
                failed(1);
        }

        // Test2 - valid ascii6
        fill_text_buffer(buffer, ASCII6T, "XYZ 0123!");
        if(!oh_valid_textbuffer(&buffer)) {
                failed(2);
        }
        
        // Test3 - valid utf16 - this is Om 42, incase anyone cares
        fill_text_buffer(buffer, UNICODET, "à¼à¼¤à¼¢");
        if(!oh_valid_textbuffer(&buffer)) {
                failed(3);
        }
        
        // Test 4 - valid BCDPLUS
        fill_text_buffer(buffer, BCDPLUST, "1234-98.56:11");
        if(!oh_valid_textbuffer(&buffer)) {
                failed(4);
        }

        // Test 5 - invalid utf16 - 
        fill_text_buffer(buffer, UNICODET, "à¼à¼¤à¼");
        if(oh_valid_textbuffer(&buffer)) {
                failed(5);
        }
        
        // Test 6 - invalid BCDPLUS
        fill_text_buffer(buffer, BCDPLUST, "a quick brown fox");
        if(oh_valid_textbuffer(&buffer)) {
                failed(6);
        }
        
        // Test7 - invalid ascii6
        fill_text_buffer(buffer, ASCII6T, "abc");
        if(oh_valid_textbuffer(&buffer)) {
                failed(7);
        }
        
        // Test 8 - invalid ascii6 (has a null)
        memset(buffer.Data, 0, sizeof(*buffer.Data));
        fill_text_buffer(buffer, ASCII6T, "abc");
        buffer.DataLength++;
        if(oh_valid_textbuffer(&buffer)) {
                failed(8);
        }

        // Test 9 - invalid bcdplus (has a null)
        memset(buffer.Data, 0, sizeof(*buffer.Data));
        fill_text_buffer(buffer, BCDPLUST, "1234");
        buffer.DataLength++;
        if(oh_valid_textbuffer(&buffer)) {
                failed(9);
        }

        // Test 10 - invalid type 
        fill_text_buffer(buffer, BINARYT + 70, "1234");
        if(oh_valid_textbuffer(&buffer)) {
                failed(10);
        }
        // Test 11 - invalid lang
        fill_text_buffer(buffer, TEXTT, "1234");
        buffer.Language = SAHPI_LANG_ENGLISH + 250;
        if(oh_valid_textbuffer(&buffer)) {
                failed(11);
        }

        /*
         * TODO: add append tests for regular text buff
         */

        
        memset(&buffer2, 0, sizeof(buffer2));
        buffer2.DataType = SAHPI_TL_TYPE_TEXT;
        buffer2.Language = SAHPI_LANG_ENGLISH;
        oh_init_textbuffer(&buffer);
        if(memcmp(&buffer, &buffer2, sizeof(buffer)) != 0) {
                printf("Value %d\n",memcmp(&buffer, &buffer2, sizeof(buffer)));
                oh_print_text(&buffer2);
                oh_print_text(&buffer);
                failed(12);
                goto end;
        }

        /* we know init works now */
        
        oh_append_textbuffer(&buffer, "Some Text");

        if(strncmp((char *)&buffer.Data, "Some Text", buffer.DataLength) != 0) {
                failed(13);
        }
        if(buffer.DataLength != 9) {
                failed(14);
        }
        // 
        oh_append_textbuffer(&buffer, ", Some More Text");
        if(strncmp((char *)&buffer.Data, "Some Text, Some More Text", buffer.DataLength) != 0) {
                failed(15);
        }
        if(buffer.DataLength != 25) {
                failed(16);
        }

        /***********************
         *
         *  oh_big_textbuffer tests
         *
         **********************/
        memset(&oh_buff2, 0, sizeof(oh_buff2));
        oh_buff2.DataType = SAHPI_TL_TYPE_TEXT;
        oh_buff2.Language = SAHPI_LANG_ENGLISH;
        oh_init_bigtext(&oh_buff);
        if(memcmp(&oh_buff, &oh_buff2, sizeof(oh_buff)) != 0) {
                printf("Value %d\n",memcmp(&oh_buff, &oh_buff2, sizeof(oh_buff)));
                oh_print_bigtext(&oh_buff2);
                oh_print_bigtext(&oh_buff);
                failed(32);
                goto end;
        }
        /* we know init works now */
        
        oh_append_bigtext(&oh_buff, "Some Text");

        if(strncmp((char *)&oh_buff.Data, "Some Text", oh_buff.DataLength) != 0) {
                failed(33);
        }
        if(oh_buff.DataLength != 9) {
                failed(34);
        }
        // 
        oh_append_bigtext(&oh_buff, ", Some More Text");
        if(strncmp((char *)&oh_buff.Data, "Some Text, Some More Text", oh_buff.DataLength) != 0) {
                failed(35);
        }
        if(oh_buff.DataLength != 25) {
                failed(36);
        }
        
end:
        if(failcount) {
                return -1;
        }
	return(0);
}
