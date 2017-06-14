#include <stdio.h>
#include <setjmp.h>

# define JMP_BUF jmp_buf
# define SETJMP(a) setjmp(a)
# define LONGJMP(a,b) longjmp(a, b)

struct _zend_executor_globals {
    JMP_BUF *bailout;
};
typedef struct _zend_executor_globals zend_ext;
zend_ext z;
# define EG(v) (z.v)

#define zend_try\
{\
JMP_BUF __bailout;\
int i;\
EG(bailout)=&__bailout;\
i = SETJMP(__bailout);\
if (i==0) {\
    printf("bailout is zero!\n");\
}else{\
    printf("bailout is no zero! %d ",i);\
}\
printf("what\n");\
}

int main() {
    zend_try{
        //printf("%d",i);
        printf("Hello, World!\n");
    };
    if(SETJMP(z.bailout) == 0){
        LONGJMP(z.bailout,10);
    }


    return 0;
}