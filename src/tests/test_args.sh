#!/bin/bash

out=$(rcc 'cout<<"argc: "<<argc<<endl;' 'for(int i=1;i<argc;i++)cout<<argv[i]<<endl;' -- "Hello" "World!" "Have a nice day!") 

diff <(
    cat <<EOF
argc: 4
Hello
World!
Have a nice day!
EOF
) <(echo "$out") || exit 1
