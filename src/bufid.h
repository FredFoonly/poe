
enum marktype { Marktype_None, Marktype_Line, Marktype_Char, Marktype_Block };

typedef struct mark_t* MARK; 
#define MARK_NULL ((MARK)NULL)

typedef struct buffer_t* BUFFER;
#define BUFFER_NULL ((BUFFER)NULL)
