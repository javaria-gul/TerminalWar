#!/bin/bash

level3_pass="Acc3ssC0de_LeV3!2025"  # Level 3 ka password

echo "------------------------------"
echo "Welcome to Level 4!"
echo "To enter this level, please enter the password you found in Level 3."
echo "------------------------------"

read -p "Enter Level 3 password: " input_pass

if [[ "$input_pass" != "$level3_pass" ]]; then
    echo "❌ Incorrect password. Access Denied!"
    exit 1
fi

# Setup task area (only once)
if [ ! -d ".level4_secret" ]; then
    mkdir -p .level4_secret
    level4_pass="MysT3rY!LvL4#58x"

    # Save password for Game to carry forward
    echo "$level4_pass" > .password

    # Hide password in a tricky place
    mkdir -p .level4_secret/archives
    echo "Old logs" > .level4_secret/archives/log1.old
    echo "$level4_pass" > .level4_secret/archives/.secret_key.txt
    echo "Random doc" > notes.txt
    echo "Ignore me" > .hidden_notes.bak
    echo "Trash file" > .trash.txt
    echo "Confusing data" > .level4_secret/fake_info.log
fi

echo ""
echo "✅ Access Granted!"
echo "------------------------------"
echo "Task: The password to enter Level 5 is hidden in one of the files."
echo "Hint: Explore '.level4_secret/archives' directory — some files may be hidden!"
echo "You can now use commands like 'ls', 'cat', 'cd' etc."
echo "When you're done, exit the shell to go back to main game."
echo "------------------------------"

# User interactive shell
bash
