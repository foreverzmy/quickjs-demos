// fetch('https://jsonplaceholder.typicode.com/todos/1')
//   .then(response => {
//     console.log('===Response:', response);
//   })
//   .catch(error => {
//     console.error('===Error:', error);
//   });

console.log('----typeof fetch----：', typeof fetch);

console.log();
console.log('----fetch without URL----');
try {
  const resp = await fetch();
  console.log('----fetch without URL----：', resp);
} catch (error) {
  console.log('----fetch empty error----：', error);
}

console.log();
console.log('----fetch with URL----');
const resp = fetch('https://jsonplaceholder.typicode.com/todos/1')
console.log('----fetch resp----：', resp);

resp
  .then(response => {
    console.log('----fetch response----:', response);
  })
  .catch(error => {
    console.error('----fetch error----:', error);
  });

