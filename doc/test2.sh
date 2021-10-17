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
