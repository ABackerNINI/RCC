#!/bin/bash

out=$(rcc 'cout<<"argc: "<<argc<<endl;' 'for(int i=1;i<argc;i++)cout<<argv[i]<<endl;' -- \' "O'Reilly, What's up?")

diff <(
    cat <<EOF
argc: 3
'
O'Reilly, What's up?
EOF
) <(echo "$out") || exit 1
