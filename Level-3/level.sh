#!/bin/bash

level2_pass="R@reP@ss_LeV2!91x"  # Level 2 ka password

echo "------------------------------"
echo "Welcome to Level 3!"
echo "To enter this level, please enter the password you found in Level 2."
echo "------------------------------"

read -p "Enter Level 2 password: " input_pass

if [[ "$input_pass" != "$level2_pass" ]]; then
    echo "❌ Incorrect password. Access Denied!"
    exit 1
fi

# Setup task area (only once)
if [ ! -d ".level3_secret" ]; then
    mkdir -p .level3_secret
    level3_pass="Acc3ssC0de_LeV3!2025"

    # Save password for Game to carry forward
    echo "$level3_pass" > .password

    # Hide actual password in confusing place
    mkdir -p .level3_secret/logs
    echo "Log start" > .level3_secret/logs/system.log
    echo "$level3_pass" > .level3_secret/logs/.hidden_access_code
    echo "Nothing here" > notes.txt
    echo "Ignore this file" > .hidden_notes.bak
    echo "Trash data" > .trash.txt
    echo "Dummy file" > .level3_secret/fake_data.txt
    echo "Trace info" > .level3_secret/logs/trace.dump
fi

echo ""
echo "✅ Access Granted!"
echo "------------------------------------------------------------------------------------"
echo "Task: The password to enter Level 4 is hidden in one of the files."
echo "Hint: Use commands like 'ls -a', 'cat', 'grep' to find the hidden password in level 3"
echo "When you're done, type 'exit' to leave the shell and return to the main game."
echo "-------------------------------------------------------------------------------------"

# User interactive shell
bash



