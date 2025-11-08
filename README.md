# Dancing Links and Planning Pokémon

![gui-app](/images/pokemon-gui.png)

## Navigation

- [Pokemon Planning Web App](#pokémon-planning-web-app)
- [Pokemon Planning GUI](#pokémon-planning-gui-usage-instructions)
- [Pokemon Planning CLI](#pokémon-planning-cli-usage-instructions)
- [Citations](#citations)

> [!NOTE]
> This project makes use of new C++ module features and therefore must be built with CMake >= 3.28.1, Clang >= 17.0.6, and Ninja >= 1.11.1. The C++ standard used is C++ 20 to enable use of modules and many other convenient features that cut down on lines of code significantly.

## Pokémon Planning Web App

While you can build and run the GUI locally, there is also a live web app version of the program. To jump right in and give it a whirl, try the [web app powered raylib and Emscripten](https://agl-alexglopez.github.io/dancing-links-and-planning-pokemon/)!

> [!TIP]
> Be sure to enable the window resizing and disable mouse locking and hiding for the program to work correctly. Also, it seems to work best in full screen mode.

If you would like to build and view the web app locally follow these steps.

1. Clone the repository.

```zsh
git clone https://github.com/agl-alexglopez/ccc.git
```

2. Set up the `emsdk` submodule. This remains contained to the submodule directory and does not touch system paths.

```zsh
make emsdk
```

3. Build the web application. Only a small release build is supported.

```zsh
make webrel
```

4. Run the web application on your local browser with `emrun`.

```zsh
make emrun
```

The same advice applies. Disable mouse locking and enable resizing. Use full screen for best results.

## Pokémon Planning GUI Usage Instructions

This program implements an interactive Dancing Links graph cover visualizer. The user is able to load in any of 9 generation Pokemon maps and solve various graph cover questions. These questions basically boil down to the following:

- What team of at most 6 Pokemon can I choose to be resilient to any attack type I will encounter in the game?
- Of my 24 attack slots for my 6 Pokemon, which attack types can I choose to give them so that I am super effective against any defensive types I will encounter in the game?

These questions can be answered for an entire game or a subset of maps to choose from on the mini map. If solutions exist, the best will be shown as the inner ring of attack or defensive types that covers all the requested attack or defensive types that surround this inner ring. The inner ring has edges that lead to the types that are covered. These edges are color coded to indicate the quality of the solution.

A line is colored based on the multiplier of the type interaction. If the inner ring is defensive types than the edges indicate what resistance multiplier the defensive type receives against the type it covers. The multiplier could be 0.5, 0.25, or 0.0. These are all good but obviously a 0.0 multiplier is best because it means immunity from damage. If the inner ring is attack types the edges indicate the damage multiplier these types do against the defensive types they cover. The multiplier could be 2 or 4. Both are good but a 4x multiplier is best.  

The user can cycle through the top ranked solutions provided. All solutions shown tied for the best rank among all solutions. Solutions are ranked based on the quality of their beneficial multipliers against other types. When ranks are tied, the smaller set wins. As the user cycles through the solutions that tied with the highest rank, smaller solutions sets will be shown first. The reasoning is that while building a Pokemon team or selecting types for attack slots, using as few slots as possible frees up space in the team for other purposes. 

Hovering over nodes will show their full type names. If hovering over covered nodes in the surrounding circle, the multiplier is indicated and the text matches the edge color.

1. Clone the repository.
2. Build the project from the root of the repository.
    - There is a provided configuration in `CMakePresets.json` and `CMakeUserPresets.json` that looks for `clang++` and the `Ninja` build generator to support building C++ modules. Alter any of these flags and settings to your liking if you cannot build.
    - Use the cmake preset for realease mode `cmake --preset=rel` or with the provided convenience Makefile `make rel`.
3. Run the Command Line Interface application `./build/bin/pokemon_gui`.

## Pokémon Planning CLI Usage Instructions

![cli-app](/images/pokemon-cli.png)

While the GUI app is fun, the CLI is another way to interact with the solver. Rather than only showing the solutions that tied for 1st place among all ranked solutions, this application shows **every** possible solution. Often, this means every solution is enumerated on the command line. However, solutions are cut off here as well if the program is running too long.

1. Clone the repository.
2. Build the project from the root of the repository.
    - There is a provided configuration in `CMakePresets.json` and `CMakeUserPresets.json` that looks for `clang++` and the `Ninja` build generator to support building C++ modules. Alter any of these flags and settings to your liking if you cannot build.
    - Use the cmake preset for realease mode `cmake --preset=rel` or with the provided convenience Makefile `make rel`.
3. Run the Command Line Interface application `./build/bin/pokemon_cli`.

Here are the flags and settings available for this program.

```txt
Pokemon CLI Usage:
    h                  - Read this help message.
    plain              - Print without colors. Useful for piping or redirecting to file.
    color              - The default color output to the terminal using ANSI escape sequences.
    [Region]           - The name of any region from generations 1-9 (e.g. Paldea).
    [Town]             - Add town names to cover. Towns must have gyms or the elite four for now.
    A                  - The Attack flag to solve the attack type cover problem.
    D                  - The Defense flag to solve the defensive type cover problem. This is the default.
    E                  - Solve an Exact cover problem. This the default.
    O                  - Solve the overlapping cover problem
Example Command:
    ./build/bin/pokemon_cli Paldea Cortondo Artazon O
```

For what these types of cover problems mean, read the longer description below. A more robust and interesting graph cover visualizer is coming soon but is not complete yet. I find it interesting that only later generation maps have an exact cover for all possible types you will encounter in that generation. I am no expert on game design, but perhaps that communicates the variety and balance that Game Freak has achieved in their later games. However, looking at smaller subsets of gyms in the other maps can still be plenty of fun!

## Overview

This repository is an application of Donald Knuth's Dancing Links algorithm. For a deep dive on what that means and how I implemented my abstractions, please read my [post](https://agl-alexglopez.github.io/2025/08/07/the-pok%C3%A9mon-graph-cover-problem.html) on the topic at my [blog](https://agl-alexglopez.github.io/). Thank you!


## Citations

This project grew more than I thought it would. I was able to bring in some great tools to help me explore these algorithms. So, it is important to note what I am responsible for in this repository and what I am not.

As mentioned in the intro, the core ideas of Algorithm X via Dancing Links belongs to Knuth, I just implemented it a few different ways. Stanford course staff and Keith Schwarz implemented a graph drawing algorithm to draw the mini maps so they spread to fill available space for each region. The GUI uses some of this logic to draw generation town maps.

For the `all-types.json` file, I got the information on type interactions from the following website.

- https://pokemondb.net/type/dual

For the `all-maps.json` file, I got the information on gyms and the attack and defensive types present from the following website.

- https://serebii.net

