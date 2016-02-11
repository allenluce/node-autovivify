// Mostly cribbed from https://github.com/chaijs/deep-eql

function isValue (a) {
  return a !== null && a !== undefined
}

function iterableEqual (a, b) {
  if (a.length !== b.length) {
    return false
  }

  for (var i = 0; i < a.length; i++) {
    if (a[i] !== b[i]) {
      return false
    }
  }

  return true
}

function objectEqual (a, b, m) {
  if (!isValue(a) || !isValue(b)) {
    return false
  }

  if (m) {
    for (var i = 0; i < m.length; i++) {
      if ((m[i][0] === a && m[i][1] === b)
          || (m[i][0] === b && m[i][1] === a)) {
        return true
      }
    }
  } else {
    m = []
  }

  try {
    var ka = Object.keys(a)
    var kb = Object.keys(b)
  } catch (ex) {
    if (typeof(a) == typeof(b)) {
      return true
    }

    return false
  }

  ka.sort()
  kb.sort()

  if (!iterableEqual(ka, kb)) {
    return false
  }

  m.push([ a, b ])

  for (i = ka.length - 1; i >= 0; i--) {
    var key = ka[i]
    if (!objectEqual(a[key], b[key], m)) {
      return false
    }
  }

  return true
}

module.exports = objectEqual

