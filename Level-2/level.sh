#!/bin/bash

level1_pass="P@s\$w0rD!_LvL1#29x"  # same as Level 1's password

echo "------------------------------"
echo "Welcome to Level 2!"
echo "To enter this level, please enter the password you found in Level 1."
echo "------------------------------"

read -p "Enter Level 1 password: " input_pass

if [[ "$input_pass" != "$level1_pass" ]]; then
    echo "❌ Incorrect password. Access Denied!"
    exit 1
fi

# Setup task area (only once)
if [ ! -d ".level2_secret" ]; then
    mkdir -p .level2_secret
    level2_pass="R@reP@ss_LeV2!91x"

    # Save password for Game to carry forward
    echo "$level2_pass" > .password

    # Hide actual password in confusing place
    mkdir -p .level2_secret/logs
    echo "Temporary cache" > .level2_secret/logs/syslog1.tmp
    echo "$level2_pass" > .level2_secret/logs/.core_hidden
    echo "Random notes" > notes.txt
    echo "Ignore this" > .hidden_notes.bak
    echo "Not useful" > .trash.txt
    echo "Still nothing" > .level2_secret/fake_data.txt
    echo "Confusing logs" > .level2_secret/logs/trace.dump
fi

echo ""
echo "✅ Access Granted!"
echo "------------------------------"
echo "Task: The password to enter Level 3 is hidden in one of the files."
echo "Hint: Explore '.level2_secret/logs' directory — some files may be hidden!"
echo "You can now use commands like 'ls', 'cat', 'cd' etc."
echo "When you're done, exit the shell to go back to main game."
echo "------------------------------"

# User interactive shell
bash

