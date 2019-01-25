#!/bin/bash

glslangValidator -V triangle.vert -o ../build/bin/triangle.vert.spv &&
glslangValidator -V triangle.frag -o ../build/bin/triangle.frag.spv