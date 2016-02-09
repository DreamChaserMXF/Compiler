# Compiler
PL/0 Compiler

feature:

1. definition
1.1 procedure (parameter<integer/char> passed by value or reference)
1.2 function (procedure with a return value)
1.3 variable (integer, char, array of integer/char)
1.4 constant (constant integer/char)

2. program paradigms
2.1 nested programming(nested variable/procedure/function scope)
2.2 sequence structure (expression, assignment)
2.3 branch structure (if, switch)
2.4 loop structure (while, for, continue & break)

3. error handling
3.1 locate error
3.2 continue compile after error 

4. other features that take effort
4.1 line comment and block comment in source code
4.2 escape character handling
4.3 continue and break among loop body
4.4 automatic type inference when calling write()
4.5 minimized the number of temporary variables
4.6 optimized the constant operating process while compiling

5. limitation
5.1 only supports simple condition statement(if a > b), doesn't support compound condition statement such as (a > b || c < d)
5.2 only support single file compiling
5.3 no general optimization in intermediate code
5.4 only for intel-x86 architecture with 32 bits assembly language.