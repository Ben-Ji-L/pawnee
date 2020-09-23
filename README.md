# MyAwesomeServ [![Build Status](https://travis-ci.org/Benji59/myawesomeserv.svg?branch=master)](https://travis-ci.org/Benji59/myawesomeserv)

## Overview

This project was at the beginning a college project, this is a webserver build in C only with 
systems calls.

## Licence 

This project is a free software (GPL-3.0 License)

## Installation

For building and test the server run the following commands:

`make`

`./webserver/pawnee`

or more simple

`make run`

## Documentation

Documentation build with [Doxygen](https://www.doxygen.nl/index.html).
In doc directory you have documentation in html or latex.
If you want to rebuild html doc run:
 
 `make doc`

to rebuild latex doc run:
 
 `make doc-latex`

For this commands you need to have Doxygen install on your system and latex build need latex and some 
extra latex packages.

### HTML Doc

HTML doc are in `doc/html/` directory, for viewing it open `doc/html/index.html` file in your browser.

### Latex Doc

Latex doc are in `doc/latex/` directory, the compiled pdf is in `doc/latex/refman.pdf`
