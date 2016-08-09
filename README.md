# Autovivification of Javascript objects

[![Build Status](https://travis-ci.org/allenluce/node-autovivify.svg?branch=master)](https://travis-ci.org/allenluce/node-autovivify)

Does this ever happen to you?

```
> cars = {}
{}
> cars.mercedes.sl600.color = 'blue'
TypeError: Cannot read property 'sl600' of undefined
```

Well now there's *node-autovivify*!

```
> Av = require('autovivify')
> cars = new Av()
{}
> cars.mercedes.sl600.color = 'blue'
'blue'
```

It also works with arrays!

``` javascript
Av = require('autovivify')
> cars = new Av()
> cars.mercedes.sl600.color = 'blue'
'blue'
> cars.carlist[0] = cars.mercedes
{ sl600: { color: 'blue' } }
```

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

You cannot use undefined checks with this structure as the very act of
checking ensures that the structure is defined. Use membership or
length checks to determine if something is empty.
