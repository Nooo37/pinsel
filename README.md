# pinsel

## What

This is a program that helps with quick small annotations in screenshots made with maim or something similar. Everything is in progress currently.

## Usage

You can open a file by passing it as the first argument: `pinsel path/to/file.png` or by piping into the program. That's why `pinsel file.png` and `cat file.png | pinsel` behave the same. If there is both an argument and an image being piped into the program, piping will be prefered. 

You can pre-define the destination of a saved file with the `-o` flag. The save shortcuts will then just save the image to the destination saving you from the tiring file dialog (especially in day-to-day usage).

An example usage for screenshot taking may look like the following:

```sh
maim -s | pinsel -o $HOME/$(date).png | xclip -selection clipboard -t image/png
```

First you select your screenshot through maim like you are used to. The result gets piped into pinsel where you can do annoatations and where you are free to save the image to `~/$(date).png` or not. If you don't want to annotate your image then you can still decide on whether to save it or not (which may add one key press in total to that workflow in the "worst case"). The end result gets piped to your clipboard for further usage.

### Preview

![](https://user-images.githubusercontent.com/70270606/132033128-792f54ae-90fb-4471-98b8-5a258d56d474.gif)


### Flags

- `-o`, `--output` predefines the save-destination
- `-f`, `--format` defines the image format of the stdout output
- `--ontop` puts the window above all others on startup
- `--maximize` maximizes the window on startup
- `-h`, `--help`, `-v`, `--version` wip

### Configuration

Pinsel provides a lua configuration. You can create your own config by placing a `init.lua` file in `$XDG_CONFIG_HOME/pinsel/init.lua` or `$HOME/.config/pinsel/init.lua`. For reference: The default config can be found in `data/init.lua`. Since the lua configuration handles a lot of things (most notably keybindings and mousebindings), it might be a good idea to start out by modifying the default config.

The lua configuration is very flexible. With the right configuration, I can for example play pong in pinsel:

![](https://user-images.githubusercontent.com/70270606/132034355-63b656d1-75ff-49dc-ba12-bf8b333c5ef6.gif)


### Shortcuts

Open the shortcut dialog to look them up 👍

## Installation

```shell
git clone https://github.com/Nooo37/pinsel.git
cd pinsel
sudo make install # will put the binary in /usr/local/bin
```

## TODO
- [x] allow for all font faces
- [ ] add `--help` text, man page
- [ ] allow for other formats than png
- [x] make text actually draggable when the dialog is open
- [ ] add a titlebar to the text dialog
- [ ] release


my first C project - don't be harsh
