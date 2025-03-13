/**
 * Complex JavaScript Benchmark Suite
 *
 * This code includes a variety of computationally intensive operations:
 * - Fibonacci calculation (recursive and iterative)
 * - Prime number generation
 * - Array manipulations
 * - Object property access
 * - String operations
 * - Complex mathematical calculations
 */

// Recursive Fibonacci implementation (exponential complexity)
function fibonacciRecursive(n) {
	if (n <= 1) return n;
	return fibonacciRecursive(n - 1) + fibonacciRecursive(n - 2);
}

// Iterative Fibonacci implementation (linear complexity)
function fibonacciIterative(n) {
	if (n <= 1) return n;
	let a = 0;
	let	b = 1;
	for (let i = 2; i <= n; i++) {
		const temp = a + b;
		a = b;
		b = temp;
	}
	return b;
}

// Prime number check
function isPrime(num) {
	if (num <= 1) return false;
	if (num <= 3) return true;
	if (num % 2 === 0 || num % 3 === 0) return false;

	let i = 5;
	while (i * i <= num) {
		if (num % i === 0 || num % (i + 2) === 0) return false;
		i += 6;
	}
	return true;
}

// Generate prime numbers up to limit
function generatePrimes(limit) {
	const primes = [];
	for (let i = 2; i <= limit; i++) {
		if (isPrime(i)) {
			primes.push(i);
		}
	}
	return primes;
}

// Complex object operations
function objectOperations(iterations) {
	const result = {};

	for (let i = 0; i < iterations; i++) {
		const key = `key${i}`;
		result[key] = {
			value: i,
			nested: {
				square: i * i,
				cube: i * i * i,
				sqrt: Math.sqrt(i),
			},
			isPrime: isPrime(i),
			fibonacci: fibonacciIterative(i % 25), // Limit to avoid excessive computation
		};
	}

	// Access and manipulate properties
	let sum = 0;
	for (let i = 0; i < iterations; i++) {
		const key = `key${i}`;
		if (result[key].isPrime) {
			sum += result[key].nested.square;
		} else {
			sum += result[key].nested.cube;
		}
	}

	return sum;
}

// Array manipulations
function arrayOperations(size) {
	// Create and fill array
	const array = new Array(size);
	for (let i = 0; i < size; i++) {
		array[i] = Math.sin(i) * Math.cos(i);
	}

	// Sort array
	array.sort((a, b) => a - b);

	// Filter elements
	const filtered = array.filter((item) => item > 0);

	// Map elements
	const mapped = filtered.map((item) => item * item);

	// Reduce to sum
	const sum = mapped.reduce((acc, val) => acc + val, 0);

	return sum;
}

// String operations
function stringOperations(iterations) {
	let result = "";

	for (let i = 0; i < iterations; i++) {
		result += `Iteration ${i}: `;

		// String manipulations
		if (i % 3 === 0) {
			result += "Fizz";
		}
		if (i % 5 === 0) {
			result += "Buzz";
		}
		if (i % 3 !== 0 && i % 5 !== 0) {
			result += i.toString();
		}

		result += "\n";
	}

	// String searching and replacing
	const count = (result.match(/Fizz/g) || []).length;
	const replaced = result.replace(/Buzz/g, "BUZZ");

	return replaced.length + count;
}

// Matrix multiplication
function matrixMultiply(size) {
	// Create matrices
	const matrixA = [];
	const matrixB = [];

	for (let i = 0; i < size; i++) {
		matrixA[i] = [];
		matrixB[i] = [];
		for (let j = 0; j < size; j++) {
			matrixA[i][j] = i + j;
			matrixB[i][j] = i - j;
		}
	}

	// Multiply matrices
	const result = [];
	for (let i = 0; i < size; i++) {
		result[i] = [];
		for (let j = 0; j < size; j++) {
			result[i][j] = 0;
			for (let k = 0; k < size; k++) {
				result[i][j] += matrixA[i][k] * matrixB[k][j];
			}
		}
	}

	// Sum of all elements
	let sum = 0;
	for (let i = 0; i < size; i++) {
		for (let j = 0; j < size; j++) {
			sum += result[i][j];
		}
	}

	return sum;
}

// Benchmark all functions
function runBenchmark() {
	// Timer values
	const start = Date.now();
	const results = {};

	// 1. Fibonacci calculations
	results.fibRecursive = fibonacciRecursive(20);
	results.fibIterative = fibonacciIterative(40);

	// 2. Prime number generation
	results.primes = generatePrimes(5000).length;

	// 3. Object operations
	results.objectOps = objectOperations(500);

	// 4. Array operations
	results.arrayOps = arrayOperations(2000);

	// 5. String operations
	results.stringOps = stringOperations(300);

	// 6. Matrix multiplication
	results.matrixMultiply = matrixMultiply(25);

	// Final combination calculation
	let combinedResult = 0;
	for (const key in results) {
		if (results[key]) {
			combinedResult += results[key];
		}
	}

	const end = Date.now();
	const totalTime = end - start;

	return {
		individualResults: results,
		combinedResult: combinedResult,
		totalTimeMs: totalTime,
	};
}

// Run the benchmark
globalThis.benchmarkResult = runBenchmark();
// console.log("Benchmark completed.");
// console.log("Total time:", benchmarkResult.totalTimeMs, "ms");
// console.log("Combined result:", benchmarkResult.combinedResult);
