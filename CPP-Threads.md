# CPP-Threads

### Overview

Write a platform independent C++11 application that takes a list of [polish notation](https://en.wikipedia.org/wiki/Polish_notation)
expressions and utilizes STL concurrency to evaluate each expression and prints the end result of each
expression in the list to stdout. Each expression should be fully parallelized. Operations supported are:
addition, subtraction, multiplication, and division.

### Nested expressions

`(+ 1 1)` contains one expression

`(+ 1 ( + 1 1))` contains two expressions

`(+ (+ 1 1) (+ 2 2))` contains three expressions


### Input
A list of polish notation expressions provided by stdin delineated by new-line characters. Each sub-expression will be
wrapped in parentheses and spacing will be consistent/follow the example provided below. Expression syntax will always
be valid, but your program should account for special cases like division by zero and handle them gracefully.

### Output
A list of polish notation expressions and their results. (Said list does not have to be in the same order as the input)

### Example

###### `expressions.txt` contents:

>(+ 1 1)

>(- 2 3)

>(* 5 20)

>(/ 10 2)

>(+ (* 2 2) 10)

>(/ 10 0)

>(/ (- 1 1) 42)

>(+ 2 (* (- 2 (/ 8 2)) 8))

>(+ (* (+ 2 (* (- 2 (/ 8 2)) 8)) (+ 2 (* (- 2 (/ 8 2)) 8))) (* (+ 2 (* (- 2 (/ 8 2)) 8)) (+ 2 (* (- 2 (/ 8 2)) 8))))


###### Calling pattern
`./polish-calc < expressions.txt`


###### Output (may be in any order)

>(+ 1 1) = 2

>(- 2 3) = -1

>(* 5 20) = 100

>(/ 10 2) = 5

>(+ (* 2 2) 10) = 14

>(/ 10 0) = ERROR (Division by zero)

>(/ (- 1 1) 42) = 0

>(+ 2 (* (- 2 (/ 8 2)) 8)) = -14

>(+ (* (+ 2 (* (- 2 (/ 8 2)) 8)) (+ 2 (* (- 2 (/ 8 2)) 8))) (* (+ 2 (* (- 2 (/ 8 2)) 8)) (+ 2 (* (- 2 (/ 8 2)) 8)))) = 392
