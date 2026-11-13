#!/usr/bin/env bash
# Deploy a build to the configured remote target and optionally run it under gdbserver.
# Project-specific values are read from .vscode/settings.json.
#
# Usage: remote-task.sh <action> [args]
#   stop [preset]                       stop gdbserver and configured processes
#   mkdir [preset]                      create the configured remote directories
#   deploy <build-name> [preset]        rsync configured build artifacts to the target
#   debug <preset> <exe>    stop, deploy and run an executable under gdbserver

set -u

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
SETTINGS="${REMOTE_TASK_SETTINGS:-${ROOT}/.vscode/settings.json}"
export USER="${USER:-$(id -un)}"
export REMOTE_TASK_ROOT="${ROOT}"

log() { printf '[remote-task %s] %s\n' "$(date +%H:%M:%S)" "$*"; }
die() { log "ERROR: $*" >&2; exit 1; }
run() { log "\$ $*"; "$@"; }

cfg() {
  python3 - "${SETTINGS}" "$1" "${2-}" <<'PY'
import json, os, re, sys

path, key, default = sys.argv[1:4]
root = os.environ["REMOTE_TASK_ROOT"]

def strip_comments(s):
    out, i, n, in_str = [], 0, len(s), False
    while i < n:
        c = s[i]
        if in_str:
            out.append(c)
            if c == "\\" and i + 1 < n:
                i += 1
                out.append(s[i])
            elif c == '"':
                in_str = False
        elif c == '"':
            in_str = True
            out.append(c)
        elif s.startswith("//", i):
            while i < n and s[i] != "\n":
                i += 1
            continue
        elif s.startswith("/*", i):
            j = s.find("*/", i + 2)
            i = n if j < 0 else j + 2
            continue
        else:
            out.append(c)
        i += 1
    return re.sub(r",(\s*[}\]])", r"\1", "".join(out))

def expand(value):
    value = value.replace("${workspaceFolderBasename}", os.path.basename(root))
    value = value.replace("${workspaceFolder}", root)
    return re.sub(r"\$\{env:([^}]+)\}", lambda m: os.environ.get(m.group(1), ""), value)

try:
    with open(path) as settings_file:
        settings = json.loads(strip_comments(settings_file.read()))
except Exception as error:
    sys.exit("cannot read %s: %s" % (path, error))

value = settings.get(key)
if value is None:
    value = default.split("\n") if default else []
    value = value if len(value) != 1 else value[0]
items = value if isinstance(value, list) else [value]
for item in items:
    if item != "":
        print(expand(str(item)))
PY
}

require() {
  local value
  value="$(cfg "$1")" || exit 1
  [[ -n "${value}" ]] || die "'$1' is missing in ${SETTINGS}"
  printf '%s' "${value}"
}

command -v python3 >/dev/null || die "python3 is required to read ${SETTINGS}"
[[ -f "${SETTINGS}" ]] || die "settings file not found: ${SETTINGS}"
[[ $# -ge 1 ]] || die "usage: $0 <stop|mkdir|deploy|debug> [args]"

ACTION="$1"
shift

TARGET_USER="$(require targetUser)" || exit 1
TARGET_HOST="$(require targetHost)" || exit 1
PORT="$(require cppGdbPort)" || exit 1
TARGET="${TARGET_USER}@${TARGET_HOST}"
DEPLOY_DIR="$(cfg remoteDeployDir '/tmp/${env:USER}_${workspaceFolderBasename}')"
BUILD_DIR_TEMPLATE="$(cfg remoteBuildDir '${workspaceFolder}/out/build/{BUILD}/binaries')"
mapfile -t SYNC_DIRS < <(cfg remoteSyncDirs $'bin\nlib')
mapfile -t SYNC_MAPPINGS < <(cfg remoteSyncMappings)
mapfile -t EXTRA_SYNC < <(cfg remoteExtraSync)
mapfile -t MKDIRS < <(cfg remoteMkdirs)
mapfile -t STOP_PROCESSES < <(cfg remoteStopProcesses)
LD_PATH="$(cfg remoteLdLibraryPath)"
GDBSERVER="$(cfg remoteGdbserver gdbserver)"
GDB_LISTEN="$(cfg remoteGdbserverListen localhost)"
RELEASE_REGEX="$(cfg remoteReleasePresetRegex '-r(e)?lease$')"
SSH_KEY="$(cfg remoteSshKey)"
SSH_KEY_REGEX="$(cfg remoteSshKeyPresetRegex)"
SSH_ARGS=()
RSYNC_RSH=ssh

log "settings: ${SETTINGS}"
log "action=${ACTION} target=${TARGET} gdb-port=${PORT} deploy-dir=${DEPLOY_DIR}"

select_transport() {
  local preset="${1-}"
  SSH_ARGS=()
  RSYNC_RSH=ssh
  if [[ -n "${preset}" && -n "${SSH_KEY}" && -n "${SSH_KEY_REGEX}" && "${preset}" =~ ${SSH_KEY_REGEX} ]]; then
    [[ -f "${SSH_KEY}" ]] || die "SSH identity file not found: ${SSH_KEY}"
    SSH_ARGS=(-i "${SSH_KEY}")
    RSYNC_RSH="ssh -i \"${SSH_KEY}\""
  fi
}

remote_stop() {
  select_transport "${1-}"
  log "Stopping gdbserver on port ${PORT} on ${TARGET}"
  run ssh "${SSH_ARGS[@]}" "${TARGET}" "pkill -f \"gdbserver.*${PORT}\" || true" || true
  local name
  for name in "${STOP_PROCESSES[@]}"; do
    log "Stopping process '${name}' on ${TARGET}"
    run ssh "${SSH_ARGS[@]}" "${TARGET}" "killall -9 ${name} || true" || true
  done
}

remote_mkdir() {
  select_transport "${1-}"
  local command="mkdir -p ${DEPLOY_DIR}" directory
  for directory in "${MKDIRS[@]}"; do
    command+=" ${DEPLOY_DIR}/${directory}"
  done
  log "Creating remote directories under ${DEPLOY_DIR}"
  run ssh "${SSH_ARGS[@]}" "${TARGET}" "${command}"
}

remote_deploy() {
  select_transport "${2-}"
  local build_dir="${BUILD_DIR_TEMPLATE//\{BUILD\}/$1}" directory pair source destination
  log "Deploying '${build_dir}' to ${TARGET}:${DEPLOY_DIR}"
  if ((${#SYNC_MAPPINGS[@]})); then
    for pair in "${SYNC_MAPPINGS[@]}"; do
      source="${pair%%:*}"
      destination="${pair#*:}"
      run rsync -e "${RSYNC_RSH}" -av "${build_dir}/${source}/" "${TARGET}:${DEPLOY_DIR}/${destination}/" || return 1
    done
  else
    for directory in "${SYNC_DIRS[@]}"; do
      run rsync -e "${RSYNC_RSH}" -av "${build_dir}/${directory}/" "${TARGET}:${DEPLOY_DIR}/${directory}/" || return 1
    done
  fi
  for pair in "${EXTRA_SYNC[@]}"; do
    run rsync -e "${RSYNC_RSH}" -av "${ROOT}/${pair%%:*}/" "${TARGET}:${DEPLOY_DIR}/${pair#*:}/" || return 1
  done
  log "Deploy done"
}

remote_debug() {
  local preset="$1" executable="$2" build_name
  if [[ "${preset,,}" =~ ${RELEASE_REGEX} ]]; then
    build_name="$(cfg remoteBuildDirRelease "${preset}")"
    log "Preset '${preset}' is a release preset"
  else
    build_name="$(cfg remoteBuildDirDebug "${preset}")"
    log "Preset '${preset}' is a debug preset"
  fi
  log "Selected build: ${build_name}"
  remote_stop "${preset}"
  remote_mkdir "${preset}" || die "remote mkdir failed"
  remote_deploy "${build_name}" "${preset}" || die "deploy failed"
  local executable_dir="${DEPLOY_DIR}/$(dirname "${executable}")" command
  command="cd ${executable_dir}"
  [[ -n "${LD_PATH}" ]] && command+=" && export LD_LIBRARY_PATH=${LD_PATH}"
  command+=" && ${GDBSERVER} ${GDB_LISTEN}:${PORT} ${DEPLOY_DIR}/${executable}"
  log "Starting gdbserver for '${executable}' (blocks until the debug session ends)"
  select_transport "${preset}"
  run ssh "${SSH_ARGS[@]}" -t "${TARGET}" "${command}"
}

case "${ACTION}" in
  stop) remote_stop "${1-}" ;;
  mkdir) remote_mkdir "${1-}" ;;
  deploy)
    [[ $# -ge 1 ]] || die "deploy needs <build-name>"
    remote_deploy "$1" "${2-}" || die "deploy failed"
    ;;
  debug)
    [[ $# -ge 2 ]] || die "debug needs <preset> <exe>"
    remote_debug "$1" "$2"
    ;;
  *) die "unknown action '${ACTION}' (stop|mkdir|deploy|debug)" ;;
esac
