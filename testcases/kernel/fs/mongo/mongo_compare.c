/*
 * Copyright 2000 by Hans Reiser, licensing governed by reiserfs/README
 */

#include <stdio.h>
#include <ctype.h>
#include <string.h>

char time_str1[50];
char time_str2[50];
char name_str1[50];
char tmp_str[20][100];

char out1[256];
char out2[256];

FILE *f1;
FILE *f2;
FILE *f3;
FILE *f4;

void write_html_head(FILE * fp);
void write_html_end(FILE * fp);

char head_str[] = "\n \
<!doctype html public \"-//w3c//dtd html 4.0 transitional//en\">\n \
<html>\n \
<head>\n \
   <meta http-equiv=\"Content-Type\" content=\"text/html; charset=iso-8859-1\">\n \
   <meta name=\"GENERATOR\" content=\"Mozilla/4.5 [en] (X11; I; Linux 2.2.7 i586) [Netscape]\">\n \
</head>\n \
<body>\n \
";
/*
<tt></tt>&nbsp;\n \
<table BORDER NOSAVE >\n \
<tr BGCOLOR=\"#CCFFFF\" NOSAVE>\n \
<td NOSAVE> \n \
";
*/

char end_str[] = "\n \
</table> \n \
<tt></tt> \n \
</body> \n \
</html> \n \
";

main(int argc, char **argv)
{
	float n1, n2, ratio;
	char *p, *p1, *p2;
	char line0[100];
	char line1[100];
	char line2[100];
	char line3[100];
	char out_line[100];
	char html_line[500];
	int i, k;

	if (argc < 3) {
		printf("\nUsage: mongo_compare file1 file2 res_file\n\n");
		printf
		    ("\t<file1> should contain reiserfs or ext2 results of mogo benchmark\n");
		printf
		    ("\t<file2> should contain reiserfs or ext2 results of mogo benchmark\n");
		printf("\tMongo results   will be compared\n");
		printf
		    ("\t<res_file.txt>  will be contain results in the text form\n");
		printf
		    ("\t<res_file.html> will be contain results in the html form\n");
		exit(0);
	}

	strcpy(out1, argv[3]);
	strcat(out1, ".txt");

	strcpy(out2, argv[3]);
	strcat(out2, ".html");

	if ((f1 = fopen(argv[1], "r")) == NULL) {
		fprintf(stderr, "%s: can't open %s\n", argv[0], argv[1]);
		return 1;
	}

	if ((f2 = fopen(argv[2], "r")) == NULL) {
		fprintf(stderr, "%s: can't open %s\n", argv[0], argv[2]);
		return 1;
	}

	if ((f3 = fopen(out1, "wr")) == NULL) {
		fprintf(stderr, "%s: can't open %s\n", argv[0], out1);
		return 1;
	}

	if ((f4 = fopen(out2, "wr")) == NULL) {
		fprintf(stderr, "%s: can't open %s\n", argv[0], out2);
		return 1;
	}

	write_html_head(f4);
	i = 0;
	while (fgets(line1, 100, f1)) {
		fgets(line2, 100, f2);

		if (p = strstr(line1, "\n"))
			*(p + 1) = 0;
		if (p = strstr(line2, "\n"))
			*(p + 1) = 0;

		strcpy(line3, line1);
		line3[strlen(line3) - 1] = 0;

		while (strlen(line3) < 40) {
			strcat(line3, " ");
		}

		if (strstr(line3, "MONGO_")) {
			fprintf(f4, "</table>\n<table BORDER NOSAVE >\n");
			fprintf(f4, "<tr BGCOLOR=\"#CCFFFF\" NOSAVE>");
			fprintf(f4, "<td NOSAVE>\n");
			i = 0;
		}
		if (i < 20)
			strcpy(tmp_str[i], line2);

		if (strstr(line3, "FSYS=")) {
			fprintf(f4, "</td><td>\n");
			for (k = 0; k < i; k++) {
				fprintf(f4, "<tt>%s</tt><br>\n", tmp_str[k]);
			}
			fprintf(f4,
				"</td>\n <tr BGCOLOR=\"#CCFFFF\" NOSAVE><td COLSPAN=\"2\"><tt><B> %s %s </B></tt>\n",
				line3, line2);
			i = 20;
		} else if (NULL == strstr(line3, " :")) {

			if (strstr(line3, "(time"))
				fprintf(f4,
					"<br><tt><center>%s</center></tt>\n",
					line3);
			else {
				k = 0;
				p = line3;
				while (*p++ != 0) {
					if (*p != ' ' && *p != '\n')
						k++;
				}
				if (k > 0) {
					fprintf(f4, "<tt>%s</tt><br>\n", line3);
					if (i < 20)
						i++;
				}
			}
		}

		else if (strstr(line3, "Create"))
			fprintf(f4, "</td>\n");

		line2[strlen(line2) - 1] = 0;
		while (strlen(line2) < 40) {
			strcat(line2, " ");
		}

		strcat(line3, line2);

		strcpy(out_line, line3);
		strcat(out_line, "\n");
		name_str1[0] = 0;

		if (p1 = strstr(line1, " :")) {
			strcpy(time_str1, p1 + 2);
			strncpy(name_str1, line1, p1 - line1);

			if (p2 = strstr(line2, " :")) {
				strcpy(time_str2, p2 + 2);

				time_str1[strlen(time_str1) - 1] = 0;
				time_str2[strlen(time_str2) - 1] = 0;

				sscanf(time_str1, "%f", &n1);
				sscanf(time_str2, "%f", &n2);

				ratio = n1 / n2;
				sprintf(out_line, "%s : %6.2f / %6.2f = %.2f\n",
					name_str1, n1, n2, ratio);

				fprintf(f4,
					"<tr><td><tt> %s &nbsp </tt></td> <td><div align=right><tt> %6.2f / %6.2f = %.2f &nbsp </tt></div></td></tr>\n",
					name_str1, n1, n2, ratio);

			}
		}

		fprintf(f3, "%s", out_line);
		line1[0] = 0;
		line2[0] = 0;
		line3[0] = 0;
		out_line[0] = 0;
	}

	write_html_end(f4);

	fclose(f1);
	fclose(f2);

	fclose(f3);
	fclose(f4);

	fflush(f3);
	fflush(f4);
}

/*******************************************/
void write_html_head(FILE * fp)
{
	fprintf(fp, "%s", head_str);
}

/*******************************************/
void write_html_end(FILE * fp)
{
	fprintf(fp, "%s", end_str);
}
