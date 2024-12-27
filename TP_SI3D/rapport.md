# Rapport du projet OpenGL

## Ressources

[Sources du projet](https://github.com/Hypooxanthine/M2-SI3D-Rasterization/tree/master/TP_SI3D)

[D'autres fichiers source que j'ai créés, communs à tous les projets SI3D / CG](https://github.com/Hypooxanthine/M2-SI3D-Rasterization/tree/master/src/MyThings)

Pour compiler et lancer, depuis le répertoire racine de gkit : 
```shell
premake4 gmake
./run.sh
```

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

J'utilise un SSBO qui va stocker la concaténation des matrices de modèle des cubes, pour chaque chunk et chaque type de bloc.

D'autre part, j'ai créé une classe MultiMesh qui se charge de la cohérence des données pour l'utilisation de glMultiDrawElementsIndirect. Dans cette classe, je vais avoir un buffer pour les vertice, un buffer pour les indices, et un buffer pour les commandes. Un vertex contient une position, une coordonnée de texture et une normale. Le multimesh permet d'ajouter séquentiellement plusieurs meshes aux deux premiers buffers, tout en conservant les tailles / les indices nécessaires dans les tableaux pour s'y retrouver avec les commandes. Un VAO est paramétré dans updateBuffers.

Ensuite, on peut créer ou modifier des commandes. Pour créer une commande par exemple, il faut un indice de mesh, combien d'instances doivent être dessinées, et quel est son baseInstance. Ce dernier paramètre est crucial car c'est lui qui permettra de se repérer dans le SSBO, dans le [vertex shader](https://github.com/Hypooxanthine/M2-SI3D-Rasterization/blob/master/data/shaders/TP_SI3D/CubeShadowBuilder.glsl).

Dans mon implémentation, j'ai choisi l'approche de ne pas supprimer une commande, mais plutôt de la modifier en fixant à zéro son nombre d'instances. Visuellement ça ne change rien, et ça me permet de ne pas recréer potentiellement à chaque frame tout le command buffer et de le renvoyer à la carte graphique. J'ai supposé que cela prendrait moins de temps avec l'implémentation que j'ai choisie.

## Optimisation : frustum culling

### Détails

Les algorithmes se trouvent dans [ce fichier](https://github.com/Hypooxanthine/M2-SI3D-Rasterization/blob/master/TP_SI3D/include/FrustumCulling.h).

Contrôles : la touche C permet d'activer ou de désactiver le frustum culling.

### Implémentation

En terme de structures de données, c'est assez léger : juste un AABB contenant un first et un second. Chaque chunk peut retourner son propre englobant et c'est ce qui permettra de faire le frustum culling pour tous les chunks.

Dans l'espace monde, on a un AABB et un frustum. On va extraire les 8 coins du frustum, et regarder si une des 6 faces du AABB peut entièrement séparer le AABB du frustum. Le test est rapide car on a simplement besoin de vérifier des inégalités sur chaque axe. Si une des faces du AABB peut entièrement séparer le AABB du frustum, alors on souhaitera ne pas afficher le chunk. Sinon, on passe en espace projectif :
- Le AABB devient le cube unitaire ([-1, 1]³)
- Le frustum de vue devient le aabb déformé par les matrices VP
On se trouve dans la même situation : un AABB contre 8 sommets. Donc on refait exactement le même test. Si aucun plan n'est trouvé, alors on affichera le chunk. Sinon, on le cachera (càd: on fixera son nombre d'instances à zéro).

## Ombrage

### Détails

J'ai dû créer un shader pour la [construction de la shadowmap](https://github.com/Hypooxanthine/M2-SI3D-Rasterization/blob/master/data/shaders/TP_SI3D/CubeShadowBuilder.glsl), et modifier le shader [affichant les cubes](https://github.com/Hypooxanthine/M2-SI3D-Rasterization/blob/master/data/shaders/TP_SI3D/Cube.glsl).

Contrôles : les touches A et E permettent de changer l'inclinaison de la lumière directionnelle.

### Implémentation

J'ai créé et utilisé les classes [FrameBuffer](https://github.com/Hypooxanthine/M2-SI3D-Rasterization/blob/master/src/MyThings/FrameBuffer.h) et [Texture2D](https://github.com/Hypooxanthine/M2-SI3D-Rasterization/blob/master/src/MyThings/Texture2D.h).

Côté application, tout est dans [la classe principale](https://github.com/Hypooxanthine/M2-SI3D-Rasterization/blob/master/TP_SI3D/src/TP_SI3D.cpp).

Il faut d'abord définir le volume "de vue" de la lumière. Ma lumière est directionnelle, donc j'utilise une projection orthographique. Je prends les dimensions max du terrain pour largeur, hauteur et profondeur. De cette manière, puisque je veux faire simplement tourner la lumière directionnelle autour de la scène (selon l'axe X uniquement), toute la scène devrait plus ou moins pouvoir être ombragée si nécessaire (les bords du terrains ont une hauteur assez faible, donc cela convient).

Pour la matrice de vue, elle change quand on appuie sur A ou E. C'est une rotation suivie d'une translation pour que le volume de lumière soit centré sur le terrain.

Dans le pipeline, d'abord je dessine la scène avec un simple shader qui a pour seul objectif de stocker la profondeur dans le framebuffer. Ensuite, une deuxième passe sur le framebuffer par défaut va permettre d'attribuer leur vraie couleur aux pixels. Je passe la carte de profondeurs de la première passe au shader de la seconde passe pour vérifier si un fragment est à l'ombre ou non.

C'est dans le [deuxième shader](https://github.com/Hypooxanthine/M2-SI3D-Rasterization/blob/master/data/shaders/TP_SI3D/Cube.glsl) que la couleur d'un fragment est déterminée. Si un fragment est exposé à la lumière, je fais un simple calcul de lumière diffuse où la couleur de base est tirée de la feuille de sprites. S'il est à l'ombre, je divise par 10 son intensité.

Pour déterminer qu'un fragment est à l'ombre, je considère le fragment dans le repère projectif de la lumière. Je normalise les valeurs pour passer vers [0, 1]³, et les coordonnées xy du fragment dans cet espace sont les coordonnées de textures qui m'intéressent pour ce fragment. Mais comme le pixel de la shadowmap a été calculé pour un fragment différent (même si très proche dans l'espace), il y a beaucoup de bruit, que j'ai réglé par l'ajout d'un biais de 0.0025, trouvé empiriquement pour obtenir un résultat visuellement agréable sur cette scène.