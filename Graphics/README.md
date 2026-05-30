# AI Combat Game



The game generates a random dungeon with 8 rooms connected by corridors. Two teams (blue and red) spawn on opposite sides of the map, each with 2 Fighters, 1 Medic, and 1 Supplier. Fighters are squares, Medics are cross-shaped, and Suppliers are triangles.



Fighters use an FSM to decide what to do each frame: look for enemies, engage in combat, or go find their Medic/Supplier when HP or ammo is low. Each fighter has random personality traits (aggression, risk aversion, etc.) that affect how they pick targets and routes. Combat uses line-of-sight checks so fighters can only shoot what they can actually see.



Medics and Suppliers don't fight. They go to depots (green + for medical, yellow A for ammo) to restock, then find fighters who need help. If an enemy shows up and there's no friendly fighter around, they run.



Each agent has a health bar above them (green to red). Fighters also have a cyan ammo bar. Suppliers have a cyan bar showing how much ammo stock they're carrying, and Medics have a green bar for their healing supply. The floor gets darker in areas where fighting happened recently (that's the safety map, agents try to avoid those spots).



The game ends when all fighters on a team die, or if everyone runs out of ammo. Right-click to restart.



Students - 

Victoria Branzburg 209463322

Maya Sobel 212258271

Amit Shahar 207669896

