
typedef int POE_ERR;

const char* poe_err_message(POE_ERR err);

#define POE_ERR_OK                   (0)
#define POE_ERR_BLOCK_MARK_REQ       (1) /* block mark required */
#define POE_ERR_CANNOT_NAME_UNNAMED  (2) /* cannot rename the .unnamed file */
#define POE_ERR_CANNOT_SAVE_INTERNAL (3) /* cannot save an internal file */
#define POE_ERR_MEM_FULL             (4) /* memory is full */
#define POE_ERR_CMD_FILE_NOT_FOUND   (5) /* PE.PRO file not found */
#define POE_ERR_CMD_FILES_NESTED     (6) /* command files nested too deep */ 
#define POE_ERR_MARK_TYPE_CONFLICT   (7) /* mark type conflict */
#define POE_ERR_DISK_FULL            (8) /* disk is full */
#define POE_ERR_CLOSING_FILE         (9) /* can't close file due to some error */
#define POE_ERR_DISP_TYPE            (10) /* error in set display format */
#define POE_ERR_MARGINS              (11) /* error in set margins */
#define POE_ERR_NOTABS               (12) /* error in load/save - misspelled notabs */
#define POE_ERR_REPL_STR             (13) /* error in replacement string */
#define POE_ERR_SRCH_STR             (14) /* error in search string */
#define POE_ERR_TABS                 (15) /* error in tab stops settings */
#define POE_ERR_CANT_OPEN            (16) /* can't open file */
#define POE_ERR_READING_CMD          (17) /* can't read PE.PRO or a macro file */
#define POE_ERR_READING_FILE         (18) /* can't read file */
#define POE_ERR_WRITING_FILE         (19) /* can't write file */
#define POE_ERR_FILE_NOT_FOUND       (20) /* can't find file */
#define POE_ERR_INVALID_FUNCTION     (21) /* don't recognize function */
#define POE_ERR_INVALID_KEY          (22) /* bad key in key definition, or after ? command */
#define POE_ERR_INVALID_NAME         (23) /* name given to NAME or EDIT was invalid */
#define POE_ERR_KEY_NOT_DEFINED      (24) /* no definition in PE.PRO */
#define POE_ERR_LINE_MARK_REQ        (25) /* line mark required */
#define POE_ERR_MARKED_BLOCK_EXISTS  (26) /* already have a block in another buffer */
#define POE_ERR_MISSING_FILE_NAME    (27) /* SAVE or FILE was missing file name */
#define POE_ERR_MISSING_QUOTE        (28) /* missing ', ", or ] */
#define POE_ERR_NAME_IN_USE          (29) /* attempted to NAME current buffer to a name that is already in use */
#define POE_ERR_MISSING_EQ_DEFN      (30) /* missing '=' in definition */
#define POE_ERR_NO_CHANGE_PENDING    (31) /* tried to execute [confirm change] when a change wasn't pending */
#define POE_ERR_NO_MARKED_AREA       (32) /* no mark */
#define POE_ERR_NOT_FOUND            (33) /* search text not found */
#define POE_ERR_SET_OPTION_UNK       (34) /* set of unknown option */
#define POE_ERR_SRC_DEST_CONFLICT    (35) /* overlap of source mark and target position */
#define POE_ERR_TOO_MANY_TABSTOPS    (36) /* too many tabstops */
#define POE_ERR_UNK_CMD              (37) /* unknown command */
#define POE_ERR_WRITE_PROTECTED      (38) /* attempted to write to a write-protected file */
#define POE_ERR_NO_MARKS_SAVED       (39) /* no marks to pop */
#define POE_ERR_SET_VAL_UNK          (40) /* set of unknown option value */

