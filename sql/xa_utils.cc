#include "sql/xa_aux.h"
#include "my_config.h"

#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
