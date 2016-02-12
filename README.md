# Autovivification of Javascript objectds

[![Build Status](https://travis-ci.org/allenluce/node-autovivify.svg?branch=master)](https://travis-ci.org/allenluce/node-autovivify)

### Installation

    npm install autovivify

## Usage

```javascript
const Av = require('autovivify')

const obj = new Av()

obj.subobj[4].subarray[2].neat = 'coolio'
assert(obj.subobj[4].subarray[2].neat === 'coolio')

```

## Limitations

Symbols are not supported as properties.

Under the covers, the module fakes out accesses to the object and
never returns anything as undefined.  This means that you cannot check
to see if part of the structure is undefined, as it will never be.
Use membership or length checks to tell if something is empty as
necessary.
