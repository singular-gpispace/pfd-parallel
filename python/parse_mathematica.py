#!/usr/bin/python3

import sys
import os
from math_parser import *

if len(sys.argv) <= 1:
  print("please give input file path as argument")
  exit()

filename = sys.argv[1]

if not os.path.isfile(filename):
  print("please give a valid input file path as argument")
  exit()


row = 0

with open(filename, "r") as f:
  c = f.read(1);
  c = parse_blank(c, f)

  if not c == "{":
    print("incorrect file format, expected '{', got '" + c + "'")
    exit()
  c = f.read(1);
  c = parse_blank(c, f)

  #dbg_on()

  while c == "f":
    row += 1
    c = f.read(1); # skip 'f'
    c = f.read(1); # skip '['
    index, c = parse_number(c, f)
    print (index)
    if not str(row) == str(index):
      print("wrong expected index, expected " + str(row) + ", got" + str(index))
      exit()
    c = f.read(1); # skip ']'
    c = parse_blank(c, f)
    c = f.read(1); # skip '-'
    c = f.read(1); # skip '>'
    c = parse_blank(c, f)

    express, c = parse_expression(c, f)
    col = 0
    for term in express._terms:
      col += 1
      print(row, col)

      with open("/m/scratch/hive/heymann/pfd/data/coeffs_finiteR/txt/coeffs_fin_" +
                          str(row) + "_" + str(col) + ".txt", "w") as f_entry:
        f_entry.write(str(term) + "\n")
      with open("coeffs_finR_indices.txt", "a") as f_indices:
        f_indices.write(str(row) + ", " + str(col) +  "\n")


    c = f.read(1); # skip ','
    c = parse_blank(c, f) # skip to 'f' (or end)

if not c == "}":
  print (c)
