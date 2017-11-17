# board-draw-ascii
Simple terminal-based text art tool, for sprucing up Curses-based games.

The end-goal is to work up to supporting Code Page 437 and Unicode drawing. For now, it is 7-bit ASCII only.

Work in progress -- just a demo at this point.


##### Controls

Move cursor: Arrow keys

Draw: Spacebar

Floodfill: f

Grab character and color: Enter

Erase character: Delete


Cycle foreground color: c

Cycle background color: v

Toggle bright foreground: D

Toggle blink: F (support varies by terminal / console)

Next character: \]

Prev character: \[


Toggle Sustained Draw (Doodle Mode): Tab


Set filename (default scratch.brd): @ (Shift + 2)

Load specified file: L

Save specified file: S


##### Building

###### Linux

gcc \*.c -o draw -lncurses

###### Windows

TODO
