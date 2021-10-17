# RCC

Only for linux now.

[![GitHub](https://img.shields.io/github/license/ABackerNINI/RCC)](https://github.com/ABackerNINI/RCC/blob/master/LICENSE)

* Run short C/C++ codes in terminal or shell scripts just using `rcc 'c++ codes'`,
e.g. `rcc 'cout << sqrt(56) * pow(2,13) << endl;'`.
RCC will place the c++ codes inside a main function, which is a pre-defined template, which includes frequently-used headers, then compile and run the codes.

* RCC allows you to access all the C/C++ libraries, it's a good extension for your shell scripts.

* RCC is also beneficial to those who are familiar with C/C++ but not shell scripts.

## Features

* Run C/C++ codes in the command line or shell scripts.
* Cache compiled executable files.
* Show syntax errors.
* Simple and fast enough with Pre-Compiled Header.

## Requirements

* g++ with c++11 or higher

## Install

```shell
git clone https://github.com/ABackerNINI/RCC.git
cd RCC
chmod +x ./install.sh
./install.sh
```

## Usage

__Only two things you need to know, the first is double quotes, second is single quotes.__

```shell
# test1.sh

#!/bin/bash

# Double quotes `"` inside single quotes `'` will be interpreted as is by sh,
# the same as shell variables, so usually double quotes should be inside single
# quotes while variables should be outside single quotes.
#
# RCC will concat all the 3 arguments together.
#    1             2   3
#    v__________v  vv  v_________________________________________v
rcc 'string s = "' $1 '"; s[0] = toupper(s[0]); cout << s << endl;'

# Avoid using single quote, use `CH(c)` instead of `char c='c'`.
# CH() is a wrapper macro returns the first character passed in.
rcc 'char c = CH(f); cout << c << endl;'
```

While run with `./test1.sh abc`, it prints

```
Abc
f
```

### More examples

Something else you may want to know.

```shell
# test2.sh

#!/bin/bash

# Use C++ math functions
rcc 'cout << sqrt(56) * pow(2,13) << endl;'

# Use C++ stl
# Create 3 files: p0.txt p1.txt and p2.txt.
rcc '
    for(int i = 0; i < 3; ++i){
        string cmd = "touch p" + to_string(i) + ".txt";
        system(cmd.c_str());
    }
'

# Pipes
# Print the first column of output of `ls -l`.
# split() is a pre-defined helper function in the template.
ls -l | rcc '
    string line;
    getline(cin, line);
    while(getline(cin, line)){
        vector<string> v = split(line);
        cout << v[0] << endl;
    }
'

# File redirection
# Equal to `cat Makefile`.
rcc '
    char c;
    while((c = getchar()) != EOF) {
        cout << c;
    }
' < Makefile

# Want to define functions? use Lambda
rcc '
    auto f = [&](int a) -> bool { return a==10; };
    cout << f(10) << endl;
'
```

## Define your own template

Before that, you may want to see the [Pre-Defined Template](./doc/PredefinedTemplate.md).

You could add your own helper functions in `template/rcc_template.h`, or macros or include library you want to use or whatever. Remember to define functions as `inline`.

It's not recommended to add more codes in `template/rcc_template.cpp` because it will slow down the compiling process. Add codes in the `rcc_template.h` file, `make install` will compile the header into a Pre-Compiled Header.

Steps:
* Edit the template files in `template/rcc_template.[h/cpp]`.
* Edit the `template/test.cpp`, then `./test.sh` to test your codes.
* Finally `./install.sh`.

## Is it fast?

Well, the g++/clang++ will take around 0.4 seconds to compile, even if a small piece of codes. But with _Pre-Compiled Header_, it takes around 0.1 seconds, which is acceptable for most cases. And RCC will cache the binaries, so the next time you run RCC with the same arguments, RCC should be really fast, 0.004 seconds according to my tests.

Of course, 0.1 seconds is far more than 0.001 seconds which you could easily achieve with raw shell commands. Use RCC just when it's hard to write shell commands for what you want to achieve while C++ does.

## Uninstall:

```shell
rm -rf "$HOME/.local/rcc"
rm -rf "$HOME/.cache/rcc"
sudo rm -rf "/usr/local/bin/rcc"
```

## License

[`MIT LICENSE`](./LICENSE)
