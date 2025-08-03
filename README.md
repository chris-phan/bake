# Bake

## Getting started

### Format

Every bake file begins with a recipe name:
```md
# ChocolateLayerCake
```

Optionally, we can have ingredients and equipment
```md
## Ingredients
  1. BreadFlour (700)
  1. WholeWheatFlour (200)
  1. RyeFlour (100)
  1. Water (850)
  1. Starter (200)
  1. Salt (20)

## Equipment
  1. WorkSurface (0)
  1. Banneton (2 "")
  1. DutchOven ("")
  1. LargeMixingBowl (10 L of Int)
```

Important! These are the only two places you can declare variables! Be sure to have everything ready before you start baking.

Finally, the instructions:
```md
## Instructions
  1. add BreadFlour to LargeMixingBowl!
  1. add WholeWheatFlour to LargeMixingBowl?
  1. add RyeFlour to LargeMixingBowl!
  1. add Water to LargeMixingBowl!
  1. add Starter to LargeMixingBowl!
  1. add Salt to LargeMixingBowl!
```

### Variables

There 4 types in bake: integer, string, array of integers, and array of strings. They're initialized in the ingredients or equipment section. The value inside the parenthesis is the initial value.

Integer:
```md
  1. Thermometer (70)
```

String:
```md
  1. FavoriteFood ("viennoiserie")
```

All arrays are fixed size. There are two parts to their initialization. The first number is the size of the array, and the second part is the value that all the entries are initially set to.

Array of integers:
```md
  1. BakingSheet (12 0)
```

Array of strings:
```md
  1. StandMixer (5 "")
```

Arrays are indexed starting at 1. To access an specific entry in the array, use the `'s#` operation.

This accesses the 3rd entry in `MuffinTin`
```md
MuffinTin's#3
```

### Operations

Operations are found under the instructions section and are ended by an `!`.

```md
  1. add EggWhites to StandMixer!
```

If you're unsure about a step, you can end it with a `?`.
```md
  1. remove Degrees75F from Oven?
```

Or both
```md
  1. multiply 213078 by 12089 in your Head!?
```

Or more
```md
  1. serve Brunch?!!???
```

When specifying a destination to store the result of an operation, you can use the following prepositions:
  1. Aboard
  1. Above
  1. Across
  1. After
  1. Against
  1. Aloft
  1. Along
  1. Alongside
  1. Amid
  1. Amidst
  1. Mid
  1. Midst
  1. Among
  1. Amongst
  1. Around
  1. Round
  1. Aslant
  1. Astride
  1. At
  1. Atop
  1. Ontop
  1. Before
  1. Behind
  1. Below
  1. Beneath
  1. Neath
  1. Beside
  1. Besides
  1. Beyond
  1. By
  1. Circa
  1. Down
  1. Following
  1. In
  1. Inside
  1. Into
  1. Near
  1. Next
  1. Off
  1. On
  1. Onto
  1. Opposite
  1. Out
  1. Outside
  1. Over
  1. Past
  1. Through
  1. Throughout
  1. To
  1. Touching
  1. Toward
  1. Towards
  1. Under
  1. Underneath
  1. Unto
  1. Up
  1. Upon
  1. With
  1. Within

For example,
```md
  1. multiply Layers by 2 throughout the Dough!
```

#### Addition
In place
```md
  1. add EggWhites to StandMixer!
```
With a destination
```md
  1. add EggWhites and GranulatedSugar to StandMixer!
```
#### Subtraction
In place
```md
  1. remove Water from UnsaltedButter!
```
With a destination
```md
  1. remove Water from UnsaltedButter in SaucePan!
```
#### Multiplication
In place
```md
  1. multiply DoughVolume by 2!
```
With a destination
```md
  1. multiply DoughVolume by 2 in Bowl!
```
#### Division
In place
```md
  1. split CookieDough into 12!
```
With a destination
```md
  1. split CookieDough into 12 onto SheetTray!
```

#### Modulo
In place
```md
  1. 10 mod 3!
```
With a destination
```md
  1. 10 mod 3 to MasonJar!
```

#### Printing
```md
  1. serve Dessert!
```

### Articles
Like whitespace, the articles `a`, `an`, and `the` are ignored. `your` is also ignored. Use these to really get your point across.
```md
  1. serve the Food!
```
Show off your stutter
```md
  1. add a a a a CupOfWater to your your MixingBowl?
```

### Conditions

```md
  1. if Dough is Sticky: add Flour to Dough!
```
Supported operations: `<`, `<=`, `>`, `>=`, `is`, and `is not`.

To do multiple operations in the same if condition, simply repeat the condition:
```md
  1. if Dough is Sticky: add Flour to Dough!
  1. if Dough is Sticky: add Dough to Fridge!
```

To do an else, reverse the condition:
```md
  1. if Cookie is Stale or NumCookies < 2: add CookieDough to Oven!
  1. if Cookie is not Stale and NumCookies >= 2: add Cookie to Stomach!
```

### Loops

Start a loop with `repeat until` and end the block with `repeat`
```md
  1. repeat until Scale is 100:
  1. add APFlour to Scale!
  1. repeat!
```

### Comments

Single line comments:
```md
  1. serve "bake"!    Note: prints "bake"
```
Up to the newline, everything after `Note` is ignored.

### Functions

No

### Classes

No