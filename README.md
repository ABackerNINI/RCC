# RCC

Only for linux now.

[![GitHub](https://img.shields.io/github/license/ABackerNINI/RCC)](https://github.com/ABackerNINI/RCC/blob/master/LICENSE)

* Run short C/C++ code in terminal or shell scripts just using `rcc 'c++ code'`,
e.g. `rcc 'cout << sqrt(56) * pow(2, 13) << endl;'` or even simpler, just `rcc 'sqrt(56) * pow(2, 13)'`.

RCC will place the c++ code inside a main function, which is a pre-defined template, then compile and run the code.

* RCC allows you to access all the C/C++ libraries, it's a good extension for your shell scripts.

* RCC is also beneficial to those who are more familiar with C/C++ than with shell scripts.

## Features

* Run C/C++ code in the command line or shell scripts.
* Cache compiled executable files.
* Show syntax errors.
* Simple and fast enough with _Pre-Compiled Header_.
* Create and run permanent code.

## Is it fast?

Well, the g++/clang++ takes around 0.4 seconds to compile, even for a small piece of code. But with _Pre-Compiled Header_, it takes around __0.1__ seconds, which is acceptable for most cases. And RCC will cache the binaries, so the next time you run RCC with the same arguments, RCC should be really fast, __0.004__ seconds according to my tests.

## Requirements

* `g++` or `clang++` with c++11 or higher, on Linux.

## Install

```shell
git clone https://github.com/ABackerNINI/RCC.git
cd RCC/
bash install.sh
```

## Quick Start

### Hello World

```shell
rcc 'cout<<"Hello World!"<<endl;'
# OUTPUT: Hello World!
```

### Auto Wrap

RCC will try to wrap the code in `cout<<...<<endl;` and compile it, so you can just:

```shell
rcc '"Hello World!"'
# OUTPUT: Hello World!
```

<!-- TODO: add more examples here. -->

### Options

Compile using `clang++` with specific C++ standard and include the `filesystem` header:

```shell
rcc --clang++ -std=c++17 --include 'filesystem' 'filesystem::path p="test"; p /= "test.txt";' 'p'
# OUTPUT: "test/test.txt"
```

A lot more options are available, see `rcc --help` for more information.

### Permanent Code

The following code will create a permanent code named `try_push` that will try `git push` 10 times until it succeeds.

```shell
rcc create try_push --desc "Try 'git push' for given times, default to 10" 'int times=10; if(argc==2){times=atoi(argv[1]);} for(int i=0;i<times;i++) { cout<<"Trying "<<(i+1)<<endl; if(system("git push")==0) {break;} sleep(1); }'
```

Never mind the indentation and formatting of the C++ code, this is how you write it in a terminal.

Run it with:

```shell
rcc run try_push
```

or run with arguments:

```shell
rcc run try_push -- 15
```

## Usage

__Only two things you need to know for starting, the first is double quotes, second is single quotes.__

```shell
#!/bin/bash

# test1.sh

# Double quotes `"` inside single quotes `'` will be interpreted as is by sh,
# the same as shell variables, so usually double quotes should be inside single
# quotes while variables should be outside single quotes.
#
# RCC will concat all the 3 arguments together.
#    1              2    3
#    v__________v   vv   v_________________________________________v
rcc 'string s = "' "$1" '"; s[0] = toupper(s[0]); cout << s << endl;'

# Avoid using single quotes, use 'CH(c)' instead of 'char c='c''.
# CH() is a wrapper macro returns the first character passed in.
rcc 'char c = CH(f); cout << c << endl;'
```

While run with `./test1.sh abc`, it prints

```text
Abc
f
```

### More examples

Something else you may want to know.

```shell
#!/bin/bash

# test2.sh

# Use C++ math functions
rcc 'cout << sqrt(56) * pow(2, 13) << endl;'

# Even simpler
rcc 'sqrt(56) * pow(2, 13)'

# Use C++ stl
# Create 3 files: p0.txt p1.txt and p2.txt.
rcc '
    for(int i = 0; i < 3; ++i) {
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
    while(getline(cin, line)) {
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
    auto func = [&](int a) -> bool { return a == 10; };
    cout << func(10) << endl;
'
# or use `--function` to define a function
rcc --function 'int add(int a, int b) { return a + b; }' 'cout << add(1, 2) << endl;'
```

## Define your own template

Before that, you may want to see the [Pre-Defined Template](./docs/PredefinedTemplate.md).

You could add your own helper functions in `template/rcc_template.hpp`, or macros or include library you want to use or whatever.

It's not recommended to add more code in `template/rcc_template.cpp` because it will slow down the compiling process. Add code in the `rcc_template.hpp` file instead, the `install.sh` will compile the header into a _Pre-Compiled Header_.

Steps:

* Edit the template files in `template/rcc_template.[hpp/cpp]`.
* Edit the `template/test.cpp`, then `./test.sh` to test your code.
* Finally `./install.sh`.

## Uninstall

```shell
bash uninstall.sh
```

## License

[`MIT LICENSE`](./LICENSE)
