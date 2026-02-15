#ifndef _CDBG68_CMD_H_
#define _CDBG68_CMD_H_

/** Key modifiers */
enum  {
  K_NRM = 0,
  K_ESC = 1
} kmod_e;

/** function callback. */
typedef int (*cmd_f)(int, char **);

/** argument callback. */
typedef int (*arg_f)(int *, char ***);

/** debugger command. */
typedef struct {
  int          mod;  /**< command key modifier.      */
  int          key;  /**< command key.               */
  int          win;  /**< active (1 bit per window)  */
  const char * name; /**< command name.              */
  const char * desc; /**< command short description. */
  const char * help; /**< command long description   */
  cmd_f        func; /**< command function.          */
  arg_f        args; /**< argument read function.    */
} cmd_t;

typedef struct {
  int          i;    /**< window/category id.   */
  const char * name; /**< category name.        */
  const char * desc; /**< category description. */
  int          max;  /**< command count.        */
  cmd_t      * cmds; /**< command array.        */
} cmdcat_t;

/** Register a category of command. */
int cmd_register(cmdcat_t *);

/** Unregister a category of command. */
void cmd_unregister(int i);

/** Search a command by name. */
cmd_t * cmd_search(int cat, const char * name);

/** Search a command by key stroke. */
cmd_t * cmd_lookup(int cat, int mod, int key);

/** Execute a command. */
int cmd_run(cmd_t * cmd, int argc, char ** argv);

/** Get and reset last error message. */
const char * cmd_geterr(void);

/** Set last error message. */
int cmd_seterr(const char * cat, const char * desc, int override);

#endif /* #ifndef _CDBG68_CMD_H_ */
