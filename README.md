# Autovivification of Javascript objects

[![Build Status](https://travis-ci.org/allenluce/node-autovivify.svg?branch=master)](https://travis-ci.org/allenluce/node-autovivify)

Are you tired of having to type things like this?

``` javascript
let subarray = []
subarray[2] = {
  myvalue: 'coolio'
}

let subobj = []
subobj[4] = {
  subarray: subarray
}

const obj = {
  subobj: subobj
}

```

Wouldn't you rather just do this?

``` javascript
obj.subobj[4].subarray[2].myvalue = 'coolio'
```

Well, now you can!

## Installation

    npm install autovivify

## Usage

``` javascript
const Av = require('autovivify')

const obj = new Av()

obj.subobj[4].subarray[2].myvalue = 'coolio'
assert(obj.subobj[4].subarray[2].myvalue === 'coolio')

```

## Limitations

Symbols are not supported as properties.

You cannot check to see if part of the structure is undefined, as the
very act of checking ensures that the structure is defined. Use
membership or length checks to tell if something is empty.
