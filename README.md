# branch_predictor
A simple gshare branch predictor writting in Vlang. It is really fast than a C version!

## What is this ?
Branch predictor is one of the most import circuit in modern super scalar CPUs. By given a PC, the predictor should provide a suggestion of Taken or Not Taken.

This project provide a branch predictor simualtion framework, which is written in vlang. To demo, it provide a simple gshare implementation of branch predictor.

Also provide some branch instruction traces, which is converted from [CBP-16](https://jilp.org/cbp2016/framework.html) traces.

## How to run ?
You must install zstd in your system:
```bash
  apt install libzstd-dev
```

The project use the [cbsl](https://github.com/kbkpbot/cbsl), which need the zstd compress library to work.

```bash
  $ git clone https://github.com/kbkpbot/branch_predictor.git
  $ cd branch_predictor
  $ git submodule update --init --recursive
  $ v -prod .
  $ ./branch_predictor
```

You should get result something like this:
```bash
====================Summary====================
traces/LONG_MOBILE-1            = 0.556903
traces/LONG_MOBILE-2            = 1.388763
traces/LONG_MOBILE-3            = 7.812431
traces/LONG_MOBILE-4            = 0.006016
traces/SHORT_MOBILE-1           = 2.933743
traces/SHORT_MOBILE-2           = 11.394828
traces/SHORT_MOBILE-24          = 0.000489
traces/SHORT_MOBILE-25          = 0.001764
traces/SHORT_MOBILE-27          = 0.480042
traces/SHORT_MOBILE-28          = 0.007541
traces/SHORT_MOBILE-3           = 3.752000
traces/SHORT_MOBILE-30          = 6.294661
traces/SHORT_MOBILE-4           = 5.163341
===============================================
Total MISPRED_PER_1K_INST               = 39.8

Score = 86(39.8) time cost = 2.94 seconds
```

MISPRED_PER_1K_INST mean Miss Predictions Per 1000 Instructions. 

## How to writing my own predictor?
Just modify the predictor.v. You target is try to min the MISPRED_PER_1K_INST.

