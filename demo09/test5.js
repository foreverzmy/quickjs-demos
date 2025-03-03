let id1 = setTimeout(() => { 
  console.log('Timer 1 expired.'); 
}, 1000);

let id2 = setTimeout(() => { 
  console.log('Timer 2 expired.'); 
}, 2000);

let id3 = setTimeout(() => { 
  console.log('This should not be printed.'); 
}, 3000);

clearTimeout(id3);

console.log('Timers scheduled');
