# Development tasks

## c-test

> Run test of C.

```sh
set -ex

cargo r -- --language=c lib/c/cfgopt_gen.h

cd lib/c/
make
./cfgopt_test
```
