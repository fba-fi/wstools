#if HAVE_CONFIG_H
#  include <config.h>
#endif

#include "common.h"
#include "error.h"



static void error (int exit_status, const char *mode, 
                   const char *message);

static void
error (int exit_status, const char *mode, const char *message)
{
  fprintf (stderr, "%s: %s: %s.\n", program_name, mode, message);

  if (exit_status >= 0)
    exit (exit_status);
}

/* error messages */

void
ws_warning (const char *message)
{
  error (-1, "warning", message);
}

void
ws_error (const char *message)
{
  error (-1, "ERROR", message);
}

void
ws_fatal (const char *message)
{
  error (EXIT_FAILURE, "FATAL", message);
}

/* program name */

const char *program_name = NULL;

void
set_program_name (const char *path)
{
  if (!program_name)
    program_name = xstrdup (basename (path));
}



