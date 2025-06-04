#!/bin/bash

clear
echo "==========================="
echo "🔓 Welcome to Terminal War"
echo "==========================="

# Carry password forward between levels
pass=""

# Function to run level
run_level() {
    level=$1
    echo "---------------------------"
    echo "Starting Level $level..."
    echo "---------------------------"
    cd "Level-$level" || { echo "Level-$level not found!"; exit 1; }

    if [[ "$level" -eq 1 ]]; then
        ./level.sh
    else
        ./level.sh "$pass"
    fi

    # After level complete, get password for next
    if [[ -f ".password" ]]; then
        pass=$(cat .password)
    else
        echo "❌ Password file missing in Level $level."
        exit 1
    fi

    cd ..
}

# Sequentially run levels
for i in {1..5}
do
    run_level $i
done

echo ""
echo "🎉 Congratulations! You have completed all levels!"

