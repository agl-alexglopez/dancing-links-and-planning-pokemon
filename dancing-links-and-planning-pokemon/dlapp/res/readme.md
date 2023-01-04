# Resource Files

This folder contains all the `json` data I have put together for solving the type coverage problems and the `dst` files necessary to draw maps.

## JSON Files

The `json` files contain the most recent type interaction information I could get from Pokemon's website. I have all of the types, single and dual-types, that are present up to generation 9 and the most recent ruleset for type resistances. I then filter out types that were not present in older generations in the parser application. This might be better if I had different typing `.json` files for each generation, but I have not yet made that change.

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
