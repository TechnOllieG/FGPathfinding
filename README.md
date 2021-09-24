# FGPathfinding
Welcome to my pathfinding project Olof! Here are some brief instructions on how to navigate it:

1. L_Main is the level where the path is visualized on the grid itself. Start the game, click once on the grid to select a start point (should turn green) and a second time to select a end point (should turn orange), the path will then be highlighted in yellow. To visualize iteration by iteration of the algorithm, go into the FGFlyingPlayer blueprint and turn on SlowAStarWithVisualization and change the delay per iteration if you want. Then select start/end point again. Now the nodes in the closed list will be gray and nodes in the open list will be blue.
2. L_EnemyTest is a level with a simple enemy that tries to pathfind wherever the player goes. Each node in the path is visualized using System::DrawDebugSphere.

There currently seems to be a bug when opening the main level if it wasn't open at the start where an exception is thrown. Just press continue in your IDE and it will work fine (haven't had time to look into why it happens).
