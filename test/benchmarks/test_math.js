#!/usr/bin/env node

var args = process.argv,
	iterations = 0;

if(args.length > 2){
	iterations = parseInt(args[2]);
}
console.log("Iterations: " + iterations);
var timeStart = new Date().getTime();

var a, b, c, d;
for(var i = 0; i < iterations; ++i)
{
	a = i + 1;
	b = a * 2;
	c = b / 3;
	d = c - a;
}

var timeEnd = new Date().getTime();
console.log("Test running time: " + (timeEnd - timeStart) + "ms");
