# Predefined Template

The predefined template includes some helper functions or macros that will make your life easier using rcc.

All codes are in [template/rcc_template.h](../template/rcc_template.h)

## FOR Macros

Simple macros.

```c++
#define FOR(l, r) for (int i = l; i < r; ++i)
#define FORR(r, l) for (int i = r; i >= l; --i)

#define FORI(l, r) for (int i = l; i < r; ++i)
#define FORJ(l, r) for (int j = l; j < r; ++j)
#define FORK(l, r) for (int k = l; k < r; ++k)

#define FORRI(r, l) for (int i = r; i >= l; --i)
#define FORRJ(r, l) for (int j = r; j >= l; --j)
#define FORRK(r, l) for (int k = r; k >= l; --k)
```

## Wrappers

```c++
// Wrapper macro that returns the first character passed in.
#define CH(c) __ch(#c)

// Wrapper function of system(const char *);
int system(const std::string &cmd) { return system(cmd.c_str()); }
```

## Functions

### split

Use it to split strings into lines or tokens.

```c++
// Split string into tokens like strtok().
std::vector<std::string> split(const std::string &s, const std::string &delims = " \t\r\n\v\f");
```

### ascii

```c++
// Show the ascii value of the given characters.
void ascii(const char *characters = NULL);
```

## Traverse files

Want to go through all files in a directory and subdirectory? Use traversal/TRAVERSAL.

```c++
// This will print every file and directory in current directory and its
// subdirectory.
auto callback = [&](const string &path,
                    const string &filename,
                    bool is_file) {
    cout << path << ", " << filename << (is_file ? "" : " [dir]") << endl;
    // return 0 to continue traversing
    // return -1 to stop traversing
    // return 1 to skip traversing the directory
    return 0;
};
traversal("../.", callback);

// Same as the above codes.
TRAVERSAL("../.", {
    cout << path << ", " << filename << (is_file ? "" : " [dir]") << endl;
    return 0;
});
```
