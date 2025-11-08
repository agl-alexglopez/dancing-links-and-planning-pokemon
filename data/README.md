# Resource Files

This folder contains all the `json` data I have put together for solving the type coverage problems.

## Maps

The region map information is stored in the `data/maps/` folder.

The `json` files contain the generation tag, region name, and all mainline gyms or challenges necessary to beat the game. This usually consists of eight gyms followed by the elite four but some games changed the formula.

Each location has its name as the key and location information as its value. Information fields are as follows.

- A three letter code name for the location, similar to an airport. The abbreviations have no strict rule, they simply try to sound like the location or communicate its unique letters.
- A point array of two integers representing the `[x, y]` coordinates in abstract space this city occupies in relation to others. These are scaled appropriately by the drawing gui code.
- An edge list of city names to which this city is connected.
- An attack list of all the attack types present if this is a gym or elite four city.
- A defense list of all the defense types present if this is a gym or elite four city.

For now, I only include locations with gym leaders or challenges necessary to progress the game. However, this makes the maps look weird due to missing some locations. The current architecture is well suited to adding locations with or without gym leaders. One could expand these maps to add locations with types for things like team rocket fights. Or one could add locations with special to catch. Finally, the GUI implementation could be easily adjusted to support locations with no types that simply act as turns in the road on the map. But that is too much work for now.

## Types

Type interaction information is stored in the `data/types/` folder.

The `json` files contain the most recent type interaction information I could get from Pokemon's website and fan sites. I have the resistances for every type, dual types included, from generation 1 to generation 9. My generation json files divide the generations based on which types existed when the games in that generation were made. For example, GameFreak decided to go back and alter many Pokemon by changing their type starting in generation 6. This was because they just added the Fairy type in generation 6 and wanted to rework some past Pokemon. However, I do not acknowledge these retroactive changes in earlier generations. So, no generation has any mention of the Fairy type until generation 6. The same goes for Steel and Dark. Generation 1 has no reference to either of those types.

The `types-introduced-by-gen.json` file is a helpful file I used to more accurately create the generation `json` files. It has the name of every type that exists as of Generation 9 of Pokemon and the first generation in which that type was introduced. Any retroactive changes to Pokemon that GameFreak performed are not counted in that generation. Consider the Fairy case, as described in the previous paragraph.

## Citations

For the `gen-X-types.json` file, I got the information on type interactions from the following website.

- [pokemondb.net](https://pokemondb.net)

For the `types-introduced-by-gen.json` file, I got the information on when every type was introduced from the following website. Please note I decided against their approach to counting types retroactively based on changes made in later generations by GameFreak.

- [bullpapedia.com](https://bulbapedia.bulbagarden.net/wiki/Type)

For the `all-maps.json` file, I got the information on gyms and the attack and defensive types present from the following website.

- [serebii.net](https://serebii.net)
