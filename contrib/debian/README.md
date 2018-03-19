
Debian
====================
This directory contains files used to package lusod/luso-qt
for Debian-based Linux systems. If you compile lusod/luso-qt yourself, there are some useful files here.

## luso: URI support ##


luso-qt.desktop  (Gnome / Open Desktop)
To install:

	sudo desktop-file-install luso-qt.desktop
	sudo update-desktop-database

If you build yourself, you will either need to modify the paths in
the .desktop file or copy or symlink your luso-qt binary to `/usr/bin`
and the `../../share/pixmaps/luso128.png` to `/usr/share/pixmaps`

luso-qt.protocol (KDE)

