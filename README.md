# Custom Shell Project

A collection of shell implementations in C++, including a CLI shell with advanced features and a GUI shell using Qt.

**Author:** Komari Venkata Ramana

## Features

### CLI Shell (shell.cpp)
- Command parsing with support for pipes (`|`), input/output redirection (`<`, `>`, `>>`), and background execution (`&`)
- Built-in commands: `cd`, `pwd`, `exit`
- Command chaining with `;`
- Quoting support for arguments

### GUI Shell (gui_shell)
- Graphical user interface built with Qt6
- Executes commands via bash
- Built-in commands: `clear`, `exit`
- Real-time output display

### Hello World (hello..cpp)
- Simple C++ program demonstrating class usage

## Requirements

### For CLI Shell
- GCC or Clang
- Linux/Unix environment

### For GUI Shell
- Qt6 (install with `sudo pacman -S qt6-base qt6-tools` on Arch Linux)
- GCC

## Building

### CLI Shell
```bash
g++ shell.cpp -o shell
./shell
```

### GUI Shell
```bash
qmake6 gui_shell.pro
make
./gui_shell
```

## Usage

### CLI Shell
Run `./shell` and enter commands like:
- `ls | grep txt`
- `echo "Hello" > file.txt`
- `cd /tmp; pwd`

### GUI Shell
Run `./gui_shell` to open the GUI. Enter commands in the input field.

## Contributing

Feel free to fork and contribute improvements!

## License

MIT License