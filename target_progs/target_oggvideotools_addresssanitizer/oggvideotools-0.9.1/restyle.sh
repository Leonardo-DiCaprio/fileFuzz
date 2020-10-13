#!/bin/bash
astyle  --style=stroustrup -s2 --recursive *.cpp
astyle  --style=stroustrup -s2 --recursive *.h src/*.h
