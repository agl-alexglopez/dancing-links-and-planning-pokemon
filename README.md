# Dancing Links and Planning Pokémon

![cli-app](/images/pokemon-cli.png)

## Navigation
- Pokémon Planning
	- Dancing Links Implementation **([`pokemon_links.cc`](/src/pokemon_links.cc))**
- [Citations](#citations)

## Build Note

This project makes use of new C++ module features and therefore must be built with CMake >= 3.28.1, Clang >= 17.0.6, and Ninja >= 1.11.1. The C++ standard used is C++ 20 to enable use of modules and many other convenient features that cut down on lines of code significantly.

## Pokémon Planning Usage Instructions

I have created a small Command Line Interface program to demonstrate the interesting problems that the Dancing Links algorithm can solve. Read the Overview and breakdown of the algorithm below, but here is a quick start guide to see output in the terminal right away.

1. Clone the repository.
2. Build the project from the root of the repository.
    - There is a provided configuration in `CMakePresets.json` and `CMakeUserPresets.json` that looks for `clang++` and the `Ninja` build generator to support building C++ modules. Alter any of these flags and settings to your liking if you cannot build.
    - Use the cmake preset for realease mode `cmake --preset=rel` or with the provided convenience Makefile `make rel`.
3. Run the Command Line Interface application `./build/bin/pokemon_cli`.

Here are the flags and settings available for this program.

```txt
Pokemon CLI Usage:
    h                - Read this help message.
    plain            - Print without colors. Useful for piping or redirecting to file.
    color            - The default color output to the terminal using ANSI escape sequences.
    data/dst/map.dst - Path from the root of the repository to the generation map to solve.
    G[GYM NUMBER]    - Add as many gyms to your argument to solve cover problems only for those gyms.
    E4               - Add the "Elite Four" or equivalent stand-in final boss for a generation to the subset.
    A                - The Attack flag to solve the attack type cover problem.
    D                - The Defense flag to solve the defensive type cover problem. This is the default.
    E                - Solve an Exact cover problem. This the default.
    O                - Solve the overlapping cover problem
Example Command:
    ./build/bin/pokemon_cli G1 G2 G3 G4 data/dst/Gen-5-Unova2.dst
```

For what these types of cover problems mean, read the longer description below. A more robust and interesting graph cover visualizer is coming soon but is not complete yet. I find it interesting that only later generation maps have an exact cover for all possible types you will encounter in that generation. I am no expert on game design, but perhaps that communicates the variety and balance that Game Freak has achieved in their later games. However, looking at smaller subsets of gyms in the other maps can still be plenty of fun!

## Overview

This repository is an application of Donald Knuth's Dancing Links algorithm. For a deep dive on what that means and how I implemented my abstractions, please read my [post](https://agl-alexglopez.github.io/2025/08/07/the-pok%C3%A9mon-graph-cover-problem.html) on the topic at my [blog](https://agl-alexglopez.github.io/). Thank you!


## Citations

This project grew more than I thought it would. I was able to bring in some great tools to help me explore these algorithms. So, it is important to note what I am responsible for in this repository and what I am not.

As mentioned in the intro, the core ideas of Algorithm X via Dancing Links belongs to Knuth, I just implemented it a few different ways.

Stanford Course staff and Keith Schwarz is responsible for writing the graph drawing algorithm that makes working with `.dst` files so easy, found in `map_parser.cc`. This will likely become more helpful when I make more progress with the graph cover visualizer.

For the `all-types.json` file, I got the information on type interactions from the following website.

- https://pokemondb.net/type/dual

For the `all-maps.json` file, I got the information on gyms and the attack and defensive types present from the following website.

- https://serebii.net

## Next Steps

The graph cover visualizer is a work in progress. Thanks for reading!
