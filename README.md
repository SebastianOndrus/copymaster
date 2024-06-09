# Copymaster

Copymaster is a command-line utility for copying files with advanced options. It provides functionalities such as copying, linking, truncating, setting permissions, and more.

## Features

- **Copy:** Copy files from a source to a destination.
- **Link:** Create hard links between files.
- **Truncate:** Reduce the size of a file to a specified size.
- **Permissions:** Set permissions for copied files.
- **Directory:** List contents of a directory.

## Prerequisites

- **C Compiler:** GCC (GNU Compiler Collection)
- **Operating System:** Unix-like system

## Usage

To use Copymaster, run the following command:

```bash
copymaster [OPTIONS] SOURCE DEST
```

Replace SOURCE with the path to the source file or directory and DEST with the path to the destination file or directory.

### Options

- `-f`, `--fast`: Copy files quickly.
- `-s`, `--slow`: Copy files slowly.
- `-c`, `--create`: Create a new file.
- `-o`, `--overwrite`: Overwrite existing files.
- `-a`, `--append`: Append to existing files.
- `-l`, `--lseek`: Seek to a specific position in the file.
- `-D`, `--directory`: Copy directory contents.
- `-d`, `--delete`: Delete the source file after copying.
- `-m`, `--chmod`: Set permissions for copied files.
- `-i`, `--inode`: Copy files based on inode number.
- `-u`, `--umask`: Set umask for copied files.
- `-K`, `--link`: Create hard links instead of copying.
- `-t`, `--truncate`: Truncate copied files to a specified size.
- `-S`, `--sparse`: Copy files sparsely.


## Build

To build the project, run the following commands:

```bash
make
```
## Clean Up

To clean up the project, run the following command:

```bash
make clean
```


