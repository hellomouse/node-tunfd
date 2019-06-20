# node-tunfd

It makes tun/tap interfaces.

## How to use

```js
const tunfd = require('tunfd');
const fs = require('fs');

let iface = new tunfd.TunInterface({
  // optional, kernel will automatically assign a name if not given here
  name: 'tun0',
  // can be either "tun" or "tap", default is "tun"
  // tun mode gets you ip packets, tap mode gets you ethernet frames
  mode: 'tun',
  // set to true if you want the 4-byte packet information header
  // default is false, which adds IFF_NO_PI to ifr_flags
  pi: false
});

// if you want to know the auto-assigned name of the interface
console.log(iface.name);
// the fd of the new interface
console.log(iface.fd);

// how to get packets
let readStream = fs.createReadStream(null, { fd: iface.fd });
readStream.on('data', packet => { ... });
// how to put packets
let writeStream = fs.createWriteStream(null, { fd: iface.fd });
writeStream.write(...);

// fork()
console.log(tunfd.fork());
```

## License

ISC
