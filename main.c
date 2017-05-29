#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include "lex_vm.h"


enum SymbolTag{
	TK_ERROR,    TK_COM_BEGIN, TK_COM_END,
	TK_UNUSED,   TK_ID,        TK_INT,       TK_CHAR,
	TK_STRING,	 TK_KW_CHAR,   TK_KW_ELSE,   TK_KW_GOTO, 
	TK_KW_IF,    TK_KW_INT,    TK_KW_RETURN, TK_KW_VOID, 
	TK_KW_WHILE, TK_OP_EQ,     TK_OP_AND,    TK_OP_OR,
};

char* token_name[] = {
	"TK_ERROR",    "TK_COM_BEGIN", "TK_COM_END",
	"TK_UNUSED",   "TK_ID",        "TK_INT",       "TK_CHAR",
	"TK_STRING",   "TK_KW_CHAR",   "TK_KW_ELSE",   "TK_KW_GOTO", 
	"TK_KW_IF",    "TK_KW_INT",    "TK_KW_RETURN", "TK_KW_VOID", 
	"TK_KW_WHILE", "TK_KW_OP_EQ",  "TK_OP_AND",    "TK_OP_OR",
    [';'] = ";", [':'] = ":", ['{'] = "{", ['}'] = "}", [','] = ",",
    ['='] = "=", ['('] = "(", [')'] = ")", ['&'] = "&", ['!'] = "!",
    ['-'] = "-", ['+'] = "+", ['*'] = "*", ['/'] = "/", ['<'] = "<",
};

SymbolElement table[] = {
	{ "/\\*"                   , TK_COM_BEGIN },
	{ "\\*/"                   , TK_COM_END   },
	{ "char"                   , TK_KW_CHAR   },
	{ "else"                   , TK_KW_ELSE   },
	{ "goto"                   , TK_KW_GOTO   },
	{ "if"                     , TK_KW_IF     },
	{ "int"                    , TK_KW_INT    },
	{ "return"                 , TK_KW_RETURN },
	{ "void"                   , TK_KW_VOID   },
	{ "while"                  , TK_KW_WHILE  },
	{ "=="                     , TK_OP_EQ     }, 
	{ "&&"                     , TK_OP_AND    }, 
	{ "\\|\\|"                 , TK_OP_OR     },
	{ ";"   , ';' }, { ":"   , ':' }, { "{"   , '{' },
	{ "}"   , '}' }, { ","   , ',' }, { "="   , '=' },
	{ "\\(" , '(' }, { "\\)" , ')' }, { "&"   , '&' },
	{ "!"   , '!' }, { "-"   , '-' }, { "\\+" , '+' },
	{ "\\*" , '*' }, { "/"   , '/' }, { "<"   , '<' },
	{ "0|[1-9][0-9]*"                          , TK_INT    },
	{ "[_a-zA-Z][_a-zA-Z0-9]*"                 , TK_ID     },
	{ "'(\\\\n|\\\\'|\\\\\\\\|[^\\\\'])'"      , TK_CHAR   },
	{ "\"(\\\\n|\\\\\"|\\\\\\\\|[^\\\\\"])*\"" , TK_STRING },
	{ "[ \n\r\t]"                              , TK_UNUSED },
	{ "."                                      , TK_ERROR  },
};



char* map_file(char *filename){
	struct stat sbuf;
	char *ptr;

	int fd = open(filename,O_RDWR);

	if(fd == -1) return NULL;

	fstat(fd,&sbuf);

	ptr = mmap(NULL, sbuf.st_size + 1, PROT_READ|PROT_WRITE, MAP_PRIVATE ,fd, 0);
	if(ptr == MAP_FAILED) return NULL;

	ptr[sbuf.st_size] = '\n';
	return ptr;
}


int main(int argc,char* argv[]){
	char* input = map_file(argv[1]);
	Lexer lex = compileLex(input,table,sizeof(table)/sizeof(SymbolElement));	

	Match m;

	while(nextMatch(&lex,&m)){
		if(m.tag == -1){
			printf("error unknown character. -> %c\n",*(m.str));
			goto ERROR;
		}
		
		if(m.tag == TK_COM_BEGIN){
			printf("\n/*");
			while(nextMatch(&lex,&m)){
				if(m.tag == TK_COM_END) break;
				printf("%.*s",m.num,m.str);
			}
			printf("*/\n\n");
			continue;
		}

		if(m.tag != TK_UNUSED){
			printf("[%-13s] -> %.*s\n",token_name[m.tag],m.num,m.str);
		}
	}

	printf("success!! get \\0 token.\n");

ERROR:
	freeLex(&lex);
	return 0;
}


