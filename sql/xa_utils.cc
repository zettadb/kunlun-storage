#include "sql/xa_aux.h"
#include "my_config.h"

#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "my_sqlcommand.h"
#include "my_command.h"
#include "my_sys.h"

extern int ddc_mode;
char *serialize_xid(char *buf, long fmt, long gln, long bln,
                           const char *dat) {
  int i;
  char *c = buf;
  if (ddc_mode) {
    buf[0] = '\'';
    memcpy(buf + 1, dat, gln + bln);
    buf[gln + bln + 1] = '\'';
    buf[gln + bln + 2] = '\0';
    return buf;
  }

  /*
    Build a string like following pattern:
      X'hex11hex12...hex1m',X'hex21hex22...hex2n',11
    and store it into buf.
    Here hex1i and hex2k are hexadecimals representing XID's internal
    raw bytes (1 <= i <= m, 1 <= k <= n), and `m' and `n' even numbers
    half of which corresponding to the lengths of XID's components.
  */
  *c++ = 'X';
  *c++ = '\'';
  for (i = 0; i < gln; i++) {
    *c++ = _dig_vec_lower[static_cast<uchar>(dat[i]) >> 4];
    *c++ = _dig_vec_lower[static_cast<uchar>(dat[i]) & 0x0f];
  }
  *c++ = '\'';

  *c++ = ',';
  *c++ = 'X';
  *c++ = '\'';
  for (; i < gln + bln; i++) {
    *c++ = _dig_vec_lower[static_cast<uchar>(dat[i]) >> 4];
    *c++ = _dig_vec_lower[static_cast<uchar>(dat[i]) & 0x0f];
  }
  *c++ = '\'';
  sprintf(c, ",%lu", fmt);

  return buf;
}
/*
  @param [in] buf serialized xid string
  @retval true on format error; false on success.
*/
bool deserialize_xid(const char *buf, long &/*fmt*/, long &gln, long &bln,
                     char *dat)
{
  size_t bufl = strlen(buf);
  if (bufl < 3 || buf[0] != '\'' || buf[bufl - 1] != '\'')
    return true;
  memcpy(dat, buf + 1, bufl - 2);
  gln = bufl - 2;
  bln = 0;
  return false;
}

const char *get_sql_command_name(enum enum_sql_command cmd)
{
  // must always match the definition of enum enum_sql_command!!!
  static const char *sql_cmd_names[] = {
    "SQLCOM_SELECT",
    "SQLCOM_CREATE_TABLE",
    "SQLCOM_CREATE_INDEX",
    "SQLCOM_ALTER_TABLE",
    "SQLCOM_UPDATE",
    "SQLCOM_INSERT",
    "SQLCOM_INSERT_SELECT",
    "SQLCOM_DELETE",
    "SQLCOM_TRUNCATE",
    "SQLCOM_DROP_TABLE",
    "SQLCOM_DROP_INDEX",
    "SQLCOM_SHOW_DATABASES",
    "SQLCOM_SHOW_TABLES",
    "SQLCOM_SHOW_FIELDS",
    "SQLCOM_SHOW_KEYS",
    "SQLCOM_SHOW_VARIABLES",
    "SQLCOM_SHOW_STATUS",
    "SQLCOM_SHOW_ENGINE_LOGS",
    "SQLCOM_SHOW_ENGINE_STATUS",
    "SQLCOM_SHOW_ENGINE_MUTEX",
    "SQLCOM_SHOW_PROCESSLIST",
    "SQLCOM_SHOW_MASTER_STAT",
    "SQLCOM_SHOW_SLAVE_STAT",
    "SQLCOM_SHOW_GRANTS",
    "SQLCOM_SHOW_CREATE",
    "SQLCOM_SHOW_CHARSETS",
    "SQLCOM_SHOW_COLLATIONS",
    "SQLCOM_SHOW_CREATE_DB",
    "SQLCOM_SHOW_TABLE_STATUS",
    "SQLCOM_SHOW_TRIGGERS",
    "SQLCOM_LOAD",
    "SQLCOM_SET_OPTION",
    "SQLCOM_LOCK_TABLES",
    "SQLCOM_UNLOCK_TABLES",
    "SQLCOM_GRANT",
    "SQLCOM_CHANGE_DB",
    "SQLCOM_CREATE_DB",
    "SQLCOM_DROP_DB",
    "SQLCOM_ALTER_DB",
    "SQLCOM_REPAIR",
    "SQLCOM_REPLACE",
    "SQLCOM_REPLACE_SELECT",
    "SQLCOM_CREATE_FUNCTION",
    "SQLCOM_DROP_FUNCTION",
    "SQLCOM_REVOKE",
    "SQLCOM_OPTIMIZE",
    "SQLCOM_CHECK",
    "SQLCOM_ASSIGN_TO_KEYCACHE",
    "SQLCOM_PRELOAD_KEYS",
    "SQLCOM_FLUSH",
    "SQLCOM_KILL",
    "SQLCOM_ANALYZE",
    "SQLCOM_ROLLBACK",
    "SQLCOM_ROLLBACK_TO_SAVEPOINT",
    "SQLCOM_COMMIT",
    "SQLCOM_SAVEPOINT",
    "SQLCOM_RELEASE_SAVEPOINT",
    "SQLCOM_SLAVE_START",
    "SQLCOM_SLAVE_STOP",
    "SQLCOM_START_GROUP_REPLICATION",
    "SQLCOM_STOP_GROUP_REPLICATION",
    "SQLCOM_BEGIN",
    "SQLCOM_CHANGE_MASTER",
    "SQLCOM_CHANGE_REPLICATION_FILTER",
    "SQLCOM_RENAME_TABLE",
    "SQLCOM_RESET",
    "SQLCOM_PURGE",
    "SQLCOM_PURGE_BEFORE",
    "SQLCOM_SHOW_BINLOGS",
    "SQLCOM_SHOW_OPEN_TABLES",
    "SQLCOM_HA_OPEN",
    "SQLCOM_HA_CLOSE",
    "SQLCOM_HA_READ",
    "SQLCOM_SHOW_SLAVE_HOSTS",
    "SQLCOM_DELETE_MULTI",
    "SQLCOM_UPDATE_MULTI",
    "SQLCOM_SHOW_BINLOG_EVENTS",
    "SQLCOM_DO",
    "SQLCOM_SHOW_WARNS",
    "SQLCOM_EMPTY_QUERY",
    "SQLCOM_SHOW_ERRORS",
    "SQLCOM_SHOW_STORAGE_ENGINES",
    "SQLCOM_SHOW_PRIVILEGES",
    "SQLCOM_HELP",
    "SQLCOM_CREATE_USER",
    "SQLCOM_DROP_USER",
    "SQLCOM_RENAME_USER",
    "SQLCOM_REVOKE_ALL",
    "SQLCOM_CHECKSUM",
    "SQLCOM_CREATE_PROCEDURE",
    "SQLCOM_CREATE_SPFUNCTION",
    "SQLCOM_CALL",
    "SQLCOM_DROP_PROCEDURE",
    "SQLCOM_ALTER_PROCEDURE",
    "SQLCOM_ALTER_FUNCTION",
    "SQLCOM_SHOW_CREATE_PROC",
    "SQLCOM_SHOW_CREATE_FUNC",
    "SQLCOM_SHOW_STATUS_PROC",
    "SQLCOM_SHOW_STATUS_FUNC",
    "SQLCOM_PREPARE",
    "SQLCOM_EXECUTE",
    "SQLCOM_DEALLOCATE_PREPARE",
    "SQLCOM_CREATE_VIEW",
    "SQLCOM_DROP_VIEW",
    "SQLCOM_CREATE_TRIGGER",
    "SQLCOM_DROP_TRIGGER",
    "SQLCOM_XA_START",
    "SQLCOM_XA_END",
    "SQLCOM_XA_PREPARE",
    "SQLCOM_XA_COMMIT",
    "SQLCOM_XA_ROLLBACK",
    "SQLCOM_XA_RECOVER",
    "SQLCOM_SHOW_PROC_CODE",
    "SQLCOM_SHOW_FUNC_CODE",
    "SQLCOM_ALTER_TABLESPACE",
    "SQLCOM_INSTALL_PLUGIN",
    "SQLCOM_UNINSTALL_PLUGIN",
    "SQLCOM_BINLOG_BASE64_EVENT",
    "SQLCOM_SHOW_PLUGINS",
    "SQLCOM_CREATE_SERVER",
    "SQLCOM_DROP_SERVER",
    "SQLCOM_ALTER_SERVER",
    "SQLCOM_CREATE_EVENT",
    "SQLCOM_ALTER_EVENT",
    "SQLCOM_DROP_EVENT",
    "SQLCOM_SHOW_CREATE_EVENT",
    "SQLCOM_SHOW_EVENTS",
    "SQLCOM_SHOW_CREATE_TRIGGER",
    "SQLCOM_SHOW_PROFILE",
    "SQLCOM_SHOW_PROFILES",
    "SQLCOM_SIGNAL",
    "SQLCOM_RESIGNAL",
    "SQLCOM_SHOW_RELAYLOG_EVENTS",
    "SQLCOM_GET_DIAGNOSTICS",
    "SQLCOM_ALTER_USER",
    "SQLCOM_EXPLAIN_OTHER",
    "SQLCOM_SHOW_CREATE_USER",
    "SQLCOM_SHUTDOWN",
    "SQLCOM_SET_PASSWORD",
    "SQLCOM_ALTER_INSTANCE",
    "SQLCOM_INSTALL_COMPONENT",
    "SQLCOM_UNINSTALL_COMPONENT",
    "SQLCOM_CREATE_ROLE",
    "SQLCOM_DROP_ROLE",
    "SQLCOM_SET_ROLE",
    "SQLCOM_GRANT_ROLE",
    "SQLCOM_REVOKE_ROLE",
    "SQLCOM_ALTER_USER_DEFAULT_ROLE",
    "SQLCOM_IMPORT",
    "SQLCOM_CREATE_RESOURCE_GROUP",
    "SQLCOM_ALTER_RESOURCE_GROUP",
    "SQLCOM_DROP_RESOURCE_GROUP",
    "SQLCOM_SET_RESOURCE_GROUP",
    "SQLCOM_CLONE",
    "SQLCOM_LOCK_INSTANCE",
    "SQLCOM_UNLOCK_INSTANCE",
    "SQLCOM_RESTART_SERVER",
    "SQLCOM_CREATE_SRS",
    "SQLCOM_DROP_SRS",
    "SQLCOM_SHOW_USER_STATS",
    "SQLCOM_SHOW_TABLE_STATS",
    "SQLCOM_SHOW_INDEX_STATS",
    "SQLCOM_SHOW_CLIENT_STATS",
    "SQLCOM_SHOW_THREAD_STATS",
    "SQLCOM_LOCK_TABLES_FOR_BACKUP",
    "SQLCOM_CREATE_COMPRESSION_DICTIONARY",
    "SQLCOM_DROP_COMPRESSION_DICTIONARY",
    // Always only append new commands, OR some tests fill fail.
    "SQLCOM_SHOW_THREADPOOL_STAT",
    /* This should be the last !!! */
    "SQLCOM_END"
  };
  assert(cmd >= 0 && cmd <= SQLCOM_END);
  return sql_cmd_names[cmd];
}

const char *get_server_command_name(enum enum_server_command cmd)
{
  static const char *server_cmd_names[] =
  {
    "COM_SLEEP",
    "COM_QUIT",       /**< See @ref page_protocol_com_quit */
    "COM_INIT_DB",    /**< See @ref page_protocol_com_init_db */
    "COM_QUERY",      /**< See @ref page_protocol_com_query */
    "COM_FIELD_LIST", /**< Deprecated. See @ref page_protocol_com_field_list */
    "COM_CREATE_DB", /**< Currently refused by the server. See ::dispatch_command */
    "COM_DROP_DB",   /**< Currently refused by the server. See ::dispatch_command */
    "COM_REFRESH",   /**< Deprecated. See @ref page_protocol_com_refresh */
    "COM_DEPRECATED_1", /**< Deprecated", used to be COM_SHUTDOWN */
    "COM_STATISTICS",   /**< See @ref page_protocol_com_statistics */
    "COM_PROCESS_INFO", /**< Deprecated. See @ref page_protocol_com_process_info */
    "COM_CONNECT",      /**< Currently refused by the server. */
    "COM_PROCESS_KILL", /**< Deprecated. See @ref page_protocol_com_process_kill */
    "COM_DEBUG",        /**< See @ref page_protocol_com_debug */
    "COM_PING",         /**< See @ref page_protocol_com_ping */
    "COM_TIME",         /**< Currently refused by the server. */
    "COM_DELAYED_INSERT", /**< Functionality removed. */
    "COM_CHANGE_USER",    /**< See @ref page_protocol_com_change_user */
    "COM_BINLOG_DUMP",    /**< See @ref page_protocol_com_binlog_dump */
    "COM_TABLE_DUMP",
    "COM_CONNECT_OUT",
    "COM_REGISTER_SLAVE",
    "COM_STMT_PREPARE", /**< See @ref page_protocol_com_stmt_prepare */
    "COM_STMT_EXECUTE", /**< See @ref page_protocol_com_stmt_execute */
    /** See  @ref page_protocol_com_stmt_send_long_data */
    "COM_STMT_SEND_LONG_DATA",
    "COM_STMT_CLOSE", /**< See @ref page_protocol_com_stmt_close */
    "COM_STMT_RESET", /**< See @ref page_protocol_com_stmt_reset */
    "COM_SET_OPTION", /**< See @ref page_protocol_com_set_option */
    "COM_STMT_FETCH", /**< See @ref page_protocol_com_stmt_fetch */
    "COM_DAEMON",
    "COM_BINLOG_DUMP_GTID",
    "COM_RESET_CONNECTION", /**< See @ref page_protocol_com_reset_connection */
    "COM_CLONE",
    "COM_END" /**< Not a real command. Refused. */
  };
  assert(cmd >= 0 && cmd <= COM_END);
  return server_cmd_names[cmd];
}
