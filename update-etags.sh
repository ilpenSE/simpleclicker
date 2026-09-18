#!/bin/bash
set -xe

etags $(find . -type f \( -name "*.cpp" -o -name "*.hpp" -o -name "*.h" \))
