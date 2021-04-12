#pragma warning(disable : 4129)

#include <stdio.h>
#include <string>
#include "EscapeSequence.h"

const char EscapeSequence[128] = {
    0,    1,    2,    3,    4,    5,    6,    7,    8,    9,    10,   11,   12,   13,   14,   15,
    16,   17,   18,   19,   20,   21,   22,   23,   24,   25,   26,   27,   28,   29,   30,   31,
    '\ ', '\!', '\"', '\#', '\$', '\%', '\&', '\'', '\(', '\)', '\*', '\+', '\,', '\-', '\.', '\/',
    '\0', '\1', '\2', '\3', '\4', '\5', '\6', '\7', '\8', '\9', '\:', '\;', '\<', '\=', '\>', '\?',
    '\@', '\A', '\B', '\C', '\D', '\E', '\F', '\G', '\H', '\I', '\J', '\K', '\L', '\M', '\N', '\O',
    '\P', '\Q', '\R', '\S', '\T', 'U',  '\V', '\W', '\X', '\Y', '\Z', '\[', '\\', '\]', '\^', '\_',
    '\`', '\a', '\b', '\c', '\d', '\e', '\f', '\g', '\h', '\i', '\j', '\k', '\l', '\m', '\n', '\o',
    '\p', '\q', '\r', '\s', '\t', 'u',  '\v', '\w', 'x',  '\y', '\z', '\{', '\|', '\}', '\~', 127,
};
char GetEscapeValue(char c) { return EscapeSequence[c]; }

/// Generated code
//#include <stdio.h>
//#include <ctype.h>
//int main()
//{
//	for(int i = 0; i <= 127; ++i)
//	{
//		if(isprint(i))
//		{
//			printf("'\\%c', ", i);
//		}
//		else
//		{
//			printf("%d, ", i);
//		}
//	}
//    return 0;
//}

//char GetEscapeValue(char c)
//{
//	return EscapeSequence[c];
//	switch(c)
//	{
//    case ' ':
//        c = '\ ';
//        break;
//    case '!':
//        c = '\!';
//        break;
//    case '"':
//        c = '\"';
//        break;
//    case '#':
//        c = '\#';
//        break;
//    case '$':
//        c = '\$';
//        break;
//    case '%':
//        c = '\%';
//        break;
//    case '&':
//        c = '\&';
//        break;
//    case '\'':
//        c = '\'';
//        break;
//    case '(':
//        c = '\(';
//        break;
//    case ')':
//        c = '\)';
//        break;
//    case '*':
//        c = '\*';
//        break;
//    case '+':
//        c = '\+';
//        break;
//    case ',':
//        c = '\,';
//        break;
//    case '-':
//        c = '\-';
//        break;
//    case '.':
//        c = '\.';
//        break;
//    case '/':
//        c = '\/';
//        break;
//    case '0':
//        c = '\0';
//        break;
//    case '1':
//        c = '\1';
//        break;
//    case '2':
//        c = '\2';
//        break;
//    case '3':
//        c = '\3';
//        break;
//    case '4':
//        c = '\4';
//        break;
//    case '5':
//        c = '\5';
//        break;
//    case '6':
//        c = '\6';
//        break;
//    case '7':
//        c = '\7';
//        break;
//    case '8':
//        c = '\8';
//        break;
//    case '9':
//        c = '\9';
//        break;
//    case ':':
//        c = '\:';
//        break;
//    case ';':
//        c = '\;';
//        break;
//    case '<':
//        c = '\<';
//        break;
//    case '=':
//        c = '\=';
//        break;
//    case '>':
//        c = '\>';
//        break;
//    case '?':
//        c = '\?';
//        break;
//    case '@':
//        c = '\@';
//        break;
//    case 'A':
//        c = '\A';
//        break;
//    case 'B':
//        c = '\B';
//        break;
//    case 'C':
//        c = '\C';
//        break;
//    case 'D':
//        c = '\D';
//        break;
//    case 'E':
//        c = '\E';
//        break;
//    case 'F':
//        c = '\F';
//        break;
//    case 'G':
//        c = '\G';
//        break;
//    case 'H':
//        c = '\H';
//        break;
//    case 'I':
//        c = '\I';
//        break;
//    case 'J':
//        c = '\J';
//        break;
//    case 'K':
//        c = '\K';
//        break;
//    case 'L':
//        c = '\L';
//        break;
//    case 'M':
//        c = '\M';
//        break;
//    case 'N':
//        c = '\N';
//        break;
//    case 'O':
//        c = '\O';
//        break;
//    case 'P':
//        c = '\P';
//        break;
//    case 'Q':
//        c = '\Q';
//        break;
//    case 'R':
//        c = '\R';
//        break;
//    case 'S':
//        c = '\S';
//        break;
//    case 'T':
//        c = '\T';
//        break;
//    case 'U':
//        c = 'U';
//        break;
//    case 'V':
//        c = '\V';
//        break;
//    case 'W':
//        c = '\W';
//        break;
//    case 'X':
//        c = '\X';
//        break;
//    case 'Y':
//        c = '\Y';
//        break;
//    case 'Z':
//        c = '\Z';
//        break;
//    case '[':
//        c = '\[';
//        break;
//    case '\\':
//        c = '\\';
//        break;
//    case ']':
//        c = '\]';
//        break;
//    case '^':
//        c = '\^';
//        break;
//    case '_':
//        c = '\_';
//        break;
//    case '`':
//        c = '\`';
//        break;
//    case 'a':
//        c = '\a';
//        break;
//    case 'b':
//        c = '\b';
//        break;
//    case 'c':
//        c = '\c';
//        break;
//    case 'd':
//        c = '\d';
//        break;
//    case 'e':
//        c = '\e';
//        break;
//    case 'f':
//        c = '\f';
//        break;
//    case 'g':
//        c = '\g';
//        break;
//    case 'h':
//        c = '\h';
//        break;
//    case 'i':
//        c = '\i';
//        break;
//    case 'j':
//        c = '\j';
//        break;
//    case 'k':
//        c = '\k';
//        break;
//    case 'l':
//        c = '\l';
//        break;
//    case 'm':
//        c = '\m';
//        break;
//    case 'n':
//        c = '\n';
//        break;
//    case 'o':
//        c = '\o';
//        break;
//    case 'p':
//        c = '\p';
//        break;
//    case 'q':
//        c = '\q';
//        break;
//    case 'r':
//        c = '\r';
//        break;
//    case 's':
//        c = '\s';
//        break;
//    case 't':
//        c = '\t';
//        break;
//    case 'u':
//        c = 'u';
//        break;
//    case 'v':
//        c = '\v';
//        break;
//    case 'w':
//        c = '\w';
//        break;
//    case 'x':
//        c = 'x';
//        break;
//    case 'y':
//        c = '\y';
//        break;
//    case 'z':
//        c = '\z';
//        break;
//    case '{':
//        c = '\{';
//        break;
//    case '|':
//        c = '\|';
//        break;
//    case '}':
//        c = '\}';
//        break;
//    case '~':
//        c = '\~';
//        break;
//	}
//	return c;
//}
// generated method
//#include <stdio.h>
//#include <ctype.h>
//int main()
//{
//	char buf[3] = {'\0'};
//	for(int i = 0; i < 128; ++i)
//	{
//		if(isprint(i))
//		{
//			printf("\n    case '%c':", i);
//			printf("\n        c = '\\%c';", i);
//			printf("\n        break;");
//		}
//	}
//    return 0;
//}
