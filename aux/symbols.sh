#!/bin/bash

set -e

declare -A glibc_vers;
declare -A glibcxx_vers;
declare -A cxxabi_vers;

for file in "$@"; do
  while IFS= read -r line; do
    glibc_vers[$line]=1
  done <<< "$(nm $file | awk -F'@GLIBC_' '$2 !~ /^$/ {print $2}')"
  echo -e "GLIBC versions (unique):\n${!glibc_vers[@]}"

#   while IFS= read -r line; do
#     glibcxx_vers[$line]=1
#   done <<< "$(nm $file | awk -F'@GLIBCXX_' '$2 !~ /^$/ {print $2}')"
#   echo -e "GLIBCXX versions (unique):\n${!glibcxx_vers[@]}"

  while IFS= read -r line; do
    cxxabi_vers[$line]=1
  done <<< "$(nm $file | awk -F'@CXXABI_' '$2 !~ /^$/ {print $2}')"
  echo -e "CXXABI versions (unique):\n${!cxxabi_vers[@]}"
done
