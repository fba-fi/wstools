#ifndef WS_ERROR_H
#define WS_ERROR_H 1

#include <common.h>

BEGIN_C_DECLS

extern const char *program_name;
extern void set_program_name	(const char *argv0);

extern void ws_warning		(const char *message);
extern void ws_error		(const char *message);
extern void ws_fatal		(const char *message);

END_C_DECLS

#endif /* !WS_ERROR_H */
