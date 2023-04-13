# branch_predictor
A simple gshare branch predictor writting in Vlang. It is really fast than a C version!

## What is this ?
Branch predictor is one of the most import circuit in modern super scalar CPUs. By given a PC, the predictor should provide a suggestion of Taken or Not Taken.

This project provide a branch predictor simualtion framework, which is written in vlang. To demo, it provide a simple gshare implementation of branch predictor.

Also provide some branch instruction traces, which is converted from [CBP-16](https://jilp.org/cbp2016/framework.html) traces.

## How to run ?
```bash
You must install zstd in your system:

  apt install libzstd-dev

The project use the [cbsl](https://github.com/kbkpbot/cbsl), which need the zstd compress library to work.

  $ git clone https://github.com/kbkpbot/branch_predictor.git

  $ cd branch_predictor
  $ git submodule update --init --recursive
  $ v -prod .
  $ ./branch_predictor
```
