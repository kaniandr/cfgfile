# CFG File

This is a simple configuration file parser. A configuration file has the follwing structure:

```
# Comments can be placed after '#' character.

VARIABLE_1     VALUE_1                    # VARIABLE_1='VALUE_1'
VARIABLE_2 :   VALUE_2                    # VARIABLE_2='   VALUE_2'
VARIABLE_3 :                              # VARIABLE_3=''
VARIABLE_4     1
VARIABLE_5     ${VARIABLE_${VARIABLE_4}}  # VARIABLE_5='VALUE_1'
```
