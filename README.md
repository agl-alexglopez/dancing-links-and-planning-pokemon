# Dancing Links and Planning Pokémon

## Navigation
- Pokémon Planning
	- Dancing Links Class **([`PokemonLinks.h`](/dancing-links-and-planning-pokemon/dlapp/Src/PokemonLinks.h))**
	- Dancing Links Implementation **([`PokemonLinks.cpp`](/dancing-links-and-planning-pokemon/dlapp/Src/PokemonLinks.cpp))**
- [Citations](#citations)

## Build Note

In order to build this project, you will need a copy of the CS106 C++ library from the Winter of 2022. To the best of my knowledge, newer libraries from Stanford's class websites have some graphics conflicts that prevent this assignment from building. I have included a copy of this library as a `.zip` file in the `/packages/` folder. Unzip the folder in a separate location on your computer because it will need to built as its own Qt project. You can then follow the same instructions to build this library that Stanford normally provides on their course websites [HERE](https://web.stanford.edu/dept/cs_edu/resources/qt/).

Instead of building their new libraries, however, you will just build this older version. It will install all the necessary Stanford specific libraries on your system for use with Qt.

## Overview

In October of 2022, Donald Knuth released *The Art of Computer Programming: Volume 4b: Combinatorial Algorithms, Part 2*. In this work he revised his previous implementation of his Algorithm X via Dancing Links. His revision included changing the quadruple linked nodes that solved backtracking problems to an array of nodes. This is an interesting optimization. It provides good locality, ease of debugging, and memory safety if you use an array that does not need to be managed like a C++ vector. One of the points Knuth makes clear through his section on Dancing Links is that he wants to give people the tools they need to expand and try his backtracking strategies on many problems. If you want the original Algorithm, please read Knuth's work. Below, I will discuss how I applied or modified his strategies to fit a fun puzzle I have often considered for the game of Pokémon.

## Pokémon Type Coverage

Knuth brings up many puzzles in his work on Combinatorial Algorithms. He has word puzzles, number puzzles, visual *puzzle* puzzles. All of them got me thinking about more problems I could find that use his methods and I found one! The videogame Pokémon by developer Game Freak is part of a well established franchise that started in the 1990's. At their core, the Pokémon videogames are a game of rock-paper-scissors with a few dozen layers of complexity added on top. While I can't explain the entire ruleset of Pokémon here, I can explain the relevant parts to the problem I decided to solve.

In Pokémon, trainable animals/monsters battle on your behalf imbued with pseudo-elemental types like fire, water, flying, ground, and many others. There are 15 to 18 fundamental types that Pokémon can use depending on the generation of games you are considering. Because Pokémon has been around for so many years, they have made many changes to their games. The community that plays those games often divides the games by generations in which different game mechanics were introduced. Right now, there are nine generations. At the most abstract level you only need to consider how to attack other Pokémon with these fundamental types and how to defend against these attack types with your own Pokémon. The twist on defense is that your Pokémon can have up to two different types--such as Fire-Flying or Ice-Water--that determine how weak or resistant they are to these fundamental attack types. While these fundamental types could be combined to form 306 dual types, counting dual types in different orders as different types, not to mention the additional 15 to 18 single types, the developers of this game have not done so yet. Depending on the generation, there are far fewer types than this, but there are still many. The most recent release is up to 162 unique Pokémon types.

So, there are two cover problems hiding in the complexities of the Pokémon games: one for defense and one for attack. The two essential cover questions we can ask are as follows.

- Which teams of at most 6 Pokémon--the most you can carry with you at once--give me resistance to every attack type I will encounter? If you consider an entire game, you would want to know the answer to that question for every attack type in the game. If you are considering just the attacks you will face in some portions of the game, then the range of attack type shrinks but the question remains the same.
- Which attack types can I choose to be effective against every defensive type I will encounter in the game? Again, considering the entire game versus some smaller sections will change the range of defensive types you will see, but the question remains the same.

To try to answer these questions we can begin to organize Pokémon's data with Knuth's dancing links method.

### Defense

Let's start with the defensive case. Here is a small subset of both attack and defensive types we can look at as a toy example.

![defense-links](/images/defense-links.png)

Here are the key details from the above image.

- Attack types we need to cover are our items and defensive types we can use to resist those attack types are our options.
- There are different levels of resistance that types can have to attack types. You can think of resistances as fractional multipliers that are applied to what would otherwise be normal damage. The best we can do is nullify damage by being immune to that type. You can see this in the cases of Ground-Water vs Electric and Bug-Ghost vs Normal type attacks. A `x0.25` multiplier is the next best followed by a `x0.50`.
- While single defense types exist, I am not including them here to avoid confusion.
- The order of the two types in a dual type does not change any resistance or weakness to attack types. I organize all dual-types alphabetically for consistency.

To solve an **exact cover** version of the defensive problem, as Knuth describes in his work, we want to resist each of these attack types exactly once across the options we choose. You can take a moment to try to pick the types that achieve this before we go over the selection process.

![defense-links-2](/images/defense-links-2.png)

Here are the key details from the above image.

- We start by trying to cover an attack type that appears the least across all options, Normal. We choose option `Bug-Ghost`.
- This choice eliminates `Bug-Ghost` and any other options that contain the attack items `Grass` and `Normal` because those items have already been covered by our current choice.

We are then left with a shrunken grid to consider solving.

![shrink-defense-1](/images/shrink-defense-1.png)

If we follow the same selection and elimination process we are led to the following solution.

![solve-defense](/images/solve-defense.png)

Here are the key details from the above image.

- Our remaining two selections are marked with the red asterisk.
- We know we have solved the problem when no items remain in the world.
- The heuristic of selecting an item that appears few times across all options will often decrease the number of iterations of this algorithm.

While it is good to see how quickly this process shrinks the problem, it can sometimes be hard to see what about this selection process makes it an **exact cover**. To help visualize the exact nature of these choices here is another way to look at the cover solution.

![exact-walk-defense](/images/exact-walk-defense.png)

Here are the key details from the above image.

- No two defensive types that we chose covered the same attack type. I find it easier to think of this as a perfect walk across the items from left to right with no wasted steps, as illustrated by the line I take through these items.
- In addition to finding all solutions to this problem, I rank the solutions that I find according to a scoring system for defense.
  - The `x0.00` multiplier is `1 point`.
  - The `x0.25` multiplier is `2 points`.
  - The `x0.50` multiplier is `3 points`.
  - A **LOWER** score is better than a higher score. This favors stronger resistances.
- This scoring systems is an arbitrary product of my implementation based on what I assume is good in these games. I haven't thought critically about Pokémon in quite some time so maybe an expert could provide more guidance here.

All choices considered, here is the final score of our **exact cover** team of 3.

![defensive-cover](/images/defensive-cover.png)

In such a small case the significance of this score may not be apparent, but when generating thousands of solutions it can become helpful to sift through them with a ranking system. Let's now look at Attack.

### Attack

We will use the same types we did for the defensive example, but flip attack and defense.

![attack-links](/images/attack-links.png)

Here are the key details from the above image.

- Defensive types are the items we must cover with attack types that serve as our options.
- We can only have single attack types. No attack in the game does two simultaneous forms of damage.
- `Normal` is not effective against any types in this case. This is possible for many types depending on the size of the subproblem and simply means we will not choose it as an option. Fun fact: Normal is the only attack type that receives no damage multiplier against other types in the game.

The shrinking of this problem is exactly the same as in the defensive case--choose the item that appears the least across all options and try to cover it first--so instead I will show the **exact cover** walk across the items that forms the solution.

![exact-walk-attack](/images/exact-walk-attack.png)

Here are the key details from the above image.

- No two attack types are effective against the same defensive type. Just complete the walk from left to right to see this.
- In addition to choosing our attack types, we can rank them as we choose under the following point system.
  - The `x2` multiplier is `5 points`.
  - The `x4` multiplier is `6 points`.
  - A **HIGHER** score is better. This favors stronger attack types for the given defensive items.
- I use this scoring method because I assume quadruple damage is more desirable and a one point difference seems fair if we are only using it to distinguish the quality of the solutions we find.

Again, this ranking system will help when we generate many solutions. Here is the rank of our final picks.

![attack-cover](/images/attack-cover.png)

Now that I have summarized how **exact cover** works for the Pokémon type coverage problem, we can briefly discuss **overlapping coverage**.

### Overlapping Coverage

An **exact cover** can be difficult to achieve. Depending on the Pokémon generation, it can be impossible. In addition, most people don't think about being so exact with their choices so that no choice is wasted. Rather, they might just take a sweeping approach, trying to get as much coverage as possible. So, it can be fun to look at cover in a slightly different way, allowing for overlap. With a simple adjustment we can find many more solutions to the cover problem.

![overlapping-defense](/images/overlapping-defense.png)

Here are the key details from the above image.

- We select the same starting option `Bug-Ghost`.
- When we cover the items in those options we leave all other options that contain those items available to solve the cover problem.

By allowing other options to remain available we end up with a slightly different walk from left to right to solve the problem.

![overlapping-walk-defense](/images/overlapping-walk-defense.png)

Here are the key details from the above image.

- All scoring systems remain the same as in previous examples.
- Three of our choices for options overlap on the `Grass` coverage. This is acceptable because our goal is to simply cover all attack types within our 6 choice limit.

Here are the results of those choices.

![overlapping-defense-cover](/images/defense-overlapping-cover.png)

There are a few other **overlapping** covers within this example if you want to try finding your own to solidify the concepts. You could also try the Attack version, which operates under the exact same principles.

### Pokémon Planning Implementation

In order to accomplish the in-place, no-copy recursion that comes with Knuth's Dancing Links, I have chosen to use a C++ vector. In older implementations of Dancing Links, Knuth used a 4-way linked grid of nodes with up, down, left, and right fields. Now, the left-right fields of these nodes can be implicit because we place everything in one vector.

Here is the type that I use to manage the recursion and know when every item is covered. The name corresponds to the item.

```c++
struct typeName {
	TypeEncoding name;
	int left;
	int right;
};
std::vector<typeName> headers = {
	{{""},6,1},
	{{"Electric"},0,2},
	{{"Fire"},1,3},
	{{"Grass"},2,4},
	{{"Ice"},3,5},
	{{"Normal"},4,6},
	{{"Water"},5,0},
};
```

The `TypeEncoding` is a new addition to this project. Previously, this implementation produced solutions in string format. This means all input and output for the Pokémon types came in the form of `std::string`. Normally, this would be fine, but the exact cover problem as I have set it up communicates with sets and maps which means behind the scenes the algorithm is performing thousands if not millions of string comparisons of varying lengths. We can reduce all of these comparisons that are happening to a single comparison between two numbers. This will make moving data and some of the algorithms faster while vastly reducing the memory footprint. We simply encode all types into this format.

```c++
const size_t TYPE_TABLE_SIZE = 18;
// Lexographically organized table. 17th index is the first lexographical order Bug.
const std::string TYPE_ENCODING_TABLE[TYPE_TABLE_SIZE] = {
	"Water","Steel","Rock","Psychic","Poison","Normal","Ice","Ground","Grass",
	"Ghost","Flying","Fire","Fighting","Fairy","Electric","Dragon","Dark","Bug"
};

struct TypeEncoding {
	uint32_t encoding_;
	TypeEncoding() = default;
	TypeEncoding(std::string_view type);
	bool operator==(TypeEncoding rhs) const {
		return this->encoding_ == rhs.encoding_;
	}
	bool operator!=(TypeEncoding rhs) const {
		return !(*this == rhs);
	}
	// Not a mistake! Read on for why this works.
	bool operator<(TypeEncoding rhs) const {
		return this->encoding_ > rhs.encoding_;
	}
	bool operator>(TypeEncoding rhs) const {
		return rhs < *this;
	}
	bool operator<=(TypeEncoding rhs) const {
		return !(*this > rhs);
	}
	bool operator>=(TypeEncoding rhs) const {
		return !(*this < rhs);
	}
};

std::pair<std::string_view,std::string_view> to_pair(TypeEncoding type);

```

We place every Pokémon type in this `uint32_t` such that the 0th index bit is Water and the 17th index bit is Bug. In its binary representation it looks like this.

![type-encoding](/images/type-encoding.png)

We now have the ability to turn specific bits on in this type to represent the type we are working with. Turn on one bit for single types and two bits for dual types. For example, "Bug-Water" would be the following.

![type-encoding-bug-water](/images/type-encoding-bug-water.png)

The final challenge for this strategy is making sure that we can use this type as you would a normal string, in a set or map for example. This means that the `TypeEncoding` must behave the same as its string representation in terms of lexicographic sorting. To achieve this we must take a counterintuitive approach; the bits must be aligned in the HIGHEST order bit position according to LOWEST lexicographical order of the string they represent. 

So above, "Bug" will always be first in terms of lexicographical ordering among these strings. It's bit must be in the highest order position we have available. We need to ensure that any type that starts with "Bug" will always be less than any other possible type combination that does not start with bug, and so on for the next string in ascending order. The easy way to do this is to ensure that any `TypeEncoding` that contains the "Bug" bit is larger than one that does not (counterintuitive right?). That is why you see the `operator<` overload for this type flipped. Once we have consistent logic for this type we just need to flip the meaning of a larger value so it behaves like a normal string. If this is confusing consider the problems you would run into if you flipped this bit array, placing "Bug" at the zero index. The type "Bug-Water" would be a larger numeric value than "Ghost-Grass," but "Bug-Water" should be sorted first. It becomes a mess! Using the doubling nature of base 2 bits, we can achieve the consistency we want, we just need to take an odd approach. There are other bit tricks and strategies I use to implement this type efficiently but you can explore those in the code if you wish. 

The final optimization involves the newish type `std::string_view`. I try to avoid creating heap allocated strings whenever necessary. Because we must have a table with type names to refer to for the `TypeEncoding` I just point to those strings to display type information when decoding a `TypeEncoding`. I added this constraint to learn more about how to properly use `std::string_view` and I like the design decisions that followed. See the code for more.

Here is the type that I use within the dancing links array.

```c++
struct pokeLink {
    int topOrLen;
    int up;
    int down;
    // x0.0, x0.25, x0.5, x1.0, x2, or x4 damage multipliers.
    Multiplier multiplier;
    // Use this to efficiently find Overlapping covers.
    int tag;
};
```

The Multiplier is a simple `enum`.

```c++
enum Multiplier {
    EMPTY_=0,
    IMMUNE,  // x0.00
    FRAC14,  // x0.25
    FRAC12,  // x0.50
    NORMAL,  // x1.00
    DOUBLE,  // x2.00
    QUADRU   // x4.00
};
```

We then place all of this in one array. Here is a illustration of these links as they exist in memory.

![pokelinks-illustrated](/images/pokelinks-illustrated.png)

There is also one node at the end of this array to know we are at the end and that our last option is complete. Finally, to help keep track of the options that we choose to cover items, there is another array consisting only of names of the options. These also have an index for the option in the dancing links data structure for some class methods I cover in the next section.

```c++
struct encodingAndNum {
    TypeEncoding name;
    int num;
};
const std::vector<encodingAndNum> options = {
    {{""},0},
    {{"Bug-Ghost"},7},	
    {{"Electric-Grass"},10},
    {{"Fire-Flying"},14},	
    {{"Ground-Water"},17},
    {{"Ice-Psychic"},20},
    {{"Ice-Water"},22},
};
```

The spacer nodes in the dancing links array have a negative `topOrLen` field that correspond to the index in this options array and this array has the index of that option. There are other subtleties to the implementation that I must consider, especially how to use the depth tag to produce Overlapping Type Coverages, but that can all be gleaned from the code itself.

## Bonus: Justifying a PokemonLinks Class

I wrote this implementation as a class from the beginning. However, early on, I had doubts that this algorithm had any state or attributes that would help justify a class. I could have just as easily wrote a procedural algorithm in a functional style where I take input and provide solutions in my output. However, building the dancing links data structure is non-trivial, so building this structure on every inquiry seemed slow. With a few adjustments, invariants, and runtime guarantees I think there is a case to be made for the PokemonLinks class, and more generally Dancing Links classes for more general algorithms. With minor changes, all the techniques I discuss could be applied to any Dancing Links solver.

### Invariants

In order for the following techniques to work we must maintain some invariants in the Dancing Links internals.

1. Maintain the identifiers for items--for me this is the `string` name of the item--in a sorted vector. This is required by Knuth's algorithms already but I am adding that the items must be sorted for some later techniques.
2. Maintain the identifiers for the options--for me this is the string name of the option--in a sorted vector. This is not required by Knuth's algorithm, but my options have meaningful names that may be different than the item names so I must make this vector. This will also help with later techniques.
3. Do not support insertion or deletion of items or options from any data structure. Inserting an item or option may be possible but it would require substantial modification to the dancing links array that would either be slow or require more space to store the required information. We can support hiding items and options but not deleting them completely. All operations will be in-place.

### Searching

If a user wants to membership test an item or option we can make some guarantees because we maintain that all items and options are in sorted vectors.

```c++
namespace DancingLinks{
bool hasItem(const PokemonLinks& dlx, const TypeEncoding& item);
bool hasOption(const PokemonLinks& dlx, const TypeEncoding& option);
}
```

- `hasItem` - Finding an item is O(lgN) where N is the number of all items. You cannot find a hidden item.
- `hasOption` - Finding an option is O(lgN) where N is the number of all options. You cannot find a hidden option.

### Hiding Items

As a part of Algorithm X via Dancing Links, covering items is central to the process. However, with a slightly different technique for hiding items we can give the user the power to temporarily make an item disappear from the world for upcoming inquiries. Here are the hide options we can support.

```c++
namespace DancingLinks {
bool hideItem(PokemonLinks& dlx, const TypeEncoding& toHide);
bool hideItem(PokemonLinks& dlx, const std::vector<TypeEncoding>& toHide);
bool hideItem(PokemonLinks& dlx, const std::vector<TypeEncoding>& toHide,
              std::vector<TypeEncoding>& failedToHide);
void hideItemsExcept(PokemonLinks& dlx, const std::set<TypeEncoding>& toKeep);
}
```

For the why behind these runtime guarantees, please see the code, but here is what these operations offer.

- `hideItem` - It costs O(lgN) to find one item and a simple O(1) operation to hide it. The other two options add an O(N) operation to iterate through all requested items. The last overload can report any items that could not be hidden because they were hidden or did not exist in the links. 
- `hideItemsExcept` - We must look at all items, however thanks to Knuth's algorithm the number of items we must examine shrinks if some items are already hidden. It costs O(NlgK) where N is not-hidden items and K is the size of the set of items to keep.  

### Unhiding Items

The only space complexity cost we incur from hiding an item is that we must remember the order in which items were hidden. If you want to undo the hiding you must do so in precisely the reverse order. While the order does not matter for my implementation, if you had appearances of items across options as nodes with `left` and `right` pointers, the order that you spliced them out of options would be important.

To track the order, I use a stack and offer the user stack-like operations that limit how they interact with hidden items.

```c++
namespace DancingLinks {
int numHidItems(const PokemonLinks& dlx);
TypeEncoding peekHidItem(const PokemonLinks& dlx);
void popHidItem(PokemonLinks& dlx);
bool hidItemsEmpty(const PokemonLinks& dlx);
std::vector<TypeEncoding> hiddenItems(const PokemonLinks& dlx);
void resetItems(PokemonLinks& dlx);
}
```

Here are the guarantees I can offer for these operations.

- `numHidItems`/`hidItemsEmpty`/`peekHidItems` - These are your standard top of the stack operations offering O(1) runtime. Just as with a normal stack you should not try to peek or pop an empty stack.
- `hidItems` - I do provide the additional functionality of viewing the hidden stack for clarity. O(N). 
- `popHidItem` - Luckily, because of some internal implementation choices, unhiding an item is an O(1) operation. If you were using `left/right` fields for appearances of items in the options you would need to restore every appearance of those items. But with the tagging technique I use, this is not required in my implementation. This does incur a slight time cost when running a query on the PokemonLinks object if the user has hidden items, but due to the simple nature of traversing an array with indices, and the sparse nature of most matrices, this is a small cost.
- `resetItems` - This is an O(H) operation where H is hidden items.

### Hiding Options

We will also use a stack to manage hidden options. Here, however, the stack is required regardless of the implementation technique. We will be splicing options out of the links entirely, requiring that we undo the operation in the reverse order.

```c++
namespace DancingLinks {
bool hideOption(PokemonLinks& dlx, const TypeEncoding& toHide);
bool hideOption(PokemonLinks& dlx, const std::vector<TypeEncoding>& toHide);
bool hideOption(PokemonLinks& dlx, const std::vector<TypeEncoding>& toHide,
                std::vector<TypeEncoding>& failedToHide);
void hideOptionsExcept(PokemonLinks& dlx, const std::set<TypeEncoding>& toKeep);
}
```

Here are the runtime guarantees these operations offer.

- `hideOption` - It costs O(lgN) to find an option and O(I) to hide it, where I is the number of items in an option. The vector version is O(HIlgN) where H is the number of options to hide, I is the number of items for each option, and N is all options. We can also report back options we could not hide with the last overload. We cannot hide hidden options or options that don't exist.
- `hideOptionsExcept` - This operation will cost O(NlgKI) where N is the number of options, K is the size of the set of items to keep, and I is the number of items in an option.

### Unhiding Options

Here are the same stack utilities we offer for the option version of these operations.

```c++
namespace DancingLinks {
int numHidOptions(const PokemonLinks& dlx);
TypeEncoding peekHidOption(const PokemonLinks& dlx);
void popHidOption(PokemonLinks& dlx);
bool hidOptionsEmpty(const PokemonLinks& dlx);
std::vector<TypeEncoding> hidOptions(const PokemonLinks& dlx);
void resetOptions(PokemonLinks& dlx);
}
```

- `numHidOptions`/`peekHidOption`/`hidOptionsEmpty` - standard O(1) stack operations.
- `popHidOption` - O(I) where I is the number of items in an option.
- `resetOptions` - O(HI) where H is the number of hidden items, and I is the number of items in an option. Usually, we are dealing with sparse matrices, so I hopefully remains low.

### Other Operations

With the hiding and unhiding logic in place you now have a complete set of operations you can use on an in-place data structure that can alter its state and restore the original state when required. Here are the other operations we can use.

```c++
namespace DancingLinks {
std::set<RankedSet<TypeEncoding>> solveExactCover(PokemonLinks& dlx, int choiceLimit);
std::set<RankedSet<TypeEncoding>> solveOverlappingCover(PokemonLinks& dlx, int choiceLimit);
std::vector<TypeEncoding> items(const PokemonLinks& dlx);
int numItems(const PokemonLinks& dlx);
std::vector<TypeEncoding> options(const PokemonLinks& dlx);
int numOptions(const PokemonLinks& dlx);
PokemonLinks::CoverageType coverageType(const PokemonLinks& dlx);
}
```

We are now able to solve cover problems on a PokemonLinks object that is in a user-defined, altered state. The user can modify the structure as much as they would like and restore it to its original state with minimal internal maintenance of the object required. With the decent runtime guarantees we can offer with this data structure, the memory efficiency, lack of copies, and flexible state, I think there is a strong case to be made for a class implementation of Dancing Links.

Treating the PokemonLinks as an alterable object with a prolonged lifetime was useful in the GUI program you can use in this repository. For each Pokémon map I load in, I only load two PokemonLinks objects, one for ATTACK and one for DEFENSE. As the user asks for solutions to only certain sets of gyms, we simply hide the items the user is not interested in and restore them after every query. I have not yet found a use case for hiding options but this project could continue to grow as I try out different techniques.

### Pokémon Planning Usage Instructions

I have created a small testing ground for the Pokémon Cover Problem. I adapted a graph drawing application written by Keith Schwarz and Stanford course staff to allow you to explore various Pokémon maps. The maps are divided by Pokémon Generation. For example, the Kanto map is based around the attack and defense types available in Generation I of Pokémon. I included a Generation V and Generation IX map as well with more types available as you progress through generations.

You can solve the maps entirely for exact and overlapping cover problems or you can select specific gyms. In Pokémon, you progress through the game by defeating 8 gym leaders and then a final group called the Elite 4 (along with one last champion of that league). You can select any combination of gyms to defend against or attack and the cover problem will be adapted to the types in those locations. Interesting results can arise as you plan out your type advantages!

1. Open the project in Qt Creator with the correct Stanford C++ library installed. (See the [Build Note](#build-note)).
2. Build and run the project.
3. Select the `Pokémon Planning` option from the top menu.
4. Solve for every possible type you can encounter on a map with the cover buttons.
5. Select only specific gyms that you would like to cover with the `G1`-`E4` buttons.
6. Clear all selections at any time with the `CL` button.

I find it interesting that only Generation IX, the `Paldea.dst` map, has an exact cover for all possible types you will encounter in that generation. I am no expert on game design, but perhaps that communicates the variety and balance that Game Freak has achieved in their later games. However, looking at smaller subsets of gyms in the other maps can still be plenty of fun!

## Citations

This project grew more than I thought it would. I was able to bring in some great tools to help me explore these algorithms. So, it is important to note what I am responsible for in this repository and what I am not. The code that I wrote is contained in the following files.

- `PokemonLinks.h`
- `PokemonLinks.cpp`
- `PokemonParser.h`
- `PokemonParser.cpp`
- `RankedSet.h`
- `Resistance.h`
- `Resistance.cpp`

As mentioned in the intro, the core ideas of Algorithm X via Dancing Links belongs to Knuth, I just implemented it a few different ways.

Any other code was rearranged or modified to fit the needs of this project. Stanford Course staff is responsible for writing the GUI library that I used to display the algorithm. Keith Schwarz and Stanford course staff is responsible for writing the graph drawing algorithm that makes displaying `.dst` files so easy.

I made the most significant modifications to the following file that had the graph drawing utility already completed as mentioned above.

- `PokemonGUI.cpp`

I added the necessary adjustments to the GUI file to bring in the `json` data that I put together from the following websites.

For the `all-types.json` file, I got the information on type interactions from the following website.

- https://pokemondb.net/type/dual

For the `all-maps.json` file, I got the information on gyms and the attack and defensive types present from the following website.

- https://serebii.net

## Next Steps

I would like to visualize the cover problems in greater detail on a website and will try to put together a graph cover visualizer soon. Thanks for reading!
