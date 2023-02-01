# Resource Files

This folder contains all the `json` data I have put together for solving the type coverage problems and the `dst` files necessary to draw maps.

## JSON Files

The `json` files contain the most recent type interaction information I could get from Pokemon's website and fan sites. I have the resistances for every type, dual types included, from generation 1 to generation 9. My generation json files divide the generations based on which types existed when the games in that generation were made. For example, GameFreak decided to go back and alter many Pokemon by changing their type starting in generation 6. This was because they just added the Fairy type in generation 6 and wanted to rework some past pokemon. However, I do not acknowledge these changes in the generation in which the type is introduced. So, no generation has any mention of the Fairy type until generation 6. The same goes for Steel and Dark. Generation 1 has no reference to either of those types.

The `types-introduced-by-gen.json` file is a helpful file I used to more accurately create the generation `json` files. It has the name of every type that exists as of Generation 9 of pokemon and the first generation in which that type was introduced. Any retroactive changes to Pokemon that GameFreak performed are not counted in that generation retroactively. Consider the Fairy case, as described in the previous paragraph.

The `all-maps.json` file accompanies all `.dst` files that are added to the project. It contains the name of the `.dst` file and the eight gyms plus elite four that goes along with that map. It contains the attack and defensive types that can be found in each location. If you add a new map, complete its gym typing information in the `all-maps.json` file.

## DST Files

The `.dst` files are used to draw maps to the screen. They use a logical layout system written by Keith Schwarz and Stanford course staff. This means you can draw an imaginary grid of any proportion over your desired map and enter those points into the file. For the purposes of this project I require that all `.dst` files use the following format.

- Start every `.dst` file with a comment on the first line that specifies the generation to which this map belongs. This will determine the types that exist.
  - For example.
  - `# 1`
  - This means you want the map you are using to use only types available in generation 1.
  - The first line must always be a generation specifier or a comment of some sort.
- If no generation is given it will default to the most recent generation 9.
- Always leave a comment on the first line, either the generation or a simple explanation of the map if you want the most recent generation of types.
- Gyms are numbered 1-8 with the title GNUMBER (COORDINATES): CONNECTIONS
  - For example.
    - G1 (2, 5): G2, G7
  - The gym name comes first, always a single G followed by the number.
  - The logical coordinates follow.
  - Finally, to the right of the colon place any other gyms that directly connect to this one via roads on the map.
  - The elite four is always given the abbreviation E4.

See the examples in the .dst files for more details.

## Citations

For the `all-types.json` file, I got the information on type interactions from the following website.

- https://pokemondb.net

For the `all-maps.json` file, I got the information on gyms and the attack and defensive types present from the following website.

- https://serebii.net
