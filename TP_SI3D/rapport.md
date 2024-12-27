# Rapport du projet OpenGL

## Ressources

[Sources du projet](https://github.com/Hypooxanthine/M2-SI3D-Rasterization/tree/master/TP_SI3D)

[D'autres fichiers source que j'ai créés, communs à tous les projets SI3D / CG](https://github.com/Hypooxanthine/M2-SI3D-Rasterization/tree/master/src/MyThings)

## Quelques classes d'abstraction

J'ai créé quelques classes pour me faciliter la tâche, qui servent à abstraire certains objets OpenGL en version "POO". Je trouvais ça plus lisible et surtout je n'avais plus besoin de m'occuper de la libération des ressources (destructeurs des classes).
Parmis les plus utiles dans ce TP :
- [Buffers](https://github.com/Hypooxanthine/M2-SI3D-Rasterization/blob/master/src/MyThings/Buffer.h) ([VBO](https://github.com/Hypooxanthine/M2-SI3D-Rasterization/blob/master/src/MyThings/VertexBuffer.h), [EBO](https://github.com/Hypooxanthine/M2-SI3D-Rasterization/blob/master/src/MyThings/IndexBuffer.h))
- [Vertex arrays](https://github.com/Hypooxanthine/M2-SI3D-Rasterization/blob/master/src/MyThings/VertexArray.h)
- [Multimesh](https://github.com/Hypooxanthine/M2-SI3D-Rasterization/blob/master/src/MyThings/MultiMesh.h) et son [draw indirect buffer](https://github.com/Hypooxanthine/M2-SI3D-Rasterization/blob/master/src/MyThings/DrawIndirectBO.h)
- [Frame buffer](https://github.com/Hypooxanthine/M2-SI3D-Rasterization/blob/master/src/MyThings/FrameBuffer.h)

## Heighmap vers géométrie

### Division du monde en chunks

Une classe [ChunkManager](https://github.com/Hypooxanthine/M2-SI3D-Rasterization/blob/master/TP_SI3D/include/ChunkManager.h) gère les chunks : c'est elle qui les contient, mais elle garde aussi une trace, pour chaque chunk, de l'id de sa première instance ainsi que le nombre total d'instances de tous les chunks réunis.

### Attribution d'une position et d'un type de cube pour chaque pixel

C'est la classe [chunk](https://github.com/Hypooxanthine/M2-SI3D-Rasterization/blob/master/TP_SI3D/include/Chunk.h) qui va échantillonner la heightmap, uniquement dans son domaine propre (paramètres startX, startY, chunkWidth). Pour pouvoir correctement échantillonner, on donne aussi les paramètres totalX et totalY.
Je stocke dans une map ayant pour clé l'indice des meshes, et pour valeur un vecteur de Transforms. C'est la hauteur qui déterminera le type de cube (mesh id) qui sera utilisé en ce point.

### Transfert des données à la carte graphique

J'utilise un SSBO qui va stocker la concaténation, pour chaque chunk et chaque type de bloc, des matrices de modèle des cubes.

D'autre part, j'ai créé une classe MultiMesh qui se charge de la cohérence des données pour l'utilisation de glMultiDrawElementsIndirect. Dans cette classe, je vais avoir un buffer pour les vertice, un buffer pour les indices, et un buffer pour les commandes. Le multimesh permet d'ajouter séquentiellement plusieurs meshes aux deux premiers buffers, tout en conservant les tailles / les indices nécessaires dans les tableaux pour s'y retrouver avec les commandes. Un VAO est paramétré dans updateBuffers.

Ensuite, on peut créer ou modifier des commandes. Pour créer une commande par exemple, il faut un indice de mesh, combien d'instances doivent être dessinées, et quel est son baseInstance. Ce dernier paramètre est crucial car c'est lui qui permettra de se repérer dans le SSBO, dans le vertex shader.

Dans mon implémentation, j'ai choisi l'approche de ne pas supprimer une commande, mais plutôt de la modifier en fixant à zéro son nombre d'instances. Visuellement ça ne change rien, et ça me permet de ne pas recréer potentiellement à chaque frame tout le command buffer et de le renvoyer à la carte graphique. J'ai supposé que cela prenait trop de temps.

## Optimisation : frustum culling

Les algorithmes se trouvent dans [ce fichier](https://github.com/Hypooxanthine/M2-SI3D-Rasterization/blob/master/TP_SI3D/include/FrustumCulling.h).

J'ai (re)créé quelques structures de données, notamment le Vec4 (le vec4 de gkit ne fournit par les opérateurs de base), le plan, le frustum et le AABB. Un AABB pourrait être converti en Frustum sans perte d'information, mais l'inverse est faux. Je me suis servi du fait qu'on ait un AABB en world space pour simplifier les calculs.

En effet, dans le world space, je vais vérifier s'il existe un plan du AABB qui sépare tous les points du frustum.

Normalement, j'aurais dû vérifier dans l'autre sens : dans l'espace projectif, le frustum de camera devient un AABB (de dimensions [-1, 1]^3), et refaire les mêmes calculs.