#!/bin/bash

echo "------------------------------"
echo "Welcome to Level 1!"
echo "Task: Find the password hidden in a hidden file."
echo "Hint: Use 'ls -a' and 'cat'. Be careful — some files are just there to confuse you!"
echo "------------------------------"

# Setup - only if folder doesn't exist
if [ ! -d ".secret_folder" ]; then
    mkdir -p .secret_folder

    # Complex password
    complex_pass="P@s\$w0rD!_LvL1#29x"

    # Save in hidden actual file (user has to find this)
    echo "$complex_pass" > .secret_folder/.trash_notes.txt

    # Create misleading files
    echo "not here" > dummy1.log
    echo "try again" > dummy2.bak
    echo "nope" > .hidden1.tmp
    echo "keep looking" > .hidden2.backup
    echo "nothing here" > .secret_folder/notes.txt
    echo "waste file" > .secret_folder/random_file.doc
fi

echo ""
echo "You can now explore the directory freely."
echo "Type commands to check files and find the password."
echo "When you are done, type 'exit' to finish Level 1."
echo ""

# Start interactive shell so user can run commands
bash

echo ""
echo "Level 1 finished. Remember your password for Level 2!"
echo ""

exit 0

