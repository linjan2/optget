#ifndef OPTGET_H__
#define OPTGET_H__

#ifdef __cplusplus
extern "C" {
#endif
 
#define OPTGET_SUCCESS             0
#define OPTGET_UNRECOGNIZED_OPTION 1
#define OPTGET_MISSING_ARGUMENT    2

typedef struct optget_option
{   const char *long_name;
    char short_name; // +3 bytes padding
    unsigned int argument_count;
    const char **args;
    int is_set;
    unsigned int order;
} optget_option;

static unsigned int // returns length of matched string (counting delimiter)
optget_string_compare(const char *opt_name, const char *cur)
{   unsigned int i = 0;
    while (opt_name[i] == cur[i])
    {   if (cur[i++] == '\0')
        {   return i;
        }
    }
    return 0;
}

static int
optget_getargs(const char * const **argv_ref, int *argc_ref, const char **cur_ref, unsigned int argument_count, const char **arg_list)
{   const char * const *argv = *argv_ref;
    unsigned int argc = *argc_ref;
    const char *cur = *cur_ref;
    for (unsigned int i = 0; i < argument_count; i += 1)
    {   if (argc == 0)
        {   return OPTGET_MISSING_ARGUMENT;
        }
        arg_list[i] = cur;
        cur = *(++argv);
        argc -= 1; // count argument string
    }
    *argc_ref = argc;
    *argv_ref = argv;
    *cur_ref = cur;
    return OPTGET_SUCCESS;
}

static int
optget(const char * const *argv, int argc, optget_option *options, unsigned int option_count, const char **bad_string_ref)
{   unsigned int order = 1; // skip first
    const char *cur = *(++argv); // skip first
    argc -= 1; // skip first
    unsigned int exec_args_count = 0;
    for (; argc; )
    {   *bad_string_ref = *argv;
        if (*cur == '-')
        {   cur += 1;
            if (*cur != '-') // short option
            {   if (*cur == '\0') // no character was after dash
                {   return OPTGET_UNRECOGNIZED_OPTION;
                }
                for (unsigned int i = 1; ; ) // search for matching short option
                {   if (options[i].short_name == *cur)
                    {   options[i].is_set = 1;
                        options[i].order = order++;
                        unsigned int argument_count = options[i].argument_count;
                        if (argument_count)
                        {   const char **arg_list = options[i].args;
                            if (*(++cur) == '\0') // space was between option and argument
                            {   cur = *(++argv);
                                argc -= 1;
                            }
                            int ret = optget_getargs(&argv, &argc, &cur, argument_count, arg_list);
                            if (ret)
                            {   return ret;
                            }
                        }
                        else if (*(cur+1) != '\0') // short option sequence
                        {   cur += 1;
                            i = 0;
                            continue; // restart search loop
                        }
                        else
                        {   cur = *(++argv);
                            argc -= 1;
                        }
                        break;
                    }
                    i += 1;
                    if (i == option_count)
                    {   return OPTGET_UNRECOGNIZED_OPTION;
                    }
                }
            }
            else // long option
            {   if (*(++cur) == '\0') // no character was after double dash
                {   return OPTGET_UNRECOGNIZED_OPTION;
                }
                argc -= 1;
                unsigned int len;
                for (unsigned int i = 1; ; ) // search for matching option
                {   len = optget_string_compare(options[i].long_name, cur);
                    if (len)
                    {   options[i].is_set = 1;
                        options[i].order = order++;
                        cur = *(++argv);
                        if (options[i].argument_count)
                        {   int ret = optget_getargs(&argv, &argc, &cur, options[i].argument_count, options[i].args);
                            if (ret)
                            {   return ret;
                            }
                        }
                        break;
                    }
                    i += 1;
                    if (i == option_count)
                    {   return OPTGET_UNRECOGNIZED_OPTION;
                    }
                }
            }
        }
        else // non-option argument
        {   if (exec_args_count == options[0].argument_count) // reached maximum
            {   return OPTGET_UNRECOGNIZED_OPTION;
            }
            options[0].args[exec_args_count++] = cur;
            cur = *(++argv);
            argc -= 1;
        }
    }
    options[0].argument_count = exec_args_count; // return count of found extra arguments
    return OPTGET_SUCCESS;
}

#ifdef __cplusplus
}
#endif

#endif // OPTGET_H__
