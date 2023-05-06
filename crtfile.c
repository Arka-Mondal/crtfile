/*
  crtfile --- creates file(s) with the permission flags specified
  Copyright (C) 2023  Arka Mondal

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include <stdbool.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <getopt.h>

static const char * program_name = "crtfile";
static mode_t u_opt = 0;
static mode_t g_opt = 0;
static mode_t o_opt = 0;

void errorexit(const char *, ...) __attribute__ ((__noreturn__));
void output_error(const char *, ...);
void usage(int) __attribute__ ((__noreturn__));
void display_version(void);
void setflag(const char * restrict, int);

int main(int argc, char ** argv)
{
  extern mode_t u_opt;
  extern mode_t g_opt;
  extern mode_t o_opt;

  bool if_errocr;    // checks for if error occurred
  int opt_index, option, crt_flag, fd;
  static int verbose;

  static struct option long_options[] = {
    {"help", no_argument, NULL, 'h'},
    {"version", no_argument, NULL, 'S'},
    {"truncate", no_argument, NULL, 't'},
    {"user", required_argument, NULL, 'u'},
    {"group", required_argument, NULL, 'g'},
    {"other", required_argument, NULL, 'o'},
    {"all", required_argument, NULL, 'a'},
    {"verbose", no_argument, &verbose, 1},
    {NULL, 0, NULL, 0}
  };

  if_errocr = false;
  errno = 0;
  opt_index = 0;
  opterr = 0;
  verbose = 0;
  crt_flag = O_WRONLY | O_CREAT | O_EXCL;

  if (argc == 1)
    usage(EXIT_FAILURE);

  while ((option = getopt_long(argc, argv, "vtu:g:o:a:", long_options, &opt_index)) != -1)
  {
    switch (option)
    {
      case 'h':
        usage(EXIT_SUCCESS);
      case 'S':
        display_version();
        exit(EXIT_SUCCESS);
      case 't':
        crt_flag = O_WRONLY | O_TRUNC;
        break;
      case 'v':
        verbose = 1;
        break;
      case 0:
        break;
      case 'u':
      case 'g':
      case 'o':
      case 'a':
        setflag(optarg, option);
        break;
      case '?':
        if (optopt == 'u' || optopt == 'g' || optopt == 'o' || optopt == 'a')
          errorexit("argument required: %s\n", argv[optind - 1]);
        else
          errorexit("unknown option: %s\n", argv[optind - 1]);
      default:
        usage(EXIT_FAILURE);
    }
  }

  if (optind == argc)
    errorexit("missing operand\n");

  if ((u_opt == 0) && (g_opt == 0) && (o_opt == 0))
    setflag("rw", 'a');


  for (; optind < argc; optind++)
  {
    fd = open(argv[optind], crt_flag, u_opt | g_opt | o_opt);
    if (fd == -1)
    {
      output_error("file: '%s': %s\n", argv[optind], strerror(errno));

      if_errocr = true;
      continue;
    }

    if (verbose)
    {
      if (crt_flag & O_TRUNC)
        printf("file: '%s': truncated\n", argv[optind]);
      else
        printf("file: '%s': created\n", argv[optind]);
    }

    close(fd);
  }


  return (!if_errocr) ? EXIT_SUCCESS : EXIT_FAILURE;
}

void setflag(const char * restrict flag, int opt)
{
  extern mode_t u_opt;
  extern mode_t g_opt;
  extern mode_t o_opt;


  if (strlen(flag) > 3)
    errorexit("argument: %s  too long\n", flag);

  switch (opt)
  {
    case 'u':
      if (strchr(flag, 'r'))
        u_opt |= S_IRUSR;
      if (strchr(flag, 'w'))
        u_opt |= S_IWUSR;
      if (strchr(flag, 'x'))
        u_opt |= S_IXUSR;

      break;
    case 'g':
      if (strchr(flag, 'r'))
        g_opt |= S_IRGRP;
      if (strchr(flag, 'w'))
        g_opt |= S_IWGRP;
      if (strchr(flag, 'x'))
        g_opt |= S_IXGRP;

      break;

    case 'o':
      if (strchr(flag, 'r'))
        o_opt |= S_IROTH;
      if (strchr(flag, 'w'))
        o_opt |= S_IWOTH;
      if (strchr(flag, 'x'))
        o_opt |= S_IXOTH;

      break;

    case 'a':
      if (strchr(flag, 'r'))
        u_opt |= S_IRUSR | S_IRGRP | S_IROTH;
      if (strchr(flag, 'w'))
        u_opt |= S_IWUSR | S_IWGRP | S_IWOTH;
      if (strchr(flag, 'x'))
        u_opt |= S_IXUSR | S_IXGRP | S_IXOTH;

      break;
    default:
  }
}

void errorexit(const char * format, ...)
{
  va_list arg_list;

  fprintf(stderr, "%s: ", program_name);
  va_start(arg_list, format);
  vfprintf(stderr, format, arg_list);
  va_end(arg_list);

  exit(EXIT_FAILURE);
}

void output_error(const char * format, ...)
{
  va_list arg_list;

  fprintf(stderr, "%s: ", program_name);
  va_start(arg_list, format);
  vfprintf(stderr, format, arg_list);
  va_end(arg_list);
}

void usage(int status)
{
  fputs("Usage: crtfile [OPTION]... FILE...\n"
        "Mandatory arguments to long options are mandatory for short option too.\n\n"
        "\t-u, --user    Permissions for user\n"
        "\t-g, --group   Permissions for group\n"
        "\t-o, --other   Permissions for other\n"
        "\t-a, --all     Permissions for all users\n"
            "\t   r       Gives read permission\n"
            "\t   w       Gives write permission\n"
            "\t   x       Gives execute permission\n"
            "\t(-a, --all is the default option if nothing is specified.)\n\n"

        "\t-t --truncate Truncates the file(s)\n"
        "\t-v --verbose  Explain what is being done\n"

        "\t--version     Output the version information and exit\n"
        "\t--help        Output help and exit\n", stdout);

  exit(status);
}

void display_version(void)
{
  fputs("crtfile 0.2.0\n"
        "Copyright (C) 2023 Arka Mondal\n"
        "License : GNU GPL version 3 \n"
        "This program comes with ABSOLUTELY NO WARRANTY;\n"
        "This is free software, and you are welcome to redistribute it "
        "under certain conditions;\n"
        "To learn more see https://www.gnu.org/licenses/gpl-3.0.html\n", stdout);
}
