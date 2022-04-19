/* 
 * Copyright (c) 2006, 2019, Oracle and/or its affiliates. All rights reserved.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License, version 2.0,
   as published by the Free Software Foundation.

   This program is also distributed with certain software (including
   but not limited to OpenSSL) that is licensed under separate terms,
   as designated in a particular file or component or in included license
   documentation.  The authors of MySQL hereby grant you an additional
   permission to link the program and your derivative works with the
   separately licensed software that they have included with MySQL.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License, version 2.0, for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA
*/
#ifndef _BIODIRECT_POPEN_H_
#define _BIODIRECT_POPEN_H_

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <string>
#include "errorcup.h"

namespace BinlogBackupPlugin {
// biodirectional popen implementation
class BiodirectPopen : public BinlogBackupPlugin::ErrorCup {
 public:
  explicit BiodirectPopen(const char *command)
      : command_(command),
        read_from_child_(true),
        write_to_child_(false),
        set_close_on_exe_(true),
        wait_status_(true) {
    for (int i = 0; i < 2; i++) {
      stdout_parent_read_fd[i] = -1;
      stderr_parent_read_fd[i] = -1;
      stdin_parent_write_fd[i] = -1;
    }
    write_to_fp_ = nullptr;
    read_from_stdout_fp_ = nullptr;
    read_from_stderr_fp_ = nullptr;
  }
  ~BiodirectPopen();

 private:
  bool popenImpl();
  void pclose();
  // Get and inspect the status return by child
  bool waitStatus();

 public:
  bool Launch(const char *mode);
  int readOutputLineByLine(char *, size_t);

  // file descriptor
  int getWriteFd() const;
  int getReadStdOutFd() const;
  int getReadStdErrFd() const;

  // FILE stream
  FILE *getWriteFp() const;
  FILE *getReadStdOutFp() const;
  FILE *getReadStdErrFp() const;

  int get_chiled_status() const;
  void set_block(bool);

  void closeAllFd();

 private:
  // forbide copy
  BiodirectPopen(const BiodirectPopen &rht) = delete;
  BiodirectPopen &operator=(const BiodirectPopen &rht) = delete;

 private:
  std::string command_;
  bool read_from_child_;
  bool write_to_child_;
  bool set_close_on_exe_;
  // Get the child status or not
  bool wait_status_;
  int child_return_code_;
  int stdout_parent_read_fd[2];
  int stderr_parent_read_fd[2];
  int stdin_parent_write_fd[2];

  FILE *write_to_fp_;
  FILE *read_from_stdout_fp_;
  FILE *read_from_stderr_fp_;
  pid_t child_pid_;
};

}  // namespace BinlogBackupPlugin
#endif /*_BIODIRECT_POPEN_H_*/
