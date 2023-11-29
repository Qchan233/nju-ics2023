#!/bin/bash

for i in 0 1 2 3 4 5
do
   convert "slides.pdf[$i]" -sharpen "0x1.0" -type truecolor -resize 400x300\! "slides-$i.bmp"
done

mkdir -p $NAVY_HOME/fsimg/share/slides/
rm $NAVY_HOME/fsimg/share/slides/*
mv *.bmp $NAVY_HOME/fsimg/share/slides/
