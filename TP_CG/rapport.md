# Rapport du projet Rendu différé et interpolé

## Ressources

[Sources du projet](https://github.com/Hypooxanthine/M2-SI3D-Rasterization/tree/master/TP_CG)

[D'autres fichiers source que j'ai créés, communs à tous les projets SI3D / CG](https://github.com/Hypooxanthine/M2-SI3D-Rasterization/tree/master/src/MyThings)

Pour compiler et lancer, depuis le répertoire racine de gkit : 
```shell
premake4 gmake
./run3.sh
```

## Contrôles

- La touche T permet d'alterner entre calcul de tous les pixels et interpolation de certains pixels (selon la méthode décrite dans le sujet du TP).
- La touche D permet d'alterner entre l'affichage de l'objet et l'affichage des pixels calculés (rouges) / interpolés (bleus). Ne fonctionne qu'en mode interpolation.
- Les touches M(oins) et P(lus) permettent de diminuer et d'augmenter le seuil de variance pour l'interpolation. En mode D, on peut voir les zones de grande variation s'allumer progressivement en appuyant sur M.

## Abstraction

Comme pour mon projet de OpenGL (dans le même dépôt Git), j'ai utilisé certaines de mes classes d'abstraction, notamment pour les [compute shaders](https://github.com/Hypooxanthine/M2-SI3D-Rasterization/blob/master/src/MyThings/ComputeShader.h) et les [frame buffers](https://github.com/Hypooxanthine/M2-SI3D-Rasterization/blob/master/src/MyThings/FrameBuffer.h). Le code spécifique au compute shader n'est pas extrêmement utile au final, le plus utile était pour ce qui touche aux uniforms et aux texture uniforms (hérité de la classe [Shader](https://github.com/Hypooxanthine/M2-SI3D-Rasterization/blob/master/src/MyThings/Shader.h)).

## Construction du GBuffer

### Rappel du cours

Le GBuffer est un lieu de stockage des informations minimales nécessaires au calcul de la couleur de chaque pixel. Pour chaque pixel, on va avoir besoin de stocker différentes informations relatives au fragment dont le pixel est issu. Dans mon cas, je stocke :
- Un id de matériau (si j'ai une scène avec des objets dont je voudrais calculer une couleur selon différents modèles d'ombrage)
- La position du fragment en World Space
- La normale du fragment
- L'albedo (couleur de base) du fragment
- La valeur de metallique du fragment
- La valeur de diffusion du fragment
- La valeur de brillance du fragmentcomposante

Les dernières valeurs servent au calcul d'un simple Blinn-Phong.

Le GBuffer est matérialisé par une série de textures, qui est un stockage très commode quand on cherche à stocker des informations relatives aux pixels d'une image.

### Côté carte graphique

J'ai un [shader](https://github.com/Hypooxanthine/M2-SI3D-Rasterization/blob/master/TP_CG/shaders/gshader.glsl) spécialisé dans la construction du gbuffer.

Il est assez simple : le vertex shader envoie des informations "habituelles" à savoir, l'interpolation de la position et de la normale du fragment.

Le fragment shader va remplir le GBuffer. Pour ce qui est des valeurs du modèle d'ombrage, je les ai écrites en dur dans le shader, mais on pourrait imaginer qu'ils viennent d'une albedo map, metalness map, diffuse map, shininess map.

Pour le "remplissage" du GBuffer, il s'agit de la partie la plus difficile de l'étape. Ici, dans le fragment shader, on utilise de multiples sorties, correspondant aux valeurs du GBuffer. Mais ces sorties doivent êtres paramétrées.

### Côté application

Pour paramétrer les multiples sorties du fragment shader, j'ai dû créer un frame buffer "gFrameBuffer" auquel j'ajoute des GL_COLOR_ATTACHMENTi où i va de 0 à 3. Chaque valeur du GBuffer fait l'objet d'une texture, sauf pour la position et le matid que j'ai mutualisés en une texture 4 canaux. Finalement, chacune de ces textures correspond à la sortie i dans le fragment shader.

Dans la boucle de rendu, je fais un bind du frame buffer, j'affecte mes uniforms de transformation habituels, et je fais mes draws. Ici, je dessine un simple robot, mais on pourrait faire plusieurs draws différents si on voulait, tout ce qui nous intéresse à la fin de cette étape est les informations pour chaque pixel du fragment ayant la plus faible profondeur après rastérisation pas la carte graphique.

### Captures d'écran

![screenshot](screenshots/robot_differe.png)

On a un rendu comme avec la pipeline standard, sans artefact. Cependant, les calculs de Blinn-Phong n'ont été faits qu'une seule fois par pixel du robot.

## Interpolation des pixels

### Abstraction

Maintenant qu'on a notre GBuffer, on pet