// console.log('==== test1.js ====');
function assert(b, str)
{
    if (b) {
        return;
    } else {
        throw Error("assertion failed: " + str);
    }
}
// console.log(typeof globalThis.a);
assert(typeof globalThis.a === 'undefined');

globalThis.a = 1;

assert(globalThis.a === 1);