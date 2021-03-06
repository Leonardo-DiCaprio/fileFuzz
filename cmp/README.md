Account Info
============
git repo: https://git.dev.tencent.com/cgnail/muFuzzer.git
username: cgnail
password: shiyin

Files & Directories
===================

1. `target_progs/` : all the fuzzing targets, and their source codes, such as libpng/
2. `keywords/` : keyword list for each file type
3. `010templates/`: template files from 010Editor
4. `utils/`: all the scripts needed by `monitor-outputs.sh` and others

Setup
=====

1. Download the Ubuntu 14.04 i386 box from https://mirror.tuna.tsinghua.edu.cn/ubuntu-cloud-images/vagrant/trusty/, then create your developing directory by typing 

```
  vagrant box add --name <your_box_name> <path_to_the_downloaded_box>
  mkdir <your_dev_dir> && cd <your_dev_dir>

  # Git needs an empty directory to clone the repo, and DO NOT miss the "."
  git clone <git_repo_link> .  

  vagrant init <your_box_name>
  vagrant up
  vagrant ssh
```

2. Install the necessary packages by 
````
  sudo apt-get update
  sudo apt-get install build-essential git inotify-tools python-pip
  sudo pip install pfp
````

3. Create directory for the output of fuzzing and for the monitor to watch
````
  mkdir -p fuzz_out/queue
````
in which `fuzz_out/` is the directory specified by the `-o` parameter of `afl-fuzz`, and `fuzz_out/queue/` is the monitor's interest.

Compiling tools
=================

1. extract `pin-2.14.tar.gz` from `_tarball/` to `dtracker/support/`
2. enter `dtracker/` directory, create a link to `support/pin-2.14`, namely `pin/`, and set the `PIN_ROOT` environment variable before the compilation. E.g.:
```
  export PIN_ROOT=$(pwd)/pin
```
3. compile DataTracker
````
  make support-libdft
  make
````
4. compile (original) afl, and install it to the system path:
````
  cd ../afl-2.52b
  make
  sudo make install
````
5. compile modified afl(from patches, which can be generated by `git diff 431aa23 afl-2.52b`)

Compiling fuzzing targets(i.e. libpng/readpng)
==============================================

````
  cd ../target_progs/
  ln -s libpng-1.6.34/ libpng
  cd libpng/
  patch -p0 < ../../afl-2.52b/experimental/libpng_no_checksum/libpng-nocrc.patch
  CC=afl-gcc ./configure --disable-shared
  make
  cd ..
  make
````

Preparing necessary JSON mappings
=================================
Modified `afl-fuzz` needs some mapping files to guide the input mutation, including:

1. `<FUZZ_TARGET>_loc2tuple.json`: Mapping tuple hash(the array index in trace_bits[]) to an address tuple, denoting the starting addresses of the two BBLs in the tuple 
2. `<FUZZ_TARGET>_hash2bb.json`: Mapping the random number instrumented by AFL to its BBL's starting address
3. `<FUZZ_TARGET>_loc2cmp.json`: Mapping the tuple hash to the addresses of CMP instruments in the tuple(except that in the end BBL)

Generate these files under an Ubuntu 18.04 64bits system by typing:
````
  cd <git_repo_root>
  python ./utils_x64/trans-bb-addr.py ./target_progs/<FUZZ_TARGET> 
````

Test command lines
==================
````
  afl-2.52b/read-json-v2 readpng-file-input_angr.json fuzz_out/fuzz_bitmap fuzz_out/id-00.trace.bits fuzz_out/queue_ins2pos/id\:000004\,src\:000000\,op\:flip1\,pos\:0\,+cov_ins2pos.json readpng-loc2cmp_insn_addr.json
````

./afl-fuzz -i testcases/images/png/ -o ../fuzz_out -- ../target_progs/readpng-file-input @@
 rm -r ./fuzz_out && mkdir ./fuzz_out && ./monitor-outputs.sh -p ./target_progs/readpng-file-input -t 010templates/PNG.bt -k keywords/keywords_png.txt fuzz_out/



```
./afl-fuzz -i testcases/images/png/ -o ../fuzz_out -- ../target_progs/target_tests/readpng-file-input @@

rm -r ./fuzz_out && mkdir ./fuzz_out && ./monitor-outputs.sh -p ./target_progs/target_tests/readpng-file-input -t 010templates/PNG.bt -k keywords/keywords_png.txt fuzz_out/
```

