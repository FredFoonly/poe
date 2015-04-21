


void start_testing(const char* system, int verbose);
int end_testing(void);

//#define runtest(x) {_runtest(#x, x);}
#define runtest(x) {_runtest(__FILE__, __LINE__, #x, x);}

//void failtest(int code, const char* message);
void failtest(const char* fmt, ...);



void _runtest(const char* filename, int linenum, const char* testname, void (*f)(void));
