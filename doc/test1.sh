# test1.sh

#!/bin/bash

# Double quotes `"` inside single quotes `'` will be interpreted as is by sh,
# the same as shell variables, so usually double quotes should be inside single
# quotes while variables should be outside single quotes.
#
# RCC will concat all the 3 arguments together.
#    1           2   3
#    v__________v  vv  v_________________________________________v
rcc 'string s = "' $1 '"; s[0] = toupper(s[0]); cout << s << endl;'

# Avoid using single quote, use `CH(c)` instead of `char c='c'`.
# CH() is a wrapper macro returns the first character passed in.
rcc 'char c = CH(f); cout << c << endl;'
