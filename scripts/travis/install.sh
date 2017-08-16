#!/bin/bash

if [[ $TRAVIS_OS_NAME != 'osx' ]]; then
    pip install flake8
fi
