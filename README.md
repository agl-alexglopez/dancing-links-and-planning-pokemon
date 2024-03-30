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
3. Run the Command Line Interface application `./build/rel/pokemon_cli`.

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
    ./build/rel/pokemon_cli G1 G2 G3 G4 data/dst/Gen-5-Unova2.dst
```

For what these types of cover problems mean, read the longer description below. A more robust and interesting graph cover visualizer is coming soon but is not complete yet. I find it interesting that only later generation maps have an exact cover for all possible types you will encounter in that generation. I am no expert on game design, but perhaps that communicates the variety and balance that Game Freak has achieved in their later games. However, looking at smaller subsets of gyms in the other maps can still be plenty of fun!

## Overview

In October of 2022, Donald Knuth released *The Art of Computer Programming: Volume 4b: Combinatorial Algorithms, Part 2*. In this work he revised his previous implementation of his Algorithm X via Dancing Links. His revision included changing the quadruple linked nodes that solved backtracking problems to an array of nodes. This is an interesting optimization. It provides good locality, ease of debugging, and memory safety if you use an array that does not need to be managed like a C++ vector. One of the points Knuth makes clear through his section on Dancing Links is that he wants to give people the tools they need to expand and try his backtracking strategies on many problems. If you want the original Algorithm, please read Knuth's work. Below, I will discuss how I applied or modified his strategies to fit a fun puzzle I have often considered for the game of Pokémon.

## Pokémon Type Coverage

Knuth brings up many puzzles in his work on Combinatorial Algorithms. He has word puzzles, number puzzles, visual *puzzle* puzzles. All of them got me thinking about more problems I could find that use his methods and I found one! The videogame Pokémon by developer Game Freak is part of a well established franchise that started in the 1990's. At their core, the Pokémon videogames are a game of rock-paper-scissors with a few dozen layers of complexity added on top. While I can't explain the entire ruleset of Pokémon here, I can explain the relevant parts to the problem I decided to solve.

In Pokémon, trainable animals/monsters battle on your behalf imbued with pseudo-elemental types like fire, water, flying, ground, and many others. There are 15 to 18 fundamental types that Pokémon can use depending on the generation of games you are considering. Because Pokémon has been around for so many years, they have made many changes to their games. The community that plays those games often divides the games by generations in which different game mechanics were introduced. Right now, there are nine generations. At the most abstract level you only need to consider how to attack other Pokémon with these fundamental types and how to defend against these attack types with your own Pokémon. The twist on defense is that your Pokémon can have up to two different types--such as Fire-Flying or Ice-Water--that determine how weak or resistant they are to these fundamental attack types. While these fundamental types could be combined to form 306 dual types, counting dual types in different orders as different types, not to mention the additional 15 to 18 single types, the developers of this game have not done so yet. Depending on the generation, there are far fewer types than this, but there are still many. The most recent release is up to 162 unique Pokémon types.

So, there are two cover problems hiding in the complexities of the Pokémon games: one for defense and one for attack. The two essential cover questions we can ask are as follows.

- **Which teams of at most 6 Pokémon--the most you can carry with you at once--give me resistance to every attack type I will encounter? If you consider an entire game, you would want to know the answer to that question for every attack type in the game. If you are considering just the attacks you will face in some portions of the game, then the range of attack type shrinks but the question remains the same.**
- **Which attack types can I choose to be effective against every defensive type I will encounter in the game? Again, considering the entire game versus some smaller sections will change the range of defensive types you will see, but the question remains the same.**

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
struct typeName 
{
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

The `TypeEncoding` is a new addition to this project. Previously, this implementation produced solutions in string format. This means all input and output for the Pokémon types came in the form of `std::string`. Normally, this would be fine, but the exact cover problem as I have set it up communicates with sets and maps which means behind the scenes the algorithm is performing thousands if not millions of string comparisons of varying lengths. We can reduce all of these comparisons that are happening to a single comparison between two numbers. This will make moving data and some of the algorithms faster while vastly reducing the memory footprint. We simply encode all types into this format and get the added benefit of a trivially copyable class.

```c++
class Type_encoding {

  public:
    Type_encoding() = default;
    // If encoding cannot be found encoding_ is set the falsey value 0.
    Type_encoding(std::string_view type);
    [[nodiscard]] uint32_t encoding() const;
    [[nodiscard]] std::pair<std::string_view, std::string_view>
    decode_type() const;
    [[nodiscard]] std::pair<uint64_t, std::optional<uint64_t>>
    decode_indices() const;
    [[nodiscard]] std::string to_string() const;
    [[nodiscard]] static std::span<const std::string_view> type_table();

    bool operator==(Type_encoding rhs) const;
    std::strong_ordering operator<=>(Type_encoding rhs) const;

  private:
    uint32_t encoding_;
    static uint64_t type_bit_index(std::string_view type);
    // Any and all Type_encodings will have one global string_view of the type
    // strings for decoding.
    static constexpr std::array<std::string_view, 18> type_encoding_table = {
        // lexicographicly organized table. 17th index is the highest
        // lexicographic value "Water."
        "Bug",    "Dark",   "Dragon",  "Electric", "Fairy",  "Fighting",
        "Fire",   "Flying", "Ghost",   "Grass",    "Ground", "Ice",
        "Normal", "Poison", "Psychic", "Rock",     "Steel",  "Water",
    };
};
```

We place every Pokémon type in this `uint32_t` such that the 0th index bit is Water and the 17th index bit is Bug. In its binary representation it looks like this.

![type-encoding](/images/type-encoding.png)

We now have the ability to turn specific bits on in this type to represent the type we are working with. Turn on one bit for single types and two bits for dual types. For example, "Bug-Water" would be the following.

![type-encoding-bug-water](/images/type-encoding-bug-water.png)

The final challenge for this strategy is making sure that we can use this type as you would a normal string, in a set or map for example. This means that the `TypeEncoding` must behave the same as its string representation in terms of lexicographic sorting. To achieve this we must take a counterintuitive approach; the bits must be aligned in the HIGHEST order bit position according to LOWEST lexicographical order of the string they represent. 

So above, "Bug" will always be first in terms of lexicographical ordering among these strings. It's bit must be in the highest order position we have available. We need to ensure that any type that starts with "Bug" will always be less than any other possible type combination that does not start with bug, and so on for the next string in ascending order. The easy way to do this is to ensure that any `TypeEncoding` that contains the "Bug" bit is larger than one that does not (counterintuitive right?). That is why you see the `operator<` overload for this type flipped. Once we have consistent logic for this type we just need to flip the meaning of a larger value so it behaves like a normal string. If this is confusing consider the problems you would run into if you flipped this bit array, placing "Bug" at the zero index. The type "Bug-Water" would be a larger numeric value than "Ghost-Grass," but "Bug-Water" should be sorted first. It becomes a mess! Using the doubling nature of base 2 bits, we can achieve the consistency we want, we just need to take an odd approach. There are other bit tricks and strategies I use to implement this type efficiently but you can explore those in the code if you wish. In fact, when compared to a traditional `unordered_map` that would precompute and store all possible encodings and decoding strings, my implementation is slightly slower in the encoding phase but twice as fast in the decoding phase. See the **[`type_encoding.cc`](/src/type_encoding.cc)** file and the runtime tests in **[`tests.cc`](/tests/tests.cc)**.

The final optimization involves the newish type `std::string_view`. I try to avoid creating heap allocated strings whenever necessary. Because we must have a table with type names to refer to for the `TypeEncoding` I just point to those strings to display type information when decoding a `TypeEncoding`. I added this constraint to learn more about how to properly use `std::string_view` and I like the design decisions that followed. See the code for more.

Here is the type that I use within the dancing links array.

```c++
struct Poke_link
{
    int32_t top_or_len;
    uint64_t up;
    uint64_t down;
    Multiplier multiplier; // x0.0, x0.25, x0.5, x1.0, x2, or x4
    int tag; // We use this to efficiently generate overlapping sets.
};
```

The Multiplier is a simple `enum`.

```c++
enum Multiplier
{
    emp = 0,
    imm,
    f14, // x0.25 damage aka the fraction 1/4
    f12, // x0.5 damage aka the fraction 1/2
    nrm, // normal
    dbl, // double or 2x damage
    qdr  // quadruple or 4x damage.
};
```

We then place all of this in one array. Here is a illustration of these links as they exist in memory.

![pokelinks-illustrated](/images/pokelinks-illustrated.png)

There is also one node at the end of this array to know we are at the end and that our last option is complete. Finally, to help keep track of the options that we choose to cover items, there is another array consisting only of names of the options. These also have an index for the option in the dancing links data structure for some class methods I cover in the next section.

```c++
struct Encoding_index
{
    Type_encoding name;
    uint64_t index;
};
const std::vector<Pokemon_links::Encoding_index> option_table
    = {{{""}, 0},
       {{"Dragon"}, 7},
       {{"Electric"}, 12},
       {{"Ghost"}, 14},
       {{"Ice"}, 16}};
```

The spacer nodes in the dancing links array have a negative `topOrLen` field that correspond to the index in this options array and this array has the index of that option. There are other subtleties to the implementation that I must consider, especially how to use the depth tag to produce Overlapping Type Coverages, but that can all be gleaned from the code itself.

## Bonus: Justifying a PokemonLinks Class

I wrote this implementation as a class from the beginning. However, early on, I had doubts that this algorithm had any state or attributes that would help justify a class. I could have just as easily wrote a procedural algorithm in a functional style where I take input and provide solutions in my output. However, building the dancing links data structure is non-trivial, so building this structure on every inquiry seemed slow. With a few adjustments, invariants, and runtime guarantees I think there is a case to be made for the PokemonLinks class, and more generally Dancing Links classes for more general algorithms. With minor changes, all the techniques I discuss could be applied to any Dancing Links solver.

### Invariants

In order for the following techniques to work we must maintain some invariants in the Dancing Links internals.

1. Maintain the identifiers for items--for me this is the `TypeEncoding` name of the item--in a sorted vector. This is required by Knuth's algorithms already but I am adding that the items must be sorted for some later techniques.
2. Maintain the identifiers for the options--for me this is the `TypeEncoding` name of the option--in a sorted vector. This is not required by Knuth's algorithm, but my options have meaningful names that may be different than the item names so I must make this vector. This will also help with later techniques.
3. Do not support insertion or deletion of items or options from any data structure. Inserting an item or option may be possible but it would require substantial modification to the dancing links array that would either be slow or require more space to store the required information. We can support hiding items and options but not deleting them completely. All operations will be in-place.

### Searching

If a user wants to membership test an item or option we can make some guarantees because we maintain that all items and options are in sorted vectors.

```c++
namespace DancingLinks{
[[nodiscard]] bool has_item(Type_encoding item) const;
[[nodiscard]] bool has_option(Type_encoding option) const;
}
```

- `has_item` - Finding an item is O(lgN) where N is the number of all items. You cannot find a hidden item.
- `has_option` - Finding an option is O(lgN) where N is the number of all options. You cannot find a hidden option.

### Hiding Items

As a part of Algorithm X via Dancing Links, covering items is central to the process. However, with a slightly different technique for hiding items we can give the user the power to temporarily make an item disappear from the world for upcoming inquiries. Here are the hide options we can support.

```c++
namespace DancingLinks {
void hide_item(uint64_t header_index);
bool hide_items(Pokemon_links &dlx,
                const std::vector<Type_encoding> &to_hide);
bool hide_items(Pokemon_links &dlx,
                const std::vector<Type_encoding> &to_hide,
                std::vector<Type_encoding> &failed_to_hide);
void hide_items_except(Pokemon_links &dlx,
                       const std::set<Type_encoding> &to_keep);
}
```

For the why behind these runtime guarantees, please see the code, but here is what these operations offer.

- `hide_item` - It costs O(lgN) to find one item and a simple O(1) operation to hide it. The other two options add an O(N) operation to iterate through all requested items. The last overload can report any items that could not be hidden because they were hidden or did not exist in the links. 
- `hide_items_except` - We must look at all items, however thanks to Knuth's algorithm the number of items we must examine shrinks if some items are already hidden. It costs O(NlgK) where N is not-hidden items and K is the size of the set of items to keep.  

### Unhiding Items

The only space complexity cost we incur from hiding an item is that we must remember the order in which items were hidden. If you want to undo the hiding you must do so in precisely the reverse order. While the order does not matter for my implementation, if you had appearances of items across options as nodes with `left` and `right` pointers, the order that you spliced them out of options would be important.

To track the order, I use a stack and offer the user stack-like operations that limit how they interact with hidden items.

```c++
namespace DancingLinks {
uint64_t num_hid_items(const Pokemon_links &dlx);
Type_encoding peek_hid_item(const Pokemon_links &dlx);
void pop_hid_item(Pokemon_links &dlx);
bool hid_items_empty(const Pokemon_links &dlx);
std::vector<Type_encoding> hid_items(const Pokemon_links &dlx);
void reset_items(Pokemon_links &dlx);
}
```

Here are the guarantees I can offer for these operations.

- `num_hid_items`/`hid_items_empty`/`peek_hid_items` - These are your standard top of the stack operations offering O(1) runtime. Just as with a normal stack you should not try to peek or pop an empty stack.
- `hid_items` - I do provide the additional functionality of viewing the hidden stack for clarity. O(N). 
- `pop_hid_item` - Luckily, because of some internal implementation choices, unhiding an item is an O(1) operation. If you were using `left/right` fields for appearances of items in the options you would need to restore every appearance of those items. But with the tagging technique I use, this is not required in my implementation. This does incur a slight time cost when running a query on the PokemonLinks object if the user has hidden items, but due to the simple nature of traversing an array with indices, and the sparse nature of most matrices, this is a small cost.
- `reset_items` - This is an O(H) operation where H is hidden items.

### Hiding Options

We will also use a stack to manage hidden options. Here, however, the stack is required regardless of the implementation technique. We will be splicing options out of the links entirely, requiring that we undo the operation in the reverse order.

```c++
namespace DancingLinks {
bool hide_option(Pokemon_links &dlx, Type_encoding to_hide);
bool hide_options(Pokemon_links &dlx, 
                  const std::vector<Type_encoding> &to_hide);
bool hide_options(Pokemon_links &dlx, 
                  const std::vector<Type_encoding> &to_hide,
                  std::vector<Type_encoding> &failed_to_hide);
void hide_options_except(Pokemon_links &dlx, 
                         const std::set<Type_encoding> &to_keep);
}
```

Here are the runtime guarantees these operations offer.

- `hide_option` - It costs O(lgN) to find an option and O(I) to hide it, where I is the number of items in an option. The vector version is O(HIlgN) where H is the number of options to hide, I is the number of items for each option, and N is all options. We can also report back options we could not hide with the last overload. We cannot hide hidden options or options that don't exist.
- `hide_options_except` - This operation will cost O(NlgKI) where N is the number of options, K is the size of the set of items to keep, and I is the number of items in an option.

### Unhiding Options

Here are the same stack utilities we offer for the option version of these operations.

```c++
namespace DancingLinks {
uint64_t num_hid_options(const Pokemon_links &dlx);
Type_encoding peek_hid_option(const Pokemon_links &dlx);
void pop_hid_option(Pokemon_links &dlx);
bool hid_options_empty(const Pokemon_links &dlx);
std::vector<Type_encoding> hid_options(const Pokemon_links &dlx);
void reset_options(Pokemon_links &dlx);
}
```

- `num_hid_options`/`peek_hid_option`/`hid_options_empty` - standard O(1) stack operations.
- `pop_hid_option` - O(I) where I is the number of items in an option.
- `reset_options` - O(HI) where H is the number of hidden items, and I is the number of items in an option. Usually, we are dealing with sparse matrices, so I hopefully remains low.

### Other Operations

With the hiding and unhiding logic in place you now have a complete set of operations you can use on an in-place data structure that can alter its state and restore the original state when required. Here are the other operations we can use.

```c++
namespace DancingLinks {
std::set<Ranked_set<Type_encoding>> exact_cover_functional(Pokemon_links &dlx, 
                                                           int choice_limit);
std::set<Ranked_set<Type_encoding>> exact_cover_stack(Pokemon_links &dlx, 
                                                      int choice_limit);
std::set<Ranked_set<Type_encoding>> 
overlapping_cover_functional(Pokemon_links &dlx, int choice_limit);
std::set<Ranked_set<Type_encoding>> overlapping_cover_stack(Pokemon_links &dlx, 
                                                            int choice_limit);
}
```

We are now able to solve cover problems on a PokemonLinks object that is in a user-defined, altered state. The user can modify the structure as much as they would like and restore it to its original state with minimal internal maintenance of the object required. With the decent runtime guarantees we can offer with this data structure, the memory efficiency, lack of copies, and flexible state, I think there is a strong case to be made for a class implementation of Dancing Links.

Treating the PokemonLinks as an alterable object with a prolonged lifetime can be useful in GUI and CLI programs in this repo. For each Pokémon map I load in, I only load two PokemonLinks objects, one for ATTACK and one for DEFENSE. As the user asks for solutions to only certain sets of gyms, we simply hide the items the user is not interested in and restore them after every query. I have not yet found a use case for hiding options but this project could continue to grow as I try out different techniques.

## Citations

This project grew more than I thought it would. I was able to bring in some great tools to help me explore these algorithms. So, it is important to note what I am responsible for in this repository and what I am not. The code that I wrote is contained in the following files.

- `type_encoding.cc`
- `pokemon_links.cc`
- `pokemon_parser.cc`
- `ranked_set.cc`
- `resistance.cc`
- `pokemon_cli.cc`

As mentioned in the intro, the core ideas of Algorithm X via Dancing Links belongs to Knuth, I just implemented it a few different ways.

Any other code was rearranged or modified to fit the needs of this project. Stanford Course staff and Keith Schwarz is responsible for writing the graph drawing algorithm that makes working with `.dst` files so easy.

For the `all-types.json` file, I got the information on type interactions from the following website.

- https://pokemondb.net/type/dual

For the `all-maps.json` file, I got the information on gyms and the attack and defensive types present from the following website.

- https://serebii.net

## Next Steps

The graph cover visualizer is a work in progress. Thanks for reading!
