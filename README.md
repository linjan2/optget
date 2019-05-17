# optget

Get all command-line options and arguments in one go. No globals and no heap allocations.

---

**Short and long options:**

```
executable --long-option
executable -o
```

**Multiple option arguments:**

```
executable --long-option arg0 arg1
executable -o arg0 arg1
```

**Extra arguments before and after any option:**

```
executable extra0 -o arg0 arg1 extra1 extra2 extra3
```

**Sequence of short options:**

```
executable -abc
# equivalent to:
executable -a -b -c
```

**Short options can have zero or more spaces before first argument:**

```
executable -o arg
executable -oarg
```

## Usage

Create an `optget_option` array to define all options' names, argument counts, and argument storage. `optget` writes `const char*` values into the given argument storage; these are pointers to the zero-terminated strings within `argv`.

The first option in the structure is for the program's arguments. You specify how many extra arguments are allowed and `optget` overwrites this with the number of extra arguments it encountered.

After the first option come the actual command-line options. Now the argument counts specify exactly how many arguments must be given. If arguments are missing, `optget` returns `OPTGET_MISSING_ARGUMENT` and sets the error string parameter to the option string.

If an unrecognized option is encountered, `optget` returns `OPTGET_UNCRECOGNIZED_OPTION` and sets the error string parameter to the string that caused the error.

Sample program that defines three options:

```c
#include "optget.c"

int
main(int argc, char **argv)
{
    // setup storage for option arguments
    const char *exec_args[10]  = {0};  // storage for extra exec arguments
    const char *output_args[1] = {0};  // storage for --output file name argument
    const char *input_args[2]  = {0};  // storage for --input file name argument
    optget_option options[] =
    {   {"program", 0, 10},  // the executable itself (must be at index 0)
        {"output", 'o', 1},  // --output arg
        {"input",  'i', 2},  // --input arg0 arg1
        {"verbose",'v'}      // --verbose
    };
    // attach argument storage
    options[0].args = exec_args;
    options[1].args = output_args;
    options[2].args = input_args;

    unsigned int option_count = sizeof options / sizeof(optget_option);

    const char *bad_string; // output parameter to error-causing input
    int ret = optget(argv, argc, options, option_count, &bad_string);
    if (ret == OPTGET_UNRECOGNIZED_OPTION)
    {   printf("unrecognized option: %s\n", bad_string);
    }
    else if (ret == OPTGET_MISSING_ARGUMENT)
    {   printf("missing argument after: %s\n", bad_string);
    }
    else // OPTGET_SUCCESS
    {   for (unsigned int i = 0; i < options[0].argument_count; i += 1)
        {   printf("%s\n", options[0].args[i]); // print executable's arguments
        }
        if (options[1].is_set)
        {   printf("%s: %s\n", options[1].long_name, options[1].args[0]);
        }
        if (options[2].is_set)
        {   printf("%s: %s %s\n", options[2].long_name, options[2].args[0], options[2].args[1]);
        }
        if (options[3].is_set)
        {   printf("%s is set\n", options[3].long_name);
        }
    }
    

    return 0;
}
```

Running the program:

```
program --input file0 file1
--> input: file0 file1
```

```
program --input file0
--> missing argument after: --input
```

```
program hello --verbose world
--> hello
    world
    verbose is set
```

```
program --hello
--> unrecognized option: --hello
```

## Extra documentation

The option structure `optget_option` is defined as:

```c
typedef struct optget_option
{   const char *long_name;
    char short_name;
    unsigned int argument_count;
    const char **args;
    int is_set;
    unsigned int order;
} optget_option;
```

* `args` must be set to a memory location where `const char*` values can be written--one for each argument.

* `is_set` is set to `1` when the option was encountered in `argv`.

* `order` is the order of appearance of the option, in relation to other options. It's not an index into `argv`, but can be used to check whether one option came before another.

* If the same option is given multiple times, the values for the argument pointers and the order are overwritten with new ones.

`optget` relies on the standard structure of `argv`. I.e. a list of zero-terminated strings. `argc` is the number of items in `argv`, including the executable's name.