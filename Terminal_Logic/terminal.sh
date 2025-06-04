#!/usr/bin/env bash

BASE_DIR="$(cd "$(dirname "$0")" && pwd)"
ROOT="$BASE_DIR/Root"
LEVEL="Level-1"
LEVEL_PATH="$ROOT/$LEVEL"

cd "$LEVEL_PATH" || { echo "Level directory not found!"; exit 1; }

RESTRICTED_CMDS=("pwd" "rm" "touch" "mkdir" "nano" "vim" "gedit" "code")

get_prompt_path() {
  local path=$(pwd)
  echo "${path#$BASE_DIR/}"
}

is_restricted() {
  for rcmd in "${RESTRICTED_CMDS[@]}"; do
    [[ "$1" == "$rcmd" ]] && return 0
  done
  return 1
}

manual_ls() {
  # Only show file.txt if it exists
  if [[ -f "file.txt" ]]; then
    echo "file.txt"
  else
    echo ""
  fi
}

manual_cat() {
  local file="$1"
  if [[ "$file" == "file.txt" && -f "$file" ]]; then
    cat "$file"
  else
    echo "cat: $file: No such file"
  fi
}

while true; do
  REL_PATH=$(get_prompt_path)
  read -e -p "user@TERMINALWar:$REL_PATH$ " CMD_LINE

  [[ -z "$CMD_LINE" ]] && continue

  CMD=$(echo "$CMD_LINE" | awk '{print $1}')
  ARGS=$(echo "$CMD_LINE" | cut -d' ' -f2-)

  if is_restricted "$CMD"; then
    echo "Error: '$CMD' is restricted in this terminal."
    continue
  fi

  [[ "$CMD" == "exit" ]] && echo "Exiting terminal..." && break

  case "$CMD" in
    cd)
      TARGET="$ARGS"
      [[ -z "$TARGET" ]] && TARGET="$HOME"
      NEW_PATH="$(realpath -m "$PWD/$TARGET")"
      if [[ "$NEW_PATH" == "$LEVEL_PATH"* ]]; then
        cd "$TARGET" 2>/dev/null || echo "No such directory: $TARGET"
      else
        echo "Access denied: Cannot leave level directory."
      fi
      ;;
    ls)
      manual_ls
      ;;
    cat)
      manual_cat "$ARGS"
      ;;
    echo)
      echo "$ARGS"
      ;;
    help)
      echo "Available commands: cd, ls, cat, echo, help, exit"
      echo "Restricted commands: ${RESTRICTED_CMDS[*]}"
      ;;
    clear)
      clear
      ;;
    *)
      echo "command not found: $CMD"
      ;;
  esac
done

