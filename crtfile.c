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
#include <sys/stat.h>
#include <getopt.h>

static const char * program_name = "crtfile";
static mode_t perm_opt = 0;

void errorexit(const char *, ...) __attribute__ ((__noreturn__));
void output_error(const char *, ...);
void usage(int) __attribute__ ((__noreturn__));
void display_version(void);
void setperms(const char * const restrict, int);

int main(int argc, char ** argv)
{
  extern mode_t perm_opt;

  bool if_errocr;         // checks for if error occurred
  bool is_modefound;      // checks if atleast one mode flag is found
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
    {"absolute", no_argument, NULL, 'A'},
    {"verbose", no_argument, &verbose, 1},
    {NULL, 0, NULL, 0}
  };

  if_errocr = false;
  is_modefound = false;
  errno = 0;
  opt_index = 0;
  opterr = 0;
  verbose = 0;
  crt_flag = O_WRONLY | O_CREAT | O_EXCL;

  if (argc == 1)
    usage(EXIT_FAILURE);

  while ((option = getopt_long(argc, argv, "vtAu:g:o:a:", long_options, &opt_index)) != -1)
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
      case 'A':
        umask(0);
        break;
      case 0:
        break;
      case 'u':
      case 'g':
      case 'o':
      case 'a':
        is_modefound = true;
        setperms(optarg, option);
        break;
      case '?':
        if (optopt == 'u' || optopt == 'g' || optopt == 'o' || optopt == 'a')
          errorexit("argument required: '%s'\n", argv[optind - 1]);
        else
          errorexit("panic: unknown option: '%s'\n", argv[optind - 1]);
      default:
        usage(EXIT_FAILURE);
    }
  }

  if (optind == argc)
    errorexit("missing operand\n");

  if (!is_modefound && (perm_opt == 0))
    setperms("rw", 'a');
  else if (is_modefound && (perm_opt == 0))
    errorexit("panic: permission not set\n");

  for (; optind < argc; optind++)
  {
    fd = open(argv[optind], crt_flag, perm_opt);
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

void setperms(const char * const restrict perms, int user)
{
  extern mode_t perm_opt;
  size_t perms_len, i;

  perms_len = strlen(perms);

  for (i = 0; i < perms_len; i++)
  {
    if ((perms[i] != 'r') && (perms[i] != 'w') && (perms[i] != 'x'))
      errorexit("panic: unrecognized permission\n");

    if (perms[i] == 'r')
    {
      if (user == 'u')
        perm_opt |= S_IRUSR;
      else if (user == 'g')
        perm_opt |= S_IRGRP;
      else if (user == 'o')
        perm_opt |= S_IROTH;
      else
        perm_opt |= S_IRUSR | S_IRGRP | S_IROTH;
    }
    else if (perms[i] == 'w')
    {
      if (user == 'u')
        perm_opt |= S_IWUSR;
      else if (user == 'g')
        perm_opt |= S_IWGRP;
      else if (user == 'o')
        perm_opt |= S_IWOTH;
      else
        perm_opt |= S_IWUSR | S_IWGRP | S_IWOTH;
    }
    else
    {
      if (user == 'u')
        perm_opt |= S_IXUSR;
      else if (user == 'g')
        perm_opt |= S_IXGRP;
      else if (user == 'o')
        perm_opt |= S_IXOTH;
      else
        perm_opt |= S_IXUSR | S_IXGRP | S_IXOTH;
    }
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
  fputs("Usage: crtfile [OPTION]... [MODE]... FILE...\n"
        "Apply MODE to each FILE.\n"
        "Mandatory arguments to long options are mandatory for short option too.\n\n"
        "    -t, --truncate   Truncates the file(s)\n"
        "    -A, --absolute   'umask' value is ignored while setting the file permission\n"
        "    -v, --verbose    Explain what is being done\n"
        "        --version    Output the version information and exit\n"
        "        --help       Output help and exit\n\n"
        "if -t (or --truncate) is specified then MODE is ignored and instead of\n"
        "file creation, file is truncated if exists.\n\n"

        "MODE can be selected from the following options :\n"
        "    -u, --user    Permissions for user\n"
        "    -g, --group   Permissions for group\n"
        "    -o, --other   Permissions for other\n"
        "    -a, --all     Permissions for all users\n"
        "      r       Gives read permission\n"
        "      w       Gives write permission\n"
        "      x       Gives execute permission\n\n"
        "-a (or --all) is the default mode and 'rw' is the default permission "
        "if nothing is specified.\n\n"
        "Each MODE is form of '([ugoa][rwx]+)'\n", stdout);

  exit(status);
}

void display_version(void)
{
  fputs("crtfile 0.8.0\n"
        "Copyright (C) 2023 Arka Mondal\n"
        "License : GNU GPL version 3 \n"
        "This program comes with ABSOLUTELY NO WARRANTY;\n"
        "This is free software, and you are welcome to redistribute it "
        "under certain conditions;\n"
        "To learn more see https://www.gnu.org/licenses/gpl-3.0.html\n", stdout);
}
