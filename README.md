# nanoanno

## What

This should be a program that helps with quick annotations in screenshots made with maim or something similar. Ideally, one would run a screenshot taking command such as `maim -s /tmp/screenshot.png` and then would run this program right after it with `nanoanno /tmp/screenshot.png ~/Pictures/screenshots/$(date).png`. In the program itself one can then easily do small annotations, delete the image, copy it to the clipboard and save it. That's the idea.

## Keybinds
|Keys|Action|
|---|---|
|h, j, k, l, holding the middle mouse button |moving the image|
|u, i, mouse scrolling |scaling the image|
|q |quit and save the image (currently to file.png)|
|c |copy the current image to the clipboard|
|x |undo all changes|

## Video

![](https://cdn.discordapp.com/attachments/833686255446917123/874025047524798525/output.gif)

## TODO
- [ ] make the second command line argument do its job
- [x] fix image dragging by holding the middle mouse button
- [ ] make the popup better looking
- [x] add clipboard support
- [ ] interpolate between two drawn dots to create a more smooth line
- [ ] eraser
- [ ] text tool

my first C project - don't be harsh
