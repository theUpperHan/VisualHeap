# Visheap

## Compile Modified GLIBC
Go to `hanglibc` directory, run the following commands:
```bash
make; make install
```

## Compile Shared Library

Go to `tests` directory
```
cd tests
```
and run the following

```bash
gcc -fPIC -shared -o mylib.so mylib.c -I/home/uprhan/hanglibc/install/include -L/home/uprhan/hanglibc/install/lib -Wl,-rpath,/home/uprhan/hanglibc/install/lib -Wl,--dynamic-linker=/home/uprhan/hanglibc/install/lib/ld-linux-x86-64.so.2
```


## Preload Shared Library with programs
```bash
LD_PRELOAD=./mylib.so LD_LIBRARY_PATH=/home/uprhan/hanglibc/install/lib:$LD_LIBRARY_PATH $EXECUTABLE
```

Make sure you change the file path where you want to store all the heap info file in `mylib.c`, this path will be used by the server to monitor file modifications.

## Run Server
```bash
cd server
node index.js
```
Make sure you change the `folderToWatch` variable to the folder where `mylib.so` will save all the files

## Run Client
```bash
cd visheap
npm run start
```