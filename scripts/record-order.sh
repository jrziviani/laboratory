#!/bin/bash

# ---
# My car audio player plays the songs in the order returned by find and, in a VFAT,
# it's the order the files were copied into the filesystem.
# There are utilities to reorder by changing the allocation table but I don't like
# these kind of tools so I wrote this simple script that, given a source folder and
# the destination folder, it will copy in the order desired.
#
# Note: the song name *must* be in the desired order OR pass arguments to the
#       sort command below.
#
# Example:
#
# folder/artist/01-song.mp3
# folder/artist/02-song.mp3
# folder/artist/03-song.mp3
#
# ./record-order.sh folder /mnt
SOURCE="$1"
DEST="$2"

#---
# Prints the program usage and quits
#---
usage() {

    printf "Usage: %s source dest\n" $(basename $0)
    exit 1
}

#---
# Creates the same directory tree structure in the destination
#---
create_directories() {

    printf "Creating directories..."
    find . -type d -exec echo '{}' \; | grep -v "^\.$" | sed "s@^\.\(.*\)\$@mkdir -p \"$DEST\1\"@g" | sh -x
}

#---
# Copies all files from source to dest and flushes the buffers
# Note: It's a mp3 "hardcoded" because I only use this format but it
#       is easy to change
#---
copy_files() {

    printf "Copying files..."
    find . -type f -exec echo '{}' \; | sort | sed "s@^\./\(\(.*\)/.*\.mp3\)@cp \"\1\" \"$DEST/\2\"@" | sh -x

    printf "Synchronizing...\n"
    sync
}

#---
# Entry point
#---
main() {

    # source and dest must be directories
    [ ! -d "$SOURCE" ] && usage
    [ ! -d "$DEST" ] && usage

    pushd "$SOURCE" > /dev/null
    create_directories
    copy_files
    popd > /dev/null
}

main
