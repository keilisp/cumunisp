# Cumunisp

# Description

Simple lisp-like programming language written in C

# Installation

Clone the repo into your folder

```sh
cd *your-folder*
git clone https://github.com/mediocreeee/cumunisp.git
```

Compile the programm with make

```sh
make
```

Run the _cumunisp_ binary

```sh
./cumunisp
```

# Usage

## Mathematical Functions

### Addition

Returns the result of adding two or more numbers

```common-lisp
(+ 3 2) or (add 3 2)
; Output:
> 5
```

### Subtraction

Returns the result of substracting two or more numbers

```common-lisp
(- 3 2) or (sub 3 2)
; Output:
> 1
```

### Multiplication

Returns the result of multiplying two or more numbers

```common-lisp
(* 3 2) or (mul 3 2)
; Output:
> 6
```

### Division

Returns the result of dividing two or more numbers

```common-lisp
(/ 6 2) or (div 6 2)
; Output:
> 3
```

### Remainder

Returns the result of dividing two or more numbers

```common-lisp
(% 3 2) or (rem 3 2)
; Output:
> 1
```

### Power

Returns the result of raising a number to a power equal to the next number in the queue. If the number of numbers is more than 2 then raises the result of each operation to a power equal to the next number in the queue

```common-lisp
(^ 3 2) or (pow 3 2)
; Output:
> 9
```

```common-lisp
(^ 3 2 2) or (pow 3 2 2)
; Output:
> 81
```

### Min

Returns the minimum number from the queue

```common-lisp
(min 4 5 65 76 7)
; Output:
> 4
```

### Max

Returns the maximum number from the queue

```common-lisp
(max 4 5 65 76 7)
; Output:
> 76
```

## Comparison Functions

### If

```common-lisp
( if (statement is true)
    {do this}
    {else do this})
```

```common-lisp
( if (== (* 2 2) 4)
    {print "Equal"}
    {print "Not Equal"})

; Output:
"Equal"
()
```

### Equal

Returns the result of comparing two numbers to equality (1 - _true_, 0 - _false_)

```common-lisp
(== 3 3)
; Output:
> 1
```

```common-lisp
(== 3 2)
; Output:
> 0
```

### Not Equal

Returns the result of comparing two numbers to inequality (1 - _true_, 0 - _false_)

```common-lisp
(!= 3 2)
; Output:
> 1
```

```common-lisp
(!= 3 3)
; Output:
> 0
```

### Greater Than

Returns 1 if the first number is greater than the second, otherwise returns 0

```common-lisp
(> 3 2)
; Output:
> 1
```

```common-lisp
(> 2 3)
; Output:
> 0
```

```common-lisp
(> 3 3)
; Output:
> 0
```

### Greater Equal

Returns 1 if the first number is greater than or equal to the second, otherwise returns 0

```common-lisp
(>= 3 3)
; Output:
> 1
```

```common-lisp
(>= 3 2)
; Output:
> 1
```

```common-lisp
(>= 2 3)
; Output:
> 0
```

### Lower Than

Returns 1 if the first number is lower than the second, otherwise returns 0

```common-lisp
(< 2 3)
; Output:
> 1
```

```common-lisp
(< 3 2)
; Output:
> 0
```

```common-lisp
(< 3 3)
; Output:
> 0
```

### Lower Equal

Returns 1 if the first number is lower than or equal to the second, otherwise returns 0

```common-lisp
(<= 3 3)
; Output:
> 1
```

```common-lisp
(<= 2 3)
; Output:
> 1
```

```common-lisp
(<= 3 2)
; Output:
> 0
```

## List Functions

### List

This function returns a list(q-expression) created from its arguments

```common-lisp
(list 1 2 3 4)
; Output:
{1 2 3 4}
```

### Head

This function returns the first member of the list

```common-lisp
(head (list 1 2 3 4))
; Output:
{1}
```

### Tail

This function returns a list without the first member

```common-lisp
(tail (list 1 2 3 4))
; Output:
{2 3 4}
```

### Init

This function returns a list without the last argument

```common-lisp
(init (list 1 2 3 4))
; Output:
> {1 2 3}
```

### Eval

This function returns the result of the evaluation of the q-expression

```common-lisp
(eval {head (list 1 2 3 4)})
; Output:
> {1}
```

### Join

This function joins two lists into one

```common-lisp
(join (list 1 2) (list 3 4))
; Output:
> {1 2 3 4}
```

### Cons

This function takes the first argument(of any type) and appends it to the list from the seconf argument

```common-lisp
(cons 1 (list 2 3 4))
; Output:
> {1 2 3 4}
```

### Len

This function returns the length of given list

```common-lisp
(len (list 1 2 3 4))
; Output:
> 4
```

## String Functions

### Load

This functions loads and evaluates given file

```common-lisp
(load "prelude.cp")
; Output:
> ()
```

### Print

This function prints each argument separated by a space. It return the empty expression

```common-lisp
(print "Some Useful Info"
; Output:
> "Some Useful Info"
()
```

### Err

This function takes the string and provides it as an error message

```common-lisp
(err "Oh no! Error!")
; Output:
> Error: Oh no! Error!
```

## Variable Functions

### =/Def

This function defines variable(s)

```common-lisp
(= {x y} 5 25) or (def {x y} 5 25)
(+ x y)
; Output:
> 30
```

## \\

This functions defines lambda function

```common-lisp
(\ {args body}
    {def (head args)
    (\ (tail args) body)})
```

## Standart Library

// TODO: add examples of usage

### Functional Functions

#### Function Definition

```common-lisp
(def {fun} (\ {f b} {
  def (head f) (\ (tail f) b)}))
```

#### Open new scope

```common-lisp
(fun {let b} {
  ((\ {_} b) ())
})
```

#### Unpack List to Function

```common-lisp
(fun {unpack f l} {
  eval (join (list f) l)
})
```

#### Unapply List to Function

```common-lisp
(fun {pack f & xs} {f xs})
```

#### Curried and Uncurried calling

```common-lisp
(def {curry} unpack)
(def {uncurry} pack)
```

#### Perform Several things in Sequence

```common-lisp
(fun {do & l} {
  if (== l nil)
    {nil}
    {last l}})
```

### Logical Functions

#### Not

```common-lisp
(fun {not x}   {- 1 x})
```

#### Or

```common-lisp
(fun {or x y}  {+ x y})
```

#### And

```common-lisp
(fun {and x y} {* x y})
```

### Conditional Functions

#### Select

```common-lisp
(def {otherwise} true)

(fun {select & cs} {
  if (== cs nil)
    {err "No Selection Found"}
    {if (fst (fst cs)) {snd (fst cs)} {unpack select (tail cs)}}})
```

#### Case

```common-lisp
(fun {case x & cs} {
if (== cs nil)
{err "No Case Found"}
{if (== x (fst (fst cs))) {snd (fst cs)} {
unpack case (join (list x) (tail cs))}}})
```

### List Functions

#### First Item in List

```common-lisp
(fun {fst l} { eval (head l)})
```

#### Second Item in List

```common-lisp
(fun {snd l} { eval (head (tail l))})
```

#### First, Second, or Third Item in List

```common-lisp
(fun {trd l} { eval (head (tail (tail l)))})
```

#### Nth item in List

```common-lisp
(fun {nth n l} {
if (== n 0)
{fst l}
{nth (- n 1) (tail l)}})
```

#### Last item in List

```common-lisp
(fun {last l} {nth (- (len l) 1) l})
```

#### Apply Function to List

```common-lisp
(fun {map f l} {
if (== l nil)
{nil}
{join (list (f (fst l))) (map f (tail l))}})

```

#### Apply Filter to List

```common-lisp
(fun {filter f l} {
if (== l nil)
{nil}
{join (if (f (fst l)) {head l} {nil}) (filter f (tail l))}})
```

#### Reverse List

```common-lisp
(fun {reverse l} {
if (== l nil)
{nil}
{join (reverse (tail l)) (head l)}})
```

#### Fold Left

```common-lisp
(fun {foldl f z l} {
if (== l nil)
{z}
{foldl f (f z (fst l)) (tail l)}})
```

#### Fold Right

```common-lisp
(fun {foldr f z l} {
if (== l nil)
{z}
{f (fst l) (foldr f z (tail l))}})
```

#### Sum of List

```common-lisp
(fun {sum l} {foldl + 0 l})
```

#### Product of List

```common-lisp
(fun {product l} {foldl \* 1 l})
```

#### Take N items

```common-lisp
(fun {take n l} {
if (== n 0)
{nil}
{join (head l) (take (- n 1) (tail l))}})
```

#### Drop N items

```common-lisp
(fun {drop n l} {
if (== n 0)
{l}
{drop (- n 1) (tail l)}})
```

#### Split at N

```common-lisp
(fun {split n l} {list (take n l) (drop n l)})
```

#### Take While

```common-lisp
(fun {take-while f l} {
if (not (unpack f (head l)))
{nil}
{join (head l) (take-while f (tail l))}})
```

#### Drop While

```common-lisp
(fun {drop-while f l} {
if (not (unpack f (head l)))
{l}
{drop-while f (tail l)}})
```

#### Element of List

```common-lisp
(fun {elem x l} {
if (== l nil)
{false}
{if (== x (fst l)) {true} {elem x (tail l)}}})
```

#### Find element in list of pairs

```common-lisp
(fun {lookup x l} {
if (== l nil)
{err "No Element Found"}
{do
(= {key} (fst (fst l)))
(= {val} (snd (fst l)))
(if (== key x) {val} {lookup x (tail l)})}})
```

#### Zip two lists together into a list of pairs

```common-lisp
(fun {zip x y} {
if (or (== x nil) (== y nil))
{nil}
{join (list (join (head x) (head y))) (zip (tail x) (tail y))}})

```

#### Unzip a list of pairs into two lists

```common-lisp
(fun {unzip l} {
if (== l nil)
{{nil nil}}
{do
(= {x} (fst l))
(= {xs} (unzip (tail l)))
(list (join (head x) (fst xs)) (join (tail x) (snd xs)))}})
```

### Misc

#### Fibonacci Number

```common-lisp
(fun {fib n} {
  select
    { (== n 0) 0 }
    { (== n 1) 1 }
    { otherwise (+ (fib (- n 1)) (fib (- n 2)))}})
```
