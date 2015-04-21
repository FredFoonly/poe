
#define VIEW_FLG_INSERTMODE (1<<0)


typedef struct viewport_t* VIEWPTR;

VIEWPTR view_alloc(BUFFER pbuf, int rows, int cols);
void view_free(VIEWPTR pview);

BUFFER view_buffer(VIEWPTR pview);
void view_get_port(VIEWPTR pview, int* ptop, int* pleft, int* pbot, int* pright);
void view_get_portsize(VIEWPTR pview, int* pheight, int* pwidth);
void view_get_cursor(VIEWPTR pview, int* prow, int* pcol);

PROFILEPTR view_get_profile(VIEWPTR pview);
PROFILEPTR view_get_data_profile(VIEWPTR pview);
PROFILEPTR view_get_cmd_profile(VIEWPTR pview);
void view_setflags(VIEWPTR pview, int flags);
void view_clrflags(VIEWPTR pview, int flags);
int view_tstflags(VIEWPTR pview, int flags);


void view_set_vsize(VIEWPTR pview, int);
void view_set_hsize(VIEWPTR pview, int);


// command support
POE_ERR view_move_cursor_by(VIEWPTR view, int rows, int cols);
POE_ERR view_move_port_by(VIEWPTR view, int rows, int cols);
POE_ERR view_move_cursor_to(VIEWPTR view, int rows, int cols);
POE_ERR view_move_port_to(VIEWPTR view, int rows, int cols);

int view_get_insertmode(VIEWPTR pview);
void view_set_insertmode(VIEWPTR pview, int insertmode);
void view_toggle_insertmode(VIEWPTR pview);

