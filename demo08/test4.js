// console.log('==== test4.js ====');
function assert(b, str)
{
    if (b) {
        return;
    } else {
        throw Error("assertion failed: " + str);
    }
}

assert(typeof globalThis.a === 'undefined');

globalThis.a = 4;

assert(globalThis.a === 4);
