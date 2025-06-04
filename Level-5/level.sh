#!/bin/bash

level4_pass="MysT3rY!LvL4#58x"  # Level 4 ka password

echo "------------------------------"
echo "Welcome to Level 5!"
echo "To enter this level, please enter the password you found in Level 4."
echo "------------------------------"

read -p "Enter Level 4 password: " input_pass

if [[ "$input_pass" != "$level4_pass" ]]; then
    echo "❌ Incorrect password. Access Denied!"
    exit 1
fi

# Setup task area (only once)
if [ ! -d ".level5_secret" ]; then
    mkdir -p .level5_secret

    level5_pass="Z1pExTr4ct_LvL5#73x"

    # Save password for Game to carry forward
    echo "$level5_pass" > .password

    # Create a hidden zip file containing the password
    echo "$level5_pass" > password.txt
    zip -q .level5_secret/hidden_pass.zip password.txt
    rm password.txt

    # Add some confusing files
    echo "Some junk data" > .level5_secret/random.log
    echo "Nothing here" > notes.txt
    echo "Ignore this file" > .hidden_notes.bak
    echo "Trash file" > .trash.txt
fi

echo ""
echo "✅ Access Granted!"
echo "------------------------------"
echo "Task: The password to enter Level 6 is hidden inside a zip file."
echo "Hint: Use commands like 'unzip' to extract files."
echo "Explore the '.level5_secret' directory."
echo "When you're done, exit the shell to go back to main game."
echo "------------------------------"

# User interactive shell
bash

