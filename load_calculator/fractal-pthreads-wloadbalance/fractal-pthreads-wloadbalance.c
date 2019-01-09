/* Processes the mandelbrot set and outputs index values. */
/* 0.7.0 beta development version
 * + Begining of branch, command-line parsing re-write
 * + Added bug-fix for raster output, hasn't fully worked tho :-)
 *
 * This new development branch aims to achieve the following:
 * + Proper command-line parsing
 * + Better bitmap handling, including fixing FP fckps
 * + Option for background operation
 * 
 */

/* Copyright (C) 1999-2002 Phil Stewart
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *             
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *              
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *                   
 * Contact: Phil Stewart, phil.stewart@lichp.co.uk */

/* For more details on the maths behind this, and how the program
 * computes it, see the README file. */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <pthread.h>

//#define DEBUG
struct arg_struct {
    int inf;
    int sup;
    int (*function)(double, double);
    int *pixels;
    int xsize;
};

/* all program parameters are now global variables */
static double lbr, ubr, lbj, ubj, increm;  /* bounds and increment vars */
static int mult;                           /* iterations multiplier */
static int algo;                           /* algorithm selection */
static int bgset;                          /* background operation flag */
static char filename[100];                 /* output filename */

static char vernum[5] = "0.7.0";           /* version number (Tee Hee!) */

/* prototypes */
void usage(void);                          /* prints usage info to stdout */
int getargs(int, char**);                  /* gets command-line args */

/* function to print version and usage information to stdout */
void usage(void)
{
  printf("Mandel %s: Phil Stewart, 2002\n\n", vernum);
  printf("USAGE: mandel [VALUES] [OPTIONS] FILE\n"
         "Generate Mandelbrot Set with specified VALUES and outputs to FILE\n\n"
         "Values: These MUST ALL be specified whenever you want mandel to do something;\n\n"
         "  -l, --left VAL               value to use for the left edge\n"
         "  -r, --right VAL              value to use for the right edge\n"
         "  -b, --bottom VAL             value to use for the bottom edge\n"
         "  -t, --top VAL                value to use for the top edge\n"
         "  -i, --increment VAL          value to increment by between steps\n\n"
         "Options:\n\n"
         "  -m, --multiplier VAL         calculate VAL times more iterations than usual\n"
         "                               (defaults to 1 if unspecified)\n"
         "  -a, --algorithm VAL          use a different algorithm\n"
         "                               (defaults to 1 if unspecified)\n"
         "  -k, --background             run in background\n"
         "      --help                   display this help and exit\n"
         "      --version                output version information and exit\n\n"
         "VALUES must be specified before OPTIONS.\n\n"
         "Note that you must specify the VALUES carefully; they must be sensible (i.e the\n"
         "value you specify for --left must be smaller than for --right, --bottom smaller\n"
         "than top), and you should calculate them carefully, so that --increment divides\n"
         "the difference between --top and --bottom AND the difference between --left and\n"
         "--right. Bad values will not necessarily picked up, and could lead to spurious\n"
         "output, or worse.\n\n"
         "If you supply a multiplier, it MUST be a positive integer.\n\n"
         "There are currently two choices of algorithm. Further algorithms are planned,\n"
         "and one day I intend to implemwent a plugin architecture for algorithms, so you\n"
         "can write your own and drop it straight in :-)\n\n"
         "  --algorithm 1                default, bog standard mandelbrot\n"
         "  --algorithm 2                logarithmic colour allocation\n\n"
         "For more information on the alogrithms, see the README\n");
}

/* function to obtain and parse the command-line arguments */
int getargs(int argc, char *argv[])
{
  /* declare required vars */
  int i, i_1, i_2, i_3, i_4;               /* indexing vars */
  int foundtag;                            /* tag number found in argv */
  int multflag, algoflag;                  /* flags for parsing -m, -a args */
  int usedtags[16] =
  {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
  };                                       /* flagged when each tag found */
  char *validtags[] =
  {
    "-l", "--left", "-r", "--right", "-b", "--bottom", "-t", "--top",
    "-i", "--increment", "-m", "--multiple", "-a", "--algorithm",
    "-k", "--background"
  };                                       /* array of tags passable from cl */
  
  /* first a sanity check - if there is only one argument then just spit back
   * usage and exit happily */
   if (argc == 1)
  {
    usage();
    exit(0);
  }
   
  /* if any argument is --help, return help. If any argument is --version
   * return version. Either way, exit happily. Use a seperate loop for each,
   * as would prefer to show help rather than version. */
  for (i = 0; i < argc; i++)
  {
    if(strcmp(argv[i], "--help") == 0)
    {
      usage();
      exit(0);
    };
  };
  
  for (i = 0; i < argc; i++)
  {
    if(strcmp(argv[i], "--version") == 0)
    {
      printf("Mandel %s: Copyright (C) Phil Stewart, 2002\n"
             "Mandel comes with ABSOLUTELY NO WARRANTY;\n"
             "This is free software, and you are welcome to redistribute it\n"
             "under certain conditions;\n"
             "Please see the LICENSE file which accompanied this program\n"
             "for more information.\n", vernum);
      exit(0);
    };
  };

  /* next set default values for mult and algo */
  mult = 1;
  algo = 1;
  multflag = 0;

  /* parse arg by arg. Throw at anything unexpected, such as a required value
   * not following the arg; make sure the last arg is ok for filename */
  for (i = 1; i < argc; i++)               /* argv[0] is always path/mandel */
  {
    /* reset foundtag to a dead value, otherwise tag from located from previous
     * arg will break something */
    foundtag = -1;
/* DEBUG */
/*    printf("dbg loop i: %s %d\n", argv[i], i);*/
/* END DEBUG */
    for (i_1 = 0; i_1 < 16; i_1++)
    {
      /* compare argv to each valid tag */
/* DEBUG */
/*      printf("debug loop i_1: %s %d\n", validtags[i_1], i_1);*/
/* END DEBUG */
      if(strcmp(argv[i], validtags[i_1]) == 0)
      {
/* DEBUG */
/*        printf("dbg check: %d %s %s\n", i, argv[i], validtags[i_1]);*/
/* END DEBUG */      
        /* Throw if a duplicate tag is passed */
        if (usedtags[i_1] == 1)
        {
          printf("Mandel: You can only specify %s once\n", validtags[i_1]);
          exit(1);
        };
        foundtag = i_1;
        break;
      };
    };

    /* if we've not matched a tag and this isn't the last argument, then
     * either my code is broken (doh!) or the user has passed some broken
     * options; either way, throw */
    if (foundtag == -1)
    {
      if (i == (argc - 1))
      {
        strcpy(filename, argv[i]);
      }
      else
      {
        printf("Mandel: Option %s not recognised; check your parameters\n", argv[i]);
        exit(1);
      };
    };

    /* to irradicate duplicate code in the switch section, if the found tag
     * is in long form, we use the preceeding tag, which is always the short
     * form */
    if ((foundtag % 2) == 1)
      foundtag--;

    /* ok, we've now isolated a tag, now we need to process it */
    switch (foundtag)
    {
      case 0:                              /* -l */
              usedtags[0] = 1;             /* set -l as being used */
              usedtags[1] = 1;             /* set --left as being used */
              i++;                         /* next tag should contain a val */

              /* check make sure the next argument doesn't contain a tag */
              for (i_1 = 0; i_1 < 16; i_1++)
              {
                if (strcmp(argv[i], validtags[i_1]) == 0)
                {
                  printf("Mandel: %s requires a value; check your parameters\n", validtags[foundtag]);
                  exit(1);
                };
              };
              
              /* now get the value, throw if the value is invalid
               * WARNING: this is a blind conversion, and will just
               * convert whatever is found as best it can. This may lead
               * to unexpected results ... */
              lbr = atof(argv[i]);
              break;
              
      case 2:                              /* -r */
              usedtags[2] = 1;             /* set -r as being used */
              usedtags[3] = 1;             /* set --right as being used */
              i++;                         /* next tag should contain a val */

              /* check make sure the next argument doesn't contain a tag */
              for (i_1 = 0; i_1 < 16; i_1++)
              {
                if (strcmp(argv[i], validtags[i_1]) == 0)
                {
                  printf("Mandel: %s requires a value; check your parameters\n", validtags[foundtag]);
                  exit(1);
                };
              };
              
              /* now get the value, throw if the value is invalid
               * WARNING: this is a blind conversion, and will just
               * convert whatever is found as best it can. This may lead
               * to unexpected results ... */
              ubr = atof(argv[i]);
              break;

      case 4:                              /* -b */
              usedtags[4] = 1;             /* set -b as being used */
              usedtags[5] = 1;             /* set --bottom as being used */
              i++;                         /* next tag should contain a val */

              /* check make sure the next argument doesn't contain a tag */
              for (i_1 = 0; i_1 < 16; i_1++)
              {
                if (strcmp(argv[i], validtags[i_1]) == 0)
                {
                  printf("Mandel: %s requires a value; check your parameters\n", validtags[foundtag]);
                  exit(1);
                };
              };
              
              /* now get the value, throw if the value is invalid
               * WARNING: this is a blind conversion, and will just
               * convert whatever is found as best it can. This may lead
               * to unexpected results ... */
              lbj = atof(argv[i]);
              break;
      
      case 6:                              /* -t */
              usedtags[6] = 1;             /* set -t as being used */
              usedtags[7] = 1;             /* set --top as being used */
              i++;                         /* next tag should contain a val */

              /* check make sure the next argument doesn't contain a tag */
              for (i_1 = 0; i_1 < 16; i_1++)
              {
                if (strcmp(argv[i], validtags[i_1]) == 0)
                {
                  printf("Mandel: %s requires a value; check your parameters\n", validtags[foundtag]);
                  exit(1);
                };
              };
              
              /* now get the value, throw if the value is invalid
               * WARNING: this is a blind conversion, and will just
               * convert whatever is found as best it can. This may lead
               * to unexpected results ... */
              ubj = atof(argv[i]);
              break;
      
      case 8:                              /* -i */
              usedtags[8] = 1;             /* set -i as being used */
              usedtags[9] = 1;             /* set --increment as being used */
              i++;                         /* next tag should contain a val */

              /* check make sure the next argument doesn't contain a tag */
              for (i_1 = 0; i_1 < 16; i_1++)
              {
                if (strcmp(argv[i], validtags[i_1]) == 0)
                {
                  printf("Mandel: %s requires a value; check your parameters\n", validtags[foundtag]);
                  exit(1);
                };
              };
              
              /* now get the value, throw if the value is invalid
               * WARNING: this is a blind conversion, and will just
               * convert whatever is found as best it can. This may lead
               * to unexpected results ... */
              increm = atof(argv[i]);
              break;
      
      case 10:                             /* -m */
              usedtags[10] = 1;            /* set -m as being used */
              usedtags[11] = 1;            /* set --multiple as being used */
              i++;                         /* next tag should contain a val */

              /* check make sure the next argument doesn't contain a tag */
              for (i_1 = 0; i_1 < 16; i_1++)
              {
                if (strcmp(argv[i], validtags[i_1]) == 0)
                {
                  printf("Mandel: No value given for %s; assuming 1\n", validtags[foundtag]);
                  i--;                     /* must step back again so next
                                            * arg is handled correctly */ 
                  multflag = 1;            /* flag multflag so we can break */
                };
              };
              if (multflag == 1)           /* break if instructed above */
                break;
              
              /* now get the value, throw if the value is invalid
               * WARNING: this is a blind conversion, and will just
               * convert whatever is found as best it can. This may lead
               * to unexpected results ... */
              mult = abs(atol(argv[i]));
              break;
      
      case 12:                             /* -a */
              usedtags[12] = 1;            /* set -a as being used */
              usedtags[13] = 1;            /* set --algorithm as being used */
              i++;                         /* next tag should contain a val */

              /* check make sure the next argument doesn't contain a tag */
              for (i_1 = 0; i_1 < 16; i_1++)
              {
                if (strcmp(argv[i], validtags[i_1]) == 0)
                {
                  printf("Mandel: No value given for %s; assuming 1\n", validtags[foundtag]);
                  i--;                     /* must step back again so next
                                            * arg is handled correctly */
                  algoflag = 1;            /* break if instructed above */
                };
              };
              if (algoflag == 1)           /* break if instructed above */
                break;
              
              /* now get the value, throw if the value is invalid
               * WARNING: this is a blind conversion, and will just
               * convert whatever is found as best it can. This may lead
               * to unexpected results ... */
              algo = abs(atol(argv[i]));
              break;
      
      case 14:                             /* -k */
              usedtags[14] = 1;            /* set -k as being used */
              usedtags[15] = 1;            /* set --background as being used */
              
              /* background option is not yet supported; inform user */
              printf("Mandel: the background option is not yet supported\n");
              break;
    };
  };
}

static int getPixel(double lr, double lj)
{
  int index;
  double zm = 0, zr, zj, zoldr = 0, zoldj = 0;
  for(index = 1; zm < 10000 && index < 256 * mult - 1; index++)
        {
          zr = zoldr * zoldr - zoldj * zoldj + lr;
          zj = 2 * zoldr * zoldj + lj;
          zm = zr * zr + zj * zj;
          zoldr = zr;
          zoldj = zj;
        };
  return index;
}

int GetPixelJulia(double Zx, double Zy)
 {
  double Zx2, Zy2; /* Zx2=Zx*Zx;  Zy2=Zy*Zy  */
  Zx2=Zx*Zx;
  Zy2=Zy*Zy;
  int index;
  for(index = 1; (Zx2 + Zy2) < 10000 && index < 256 * mult - 1; index++)
  {
    Zy = 2*Zx*Zy + 0.34;
    Zx = Zx2-Zy2 + 0.4;
    Zx2 = Zx*Zx;
    Zy2 = Zy*Zy;
  };
  return index;
 }

static int GetPixelMulti(double Zx, double Zy)
{
 double Zx2 = Zx*Zx;
 double Zy2 = Zy*Zy;
 double Zx3 = Zx2 * Zx;
 double Zy3 = Zy2 * Zy;
  int index;
  for(index = 1; (Zx2 + Zy2) < 10000 && index < 256 * mult - 1; index++)
  {
    Zx = Zx3 - 3*Zx*Zy2 + 0.62;
    Zy = 3*Zx2*Zy - Zy3 - 0.5;

    Zx2 = Zx*Zx;
    Zy2 = Zy*Zy;
    Zx3 = Zx2 * Zx;
    Zy3 = Zy2 * Zy;
  };
  return index;
}

void *p_function(void *x) {
  
  #ifdef DEBUG
  struct timespec start, finish;
  double elapsed;
  clock_gettime(CLOCK_REALTIME, &start);
  #endif

    struct arg_struct *args = (struct arg_struct *)x;
    int i, j, index;
    double lj, lr;
    for (i = args->inf; i <= args->sup; i++) {
      lj = lbj + i*increm;
        for(j = 0; j < args->xsize; j++)
        {
          lr = lbr + j*increm;
          index = args->function(lr, lj);
          
          index = index % 256;
          args->pixels[i * args->xsize + j] = index;
        }
    }
  #ifdef DEBUG
  clock_gettime(CLOCK_REALTIME, &finish);

  elapsed = (finish.tv_sec - start.tv_sec);
  elapsed += (finish.tv_nsec - start.tv_nsec) / 1000000000.0;
  printf("%d %d %lf\n", args->inf, args->sup, elapsed);
  #endif
}

/* main function, our biggie */
int main(int argc, char *argv[])
{
  /* initialise variables */
  FILE *fp;                                /* file pointer variable */
  int index, i_1;                          /* indexing variable for loops */
  char *endptr;                            /* string end pointer */
  int userin;                              /* user input character */
  int xsize, ysize, picsize, filesize;     /* bitmap variables */
  int waste;                               /* bitmap var for wasted pixels */
  double zr, zj, zm = 0, zoldr, zoldj, lr, lj; /* complex number variables */
  double logdiv, logindex;                 /* logarithm variables */

  /* DEBUG OVERRIDE - All code in this section MUST BE CRIPPLED for normal
   * operation!! */
   
/*   getargs(argc, argv); */
/*   return 0; */
      
  /* END DEBUG OVERRIDE */           
   
  /* get command line argument values */
  /* to be depracted in favour of new parsing function */
/*  lbr = strtod(argv[1], &endptr);  */        /* lower bound real (X) */
/*  ubr = strtod(argv[2], &endptr);  */        /* upper bound real (X) */
/*  lbj = strtod(argv[3], &endptr);  */        /* lower bound imaginary (Y) */
/*  ubj = strtod(argv[4], &endptr);  */        /* upper bound imaginary (Y) */
/*  increm = strtod(argv[5], &endptr); */      /* increment (1 / resolution) */
/*  mult = strtod(argv[6], &endptr);   */      /* iteration multiplier */

  /* new parsing function */
  getargs(argc, argv);
  
  /* check command line values */
  if (lbr >= ubr)                          /* check real range is positive */
  {
    printf("Mandel: Error - real range is zero or negative; check your parameters\n");
    return 0;
  };
  if (lbj >= ubj)                          /* ditto for imaginary */
  {
    printf("Mandel: Error - imaginary range is zero or negative; check you parameters\n");
    return 0;
  };
  if (increm >= 1)                         /* check increment is sensible */
  {
    printf("Mandel: Error - increment is too big; check your parameters\n");
    return 0;
  };
  /* set intial values */
  lr = lbr;                                /* lambda starts at lower bounds */
  lj = lbj;                                /* ditto */
  zoldr = 0;                               /* Z starts at 0 */
  zoldj = 0;                               /* ditto */
  
  
  /* calculate picture variables */
  xsize = (ubr - lbr) / increm + 1;        /* calculate length of bitmap */
  ysize = (ubj - lbj) / increm + 1;        /* calculate height of bitmap */
  picsize = xsize * ysize;                 /* calculate number of pixels */
  waste = (4 - (xsize % 4) % 4) * ysize;   /* calculate overall encoding waste */
  filesize = picsize + waste + 1078;       /* image + waste + header = file */
  
  int i, j;
  int *all_dem_pixels;
  all_dem_pixels = malloc(picsize * sizeof(int));
   
  /* open file */
  fp = fopen(filename, "w");
   
  /* write bitmap header */
  /* Bitmap is Windows RGB 8 bit, with standard 54 byte header and 1024 byte
   * colour table.
   * For more details, see the readme file. */
  /* 54 byte header */
  putc(66, fp);
  putc(77, fp);
  putc((filesize % 256), fp);
  putc((((filesize - (filesize % 256)) / 256) % 256), fp);
  putc((((filesize - (filesize % 65536)) / 65536) % 256), fp);
  putc((((filesize - (filesize % 16777216)) / 16777216) % 256), fp);
  putc(0, fp);
  putc(0, fp);
  putc(0, fp);
  putc(0, fp);

  putc(54, fp);
  putc(4, fp);
  putc(0, fp);
  putc(0, fp);
  putc(40, fp);
  putc(0, fp);
  putc(0, fp);
  putc(0, fp);
  putc((xsize % 256), fp);
  putc((((xsize - (xsize % 256)) / 256) % 256), fp);
  
  putc((((xsize - (xsize % 65536)) / 65536) % 256), fp);
  putc(0, fp);
  putc((ysize % 256), fp);
  putc((((ysize - (ysize % 256)) / 256) % 256), fp);
  putc((((ysize - (ysize % 65536)) / 65536) % 256), fp);
  putc(0, fp);
  putc(1, fp); /* this specifies there is 1 layer in the image */
  putc(0, fp);
  putc(8, fp); /* this specifies 8 bit file encoding */
  putc(0, fp);
  
  /* picsize not required in uncompressed bitmaps */
  putc(0, fp);
  putc(0, fp);
  putc(0, fp);
  putc(0, fp);
  putc(0, fp); /* putc((picsize % 256), fp); */
  putc(0, fp); /* putc((((picsize - (picsize % 256)) / 256) % 256), fp); */
  putc(0, fp); /* putc (((picsize - (picsize % 65536)) / 65536) % 256), fp); */
  putc(0, fp); /* putc((((picsize - (picsize % 16777216)) / 16777216) % 256), fp); */
  putc(0, fp);
  putc(0, fp);

  putc(0, fp);
  putc(0, fp);
  putc(0, fp);
  putc(0, fp);
  putc(0, fp);
  putc(0, fp);
  putc(0, fp);
  putc(1, fp);
  putc(0, fp);
  putc(0, fp);
  
  putc(0, fp);
  putc(1, fp);
  putc(0, fp);
  putc(0, fp);
  
  /* 1024 byte colour table */
  for(index = 0; index < 256; index++)
  {
    putc(index, fp);
    putc(index, fp);
    putc(index, fp);
    putc(0, fp);
  };
   
   
  /* compute */
  /* This is where all the number crunching takes place.
   * The iteration is processed for each value of lambda
   * and the result is output to the bitmap. The code is
   * duplicated for efficiency - one block processes for
   * normal values allocation, the other logarithmic va. */
   
  /* print start line to stdout */

  // I use env variable "TH_NUM" to get num of threads
  // easier for scripting
  char *th_num_str = getenv("TH_NUM");
  int th_num = atoi(th_num_str);
  char input_file[20];
  sprintf(input_file, "../%d_th_load.txt", th_num);

  FILE *th_load = fopen(input_file, "r");
  if (!th_load) {
    fprintf(stderr, "Could not open file: %s\n", input_file);
    exit(EXIT_FAILURE);
  }

  struct arg_struct args[th_num];
  pthread_t threadsugoi[th_num];
  int inf, sup;

  for (i = 0; i < th_num; i++) {
      fscanf(th_load, "%d%d", &inf, &sup);
      args[i].inf = inf;
      args[i].sup = sup;
      switch (algo){
        case 1:
          args[i].function = getPixel;
          break;
        case 2:
          args[i].function = GetPixelJulia;
          break;
        case 3:
          args[i].function = GetPixelMulti;
      }
      args[i].pixels = all_dem_pixels;
      args[i].xsize = xsize;
      if(pthread_create(&threadsugoi[i], NULL, p_function, &args[i])) {
          fprintf(stderr, "Error creating thread\n");
          return 1;
      }
  }

  for (i = 0; i < th_num; i++) {
    if (pthread_join(threadsugoi[i], NULL)) {
        fprintf(stderr, "Error joining thread\n");
        return 2;
    }
  }

  for (i = 0; i < ysize; i++) {
    for (j = 0; j < xsize; j++) {
        putc(all_dem_pixels[i * xsize + j], fp);
    }
    for (i_1 = 0; i_1 < ((4 - (xsize % 4)) % 4); i_1++)
      {
        putc(0, fp);
      };
  }
  /* close file */
  fclose(fp);
  fclose(th_load);
  free(all_dem_pixels);

  /* end */
  return 0;
}
