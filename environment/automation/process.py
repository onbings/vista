import os
import subprocess
import sys
import time

#
# Description
#   This function processes the given
#   command in the specified directory
#   and output the command result to
#   stdout in 'real time'.
#
# Parameters
#   _cmd - The command to execute
#   _cwd - The working directory
#
# Returns
#   This return code of the command
#
def process_cmd(_cmd, _cwd = None, _env=os.environ) :

    # Execute the command
    process = subprocess.Popen(_cmd, stdin=sys.stdin, stdout=sys.stdout, stderr=sys.stderr, cwd=_cwd, env=_env)

    # Wait for it to terminate
    while process.poll() is None :
      time.sleep(0.01)
      continue;

    return process.returncode

#
# Description
#   This function processes the given
#   command in the specified directory
#   and output the command result to
#   stdout in 'real time'.
#
# Parameters
#   _cmd   - The command to execute
#   _stdin - The data to pipe to stdin
#   _cwd   - The working directory
#
# Returns
#   This return code of the command
#
def process_cmd_pipe_stdin(_cmd, _stdin, _cwd = None, _env=os.environ) :

    # Execute the command
    process = subprocess.Popen(_cmd, stdin=subprocess.PIPE, stdout=sys.stdout, stderr=sys.stderr, cwd=_cwd, shell=True, env=_env)

    # Pipe to stdin
    process.stdin.write((_stdin).encode('utf-8'))
    process.stdin.flush()

    # Wait for it to terminate
    while process.poll() is None :
      time.sleep(0.01)
      continue;

    return process.returncode

#
# Description
#   This function processes the given
#   command in the specified directory
#   and output the command result to
#   stdout in 'real time'.
#
# Parameters
#   _cmd - The command to execute
#   _cwd - The working directory
#
# Returns
#   This return code of the command
#
def process_cmd_silent(_cmd, _cwd = None, _env=os.environ) :

    # Execute the command
    process = subprocess.Popen(_cmd, stdin=sys.stdin, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, cwd=_cwd, env=_env)

    # Wait for it to terminate
    while process.poll() is None :
      time.sleep(0.01)
      continue;

    return process.returncode
