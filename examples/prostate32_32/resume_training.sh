#!/usr/bin/env sh

../../build/tools/caffe train \
    --solver=infant_solver.prototxt \
    --snapshot=infant_fcn_iter_130000.solverstate
