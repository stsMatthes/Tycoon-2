/*********************************************************************/
/*                                                                   */
/* Description:                                                      */
/*   Fixes CR, LF, tab, and other problems caused by transferring    */
/*   files from one platform to another.                             */
/*                                                                   */
/* Usage:                                                            */
/*   Enter "FIXCRLF ?" for usage information                         */
/*                                                                   */
/* Notes:                                                            */
/*   On OS/2 compile as follows:                                     */
/*     icc /q /ti+ fixcrlf.cpp setargv.obj /B"/NOE"                  */
/*                                                                   */
/*********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <sys/stat.h>
#include <time.h>

//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\

#ifndef __UNIX__

  // system specific functions
  #include <io.h>
  #include <sys/utime.h>

  #define chmod(path,mode) _chmod(path,(mode)&0200 ? S_IREAD|S_IWRITE : S_IREAD);

  // default options
  int addcr  = 1;    // add cr's
  int addtab = 0;    // expand tabs
  int force  = 0;    // don't process readonly files
  int ctrlz  = 1;    // add eof
  int update = 0;    // leave date alone
  int quiet  = 0;    // output warning messages

#else

  // system specific functions
  #include <unistd.h>
  #include <errno.h>
  #include <utime.h>
  #define _utime utime
  #define _stat  stat

  // default options
  int addcr  = 0;    // remove cr's
  int addtab = 1;    // replace spaces with tabs
  int force  = 0;    // only process writable files
  int ctrlz  = 0;    // remove eof chars
  int update = 0;    // leave date alone
  int quiet  = 0;    // output warning messages

#endif
//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\

main(int argc, char** argv) {
  int rc = 0;

  if (argc == 1 || *argv[1] == '?') {
    fprintf(stderr, "Syntax:\n");
    fprintf(stderr, "  %s [-r|+r] [-t|+t] [-z|+z] [-d|+d] [-f|+f] [-q] file...\n", *argv);
    fprintf(stderr, "\n");
    fprintf(stderr, "Description:\n");
    fprintf(stderr, "  +r - Adds carriage control characters before every line feed\n");
    fprintf(stderr, "  -r - Removes all carriage control characters\n");
    fprintf(stderr, "  +t - Converts spaces to tabs where it will conserve space\n");
    fprintf(stderr, "  -t - Converts tabs to spaces\n");
    fprintf(stderr, "  +z - Ensures that there is a trailing EOF (control Z) character.\n");
    fprintf(stderr, "  -z - Removes trailing EOF (control Z) characters.\n");
    fprintf(stderr, "  -d - Preserves file changed date.\n");
    fprintf(stderr, "  +d - Updates file changed date.\n");
    fprintf(stderr, "  -f - no force (i.e., convert only writable files)\n");
    fprintf(stderr, "  +f - Force (i.e., convert readonly files too)\n");
    fprintf(stderr, "  -q - Quiet (e.g., no warnings on file not found)\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "Defaults:\n");
    fprintf(stderr, "  OS/2: +r -t +z -d -f\n");
    fprintf(stderr, "  AIX:  -r +t -z -d -f\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "Notes:\n");
    fprintf(stderr, "  - do not run on binary files!\n");
    fprintf(stderr, "  - carriage returns which do not preceed line feeds are discarded.\n");
    fprintf(stderr, "  - trailing blanks are removed from lines.\n");
    return (4);
  }

  while (--argc) {

    char *path = *++argv;

    // process options
    if (*path == '-' || *path == '+') {
      for (char *c=path; *++c; ) {
        switch (*c) {
          case 't':  addtab = (*path=='+');  break;
          case 'r':  addcr  = (*path=='+');  break;
          case 'z':  ctrlz  = (*path=='+');  break;
          case 'd':  update = (*path=='+');  break;
          case 'f':  force  = (*path=='+');  break;
          case 'q':  quiet  = 1;             break;

          default:
            fprintf(stderr, "Unsupported option \"%c\" ignored.", *c);
            break;
        }
      }

      continue;
    }

   // extract the directory information
#ifdef _WIN32
    struct _stat fileInfo;
#else
    struct stat fileInfo;
#endif
    if (_stat(path, &fileInfo)) {
      if (!quiet) {
        if (strchr(path,'*')) {
          fprintf(stderr, "No files match pattern \"%s\"\n", path);
        } else {
          fprintf(stderr, "Error extracting directory info for %s\n", path);
        } /* endif */
      } /* endif */

      rc = 4;
      continue;
    }

    // Save creation and last write timestamps
#ifdef _WIN32
    struct _utimbuf newtimes;
#else
    struct utimbuf newtimes;
#endif
    newtimes.actime = fileInfo.st_atime;
    newtimes.modtime = fileInfo.st_mtime;

    // force read/write
    if (~fileInfo.st_mode & 0200) {
      if (!force) {
        fprintf(stderr, "Error: file \"%s\" is readonly and force (-f) option is not specified.\n", path);
        rc = 4;
        continue;
      };

      chmod(path, fileInfo.st_mode | 0600);
    }

    // open file
    FILE *stream = fopen(path,"rb");
    if (stream == NULL) {
      fprintf(stderr, "Error opening %s\n", path);
      return 8;
    }

    // read data as binary
    char *indata = new char[fileInfo.st_size];
    int count = fread(indata, sizeof(char), fileInfo.st_size, stream);
    if (count == 0) {
      fprintf(stderr, "Error reading %s\n", path);
      return 8;
    }

    // count the number of cr, lf,  and tab characters
    char *c;
    int k;
    int cr = 0;
    int lf = 0;
    int tab = 0;

    for (c=indata,k=count; k--; c++) {
      if (*c == '\r') cr++;
      if (*c == '\n') lf++;
      if (*c == '\t') tab++;
    }

    // check for trailing eof
    int eof = (count && indata[count-1] == '\x1A');

    // print out stats
    printf("%s: size=%d cr=%d lf=%d tab=%d eof=%d\n",
           path, fileInfo.st_size, cr, lf, tab, eof);

    // copy the data
    char *outdata = new char[count - cr + lf + tab*7 + ctrlz];
    char *o = outdata;
    char *line = o;
    int  col = 0;

    for (c=indata,k=count; k--; c++) {
      switch (*c) {
        case ' ':                       // space:
          col++;                        //   update desired column
          break;

        case '\t':                      // tab:
          col = (col|7)+1;              //   update desired column
          break;

        case '\r':                      // cr:
          break;                        //   ignore

        case '\n':                      // lf:
          if (addcr) *o++='\r';         //   optionally add cr
          *o++='\n';                    //   add lf
          line=o;                       //   reset line pointer
          col=0;                        //   reset desired column
          break;

        default:                        // other chars:
          if (addtab && o+1<line+col) { //   if tabs and two or more spaces are desired then
            int diff=o-line;            //     determine logical column
            while ((diff|7)<col) {      //     while tab wouldn't pass desired column
              *o++='\t';                //       add tab char
              line-=7-(diff&7);         //       backup line ptr to compensate
              diff=o-line;              //       update logical column
            };                          //     end while
          };                            //   end if

          while (o<line+col) *o++=' ';  //   space out to desired column
          *o++=*c;                      //   add character
          col++;                        //   increment desired column
      }
    }

    // add or remove an eof character as required
    if (ctrlz) {
      if (!eof) *o++='\x1A';
    } else {
      if (eof) o--;
    }

    // reopen for output
    fclose(stream);
    stream = fopen(path, "wb");
    if (stream == NULL) {;
      fprintf (stderr, "Unable to open for output %s.\n", path, rc);
      return(8);
    }

    // output the data
    if (!fwrite(outdata, sizeof(char), (o-outdata), stream)) {
      fprintf (stderr, "Error writing to file %s.\n", path, rc);
      return(8);
    }

    // clean up
    delete[] outdata;
    fclose(stream);

    // optionally reset last changed dates
    if (!update) {
      if (_utime(path, &newtimes)) {
        fprintf (stderr, "Unable to set date for %s, rc=%ld\n", path, rc);
        return(8);
      }
    }

    // reset file permissions
    if (~fileInfo.st_mode & 0200) {
      chmod(path, fileInfo.st_mode);
    } /* endif */
  } /* end while */

  return (rc);
}

