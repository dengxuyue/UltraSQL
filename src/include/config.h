#ifndef _CONFIG_H_
#define _CONFIG_H_

/**
#define MYSQL_SUPPORT      1
#define POSTGRESQL_SUPPORT 1
*/

#define MAX(x, y) ( (x) > (y) ? (x) : (y) )

#define DEBUG_ON 0

#define GEN_PTREE 1

#define HISTORY_PATH_LENGTH  64
#define HISTORY_ENTRY_LIMIT  128
#define USQL_HISTORY ".usql_history"
#define USQL_PROFILE ".usql_profile"

#define PROFILE_LINE_LENGTH   128
#define PASSWORD_ENTRY_LIMIT  32
#define SETTING_ENTRY_LIMIT   32

/*current possible protocal: cliv2, pq, mysql, odbc*/
#define LOGON_PROTOCAL_LENGTH   8
#define LOGON_DATASOURCE_LENGTH 64
#define LOGON_USERNAME_LENGTH   32
#define LOGON_PASSWORD_LENGTH   32
#define LOGON_DBNAME_LENGTH     32
#define LOGON_SOCKET_LENGTH     128
#define SET_DELIMITER_LENGTH    4
#define SET_WIDTH_LENGTH        16

#define BUFFER_PARCELS_LIMIT   256
#define BUFFER_PARCEL_BLOCK    64
#define BUFFER_HEADLINE_BLOCK  32

#define DEFAULT_SCREEN_WIDTH  128
#define DEFAULT_SCREEN_HEIGHT 50
#define DEFAULT_SCREEN_PAGER  "less"

#define STATEMENT_NEST_LEVEL    64
#define STATEMENT_LENGTH_LIMIT  1024

#define CONN_SESSION_ALIAS_LENGTH 8
#define CONN_SESSION_ENTRY_LIMIT  8
#define CONN_INFO_STRING_LENGTH   128

#endif /* _CONFIG_H_ */

