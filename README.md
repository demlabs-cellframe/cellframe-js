# cellframe-js
Current dependencies:
```
$ sudo apt-get install nodejs npm cmake libev-dev libmagic-dev libjson-c-dev libsqlite3-dev libcurl4-openssl-dev
$ npm install -g cmake-js
```

I recommend to use nvm to manage node versions: https://github.com/nvm-sh/nvm

To build sdk you need to go to `packages/cellframe-sdk` and run
```
cmake-js build --debug
```
