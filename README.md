# quickjs demos

## Init Repo

```sh
git submodule update --init
```

## Build quickjs

```sh
cd quickjs && make
```

## Demo01

Use Quickjs Run JS Code and Call JS Function In C.

```sh
cd demo01
make clean && make && ./main
```

## Demo02

Use Quickjs Run JS Code and Call C Function In JS.

```sh
cd demo02
make clean && make && ./main
```

## Demo03

Use Quickjs Run JS Code with Console API.

```sh
cd demo03
make clean && make && ./main
```


## Demo04

Use Quickjs Run JS Code with C Class Constructor.

```sh
cd demo04
make clean && make && ./main
```


## Demo05

Use Quickjs Run JS Module Code with import C Module Code.

```sh
cd demo05
make clean && make && ./main
```

## Demo06

Use QuickJS to compile JavaScript code to bytecode, then read and execute the bytecode.

```sh
cd demo06
make clean && make && ./main
```

## Demo07

Use QuickJS to implement a custom fetch implementation with async/await support.

```sh
cd demo07
make clean && make && ./main
```

## Demo08

Use QuickJS with a thread pool to benchmark JavaScript file execution performance. This demo creates a thread pool with multiple worker threads (based on CPU cores), each with its own QuickJS runtime, to execute JavaScript files in parallel.

```sh
cd demo08
make clean && make && ./main test1.js test2.js test3.js test4.js 100
```

## Demo09

Use QuickJS with `libuv` to implement an event loop with `setTimeout` and `Promise` support. This demo shows how to integrate QuickJS with `libuv` to handle asynchronous JavaScript operations including timers and microtasks.

```sh
cd demo09
make clean && make && make run
```

