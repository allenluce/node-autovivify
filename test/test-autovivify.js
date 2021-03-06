/* global describe it beforeEach */
const expect = require('chai').expect
const Av = require('..')
const eql = require('./equal')

describe('Autovivify', function () {
  beforeEach(function () {
    this.obj = new Av()
  })

  it('must be called as a constructor', function () {
    expect(function () {
      const obj = Av()
      expect(obj).to.not.exist
    }).to.throw(/Function must be called as a constructor./)
  })

  it('creates new objects on the fly', function () {
    this.obj.something.deep = 'hey'
    expect(this.obj.something.deep).to.equal('hey')
    expect(eql(this.obj, {something: {deep: 'hey'}})).to.be.true
  })

  it('does not support symbols', function () {
    if (Number(process.version.match(/^v(\d+\.\d+)/)[1]) > 0.12) {
      var sym = Symbol('deep')
      var self = this
      expect(function () {
        self.obj.something[sym] = 'hey'
      }).to.throw(/Symbol properties are not supported/)
    }
  })

  it('creates new arrays on the fly', function () {
    this.obj.ary[4] = 12
    expect(this.obj.ary[4]).to.equal(12)
    var normal_array = []
    normal_array[4] = 12
    expect(eql(this.obj.ary, normal_array)).to.be.true
  })

  it('creates arrays and objects as necessary', function () {
    this.obj.subobj[4].subarray[2].neat = 'coolio'
    expect(this.obj.subobj[4].subarray[2].neat).to.equal('coolio')
  })

  it('can delete from arrays just fine', function () {
    this.obj.subobj[22] = 'some value'
    delete this.obj.subobj[22]
    expect(eql(this.obj.subobj[22], {})).to.be.true
  })

  it('can delete from objects just fine', function () {
    this.obj.subobj.another = 'any value'
    expect(eql(this.obj.subobj, {})).to.be.false
    delete this.obj.subobj.another
    expect(eql(this.obj.subobj, {})).to.be.true
    expect(eql(this.obj.subobj.another, {})).to.be.true
    // But because of autovivification...
    expect(eql(this.obj.subobj, {})).to.be.false
  })

  it('gives the proper length of a string', function () {
    this.obj.s = "sowofhiudf"
    expect(this.obj.s.length).to.equal(10)
  })

  it('object prototypes compare the same', function () {
    const obj1 = new Av()
    const obj2 = new Av()
    const proto1 = Object.getPrototypeOf(obj1)
    const proto2 = Object.getPrototypeOf(obj2)
    expect(proto1).to.equal(proto1)
    expect(proto1).to.equal(proto2)
  })

  it('has a working toString() method', function () {
    expect(this.obj.toString()).to.equal('[object AutoVivify]')
  })

  it('Object.prototype.toString.call() returns an appropriate representation', function () {
    expect(Object.prototype.toString.call(this.obj)).to.equal('[object AutoVivify]')
  })
})
