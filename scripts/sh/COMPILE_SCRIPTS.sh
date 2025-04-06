#!/bin/bash

directory="scripts/src"  # Change this to the actual directory name
command="make compile-script"      # Replace with the actual command

# Check if the directory exists
if [ ! -d "$directory" ]; then
    echo "scripts directory '$directory' not found."
    exit 1
fi

# Iterate over each file inside the directory
for file in "$directory"/*; do
    if [ -f "$file" ]; then  # Ensure it's a file, not a subdirectory
        echo "[scripts/sh/COMPILE_SCRIPTS.sh] Compiling Script: $command filename=$file"
        $command filename="$file"
        echo " "
        echo "compilation for $file complete"
        echo " "
    fi
done

# Iterate over each .dylib file inside the directory
for file in "$directory"/*.dylib; do
    if [ -f "$file" ]; then
        echo "[scripts/sh/COMPILE_SCRIPTS.sh] Placing Script: mv '$file' '../$(basename "$file").dylib'"
        mv "$file" "scripts/"
        echo " "
        echo "script $(basename "$file") placed / job complete"
        echo " "
    fi
done

echo [scripts/sh/COMPILE_SCRIPTS.sh] script compilation complete!
echo
