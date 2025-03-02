// console.log('==== test3.js ====');
function assert(b, str)
{
    if (b) {
        return;
    } else {
        throw Error("assertion failed: " + str);
    }
}

assert(typeof globalThis.a === 'undefined');

globalThis.a = 3;

assert(globalThis.a === 3);
